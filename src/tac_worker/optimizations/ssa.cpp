//
// Created by shoco on 1/22/2021.
//

#include "ssa.hpp"

void convert_to_ssa(Function &function) {
    // find global names
    std::set<std::string> all_names;
    std::map<std::string, std::set<BasicBlock *>> var_to_block;
    std::set<std::string> global_names;
    for (auto &b : function.basic_blocks) {
        std::set<std::string> var_kill;
        for (const auto &q : b->quads) {
            for (auto &op : q.get_rhs(false))
                if (var_kill.find(op) == var_kill.end())
                    global_names.insert(op);
            if (auto lhs = q.get_lhs(); lhs.has_value()) {
                var_kill.insert(lhs.value());
                var_to_block[lhs.value()].insert(b.get());
                all_names.insert(lhs.value());
            }
        }
    }

    place_phi_functions(function, var_to_block, global_names);
    rename_variables(function, all_names);
}

void place_phi_functions(Function &function,
                         std::map<std::string, std::set<BasicBlock *>> &var_to_blocks,
                         std::set<std::string> &global_names) {

    ID2IDOM id_to_idom = get_immediate_dominators(function);
    ID2DF id_to_dominance_frontier = get_dominance_frontier(function, id_to_idom);

    //    fmt::print("Global names: {}; All names: {}\n", global_names, all_names);
    //    fmt::print("Dominance Frontier:\n {}\n", id_to_dominance_frontier);

    // place phi functions
    for (auto &name : global_names) {
        std::vector<BasicBlock *> work_list;
        auto &work_list_set = var_to_blocks.at(name);
        std::copy(work_list_set.begin(), work_list_set.end(), std::back_inserter(work_list));
        std::set<int> visited_blocks;

        for (int i = 0; i < work_list.size(); ++i) {
            for (auto &d : id_to_dominance_frontier.at(work_list[i]->id)) {
                auto d_block = function.id_to_block.at(d);
                if (not d_block->has_phi_function(name)) {
                    d_block->add_phi_function(name, {});

                    if (visited_blocks.insert(d_block->id).second)
                        work_list.push_back(d_block);
                }
            }
        }
    }
}

void rename_variables(Function &function, std::set<std::string> &global_names) {
    ID2IDOM id_to_idom = get_immediate_dominators(function);

    std::map<std::string, int> name_to_counter;
    std::map<std::string, std::vector<int>> name_to_stack;
    for (auto &name : global_names) {
        name_to_counter[name] = 0;
        name_to_stack[name] = {};
    }

    auto MakeNewName = [&](auto &name) {
        int i = name_to_counter.at(name);
        name_to_counter.at(name)++;
        name_to_stack.at(name).push_back(i);
        return fmt::format("{}.{}", name, i);
    };

    std::function<void(int)> RenameVars = [&](int block_id) {
        std::vector<std::string> pushed_names;
        auto block = function.id_to_block.at(block_id);

        // rename phi functions
        for (int i = 0; i < block->phi_functions; ++i) {
            auto &phi = block->quads[i];
            pushed_names.push_back(phi.dest->name);
            phi.dest->name = MakeNewName(phi.dest->name);
        }

        // rename other operations of form 'x = y + z'
        for (int i = block->phi_functions; i < block->quads.size(); ++i) {
            auto &q = block->quads[i];
            auto op1 = q.get_op(0), op2 = q.get_op(1);

            if (op1 && op1->is_var() && name_to_stack.count(op1->value) > 0)
                q.ops[0].value += "." + std::to_string(name_to_stack.at(op1->value).back());

            if (op2 && op2->is_var() && name_to_stack.count(op2->value) > 0)
                q.ops[1].value += "." + std::to_string(name_to_stack.at(op2->value).back());

            if (q.dest && global_names.count(q.dest->name) > 0) {
                pushed_names.push_back(q.dest->name);
                q.dest->name = MakeNewName(q.dest->name);
            }
        }

        // fill phi function parameters for each successor
        for (auto &s : block->successors) {
            for (int i = 0; i < s->phi_functions; ++i) {
                auto &phi = s->quads[i];
                auto name = phi.dest->name;
                auto name_without_suffix = name.substr(0, name.find_first_of('.', 0));

                auto stack = name_to_stack.find(name_without_suffix);
                std::string next_name =
                    (stack != name_to_stack.end() && !stack->second.empty())
                        ? name_without_suffix + "." + std::to_string(stack->second.back())
                        : name;

                Operand op(next_name, Operand::Type::Var);
                op.phi_predecessor = block;
                phi.ops.push_back(op);
            }
        }

        // call for each successor in dominator tree
        for (auto &[child_id, parent_id] : id_to_idom)
            if (parent_id == block_id)
                RenameVars(child_id);

        for (auto &n : pushed_names)
            name_to_stack.at(n).pop_back();
    };

    RenameVars(function.find_entry_block()->id);
}

void convert_from_ssa(Function &function) {
    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    BasicBlocks new_blocks;
    for (auto &b : blocks) {
        std::map<std::pair<int, int>, int> replace_block_id;
        for (int i = 0; i < b->phi_functions; ++i) {
            auto &phi = b->quads[i];
            for (auto &op : phi.ops) {

                if (replace_block_id.find({op.phi_predecessor->id, b->id}) == replace_block_id.end() &&
                    id_to_block.at(op.phi_predecessor->id)->successors.size() > 1) {
                    auto &pred = id_to_block.at(op.phi_predecessor->id);

                    auto split_block = std::make_unique<BasicBlock>();
                    split_block->quads.push_back(Quad(
                        {}, {}, Quad::Type::Goto, Dest(b->lbl_name.value(), {}, Dest::Type::JumpLabel)));

                    split_block->id =
                        !new_blocks.empty() ? new_blocks.back()->id + 1 : blocks.back()->id + 1;
                    split_block->node_name = split_block->get_name();
                    split_block->lbl_name = split_block->get_name();

                    id_to_block[split_block->id] = split_block.get();
                    replace_block_id[{op.phi_predecessor->id, b->id}] = split_block->id;

                    pred->successors.erase(b.get());
                    b->predecessors.erase(pred);

                    if (pred->quads.back().dest->name == b->lbl_name) {
                        pred->quads.back().dest->name = split_block->lbl_name.value();
                    } else {
                        Dest dest(split_block->lbl_name.value(), {}, Dest::Type::JumpLabel);
                        pred->quads.push_back(Quad({}, {}, Quad::Type::Goto, dest));
                    }

                    pred->add_successor(split_block.get());
                    split_block->add_successor(b.get());

                    new_blocks.push_back(std::move(split_block));
                }

                BasicBlock *pred =
                    (replace_block_id.count({op.phi_predecessor->id, b->id}) > 0)
                        ? id_to_block.at(replace_block_id.at({op.phi_predecessor->id, b->id}))
                        : id_to_block.at(op.phi_predecessor->id);

                pred->add_quad_before_jump(Quad(op.value, {}, Quad::Type::Assign, phi.dest.value()));
            }
        }

        b->quads.erase(b->quads.begin(), b->quads.begin() + b->phi_functions);
        b->phi_functions = 0;
    }

    blocks.reserve(blocks.size() + new_blocks.size());
    std::move(std::begin(new_blocks), std::end(new_blocks), std::back_inserter(blocks));
    function.update_block_ids();
}
