//
// Created by shoco on 1/22/2021.
//

#include "ssa.hpp"

void convert_to_ssa(Function &function) {
    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    ID2IDOM id_to_immediate_dominator = find_immediate_dominators(function);
    ID2DF id_to_dominance_frontier = find_dominance_frontier(blocks, id_to_immediate_dominator);

    // find global names
    std::set<std::string> all_names;
    std::map<std::string, std::set<BasicBlock *>> var_to_block;
    std::set<std::string> global_names;
    for (auto &b : blocks) {
        std::set<std::string> var_kill;
        for (const auto &q : b->quads) {
            for (auto &r : q.get_rhs(false))
                if (var_kill.find(r) == var_kill.end())
                    global_names.insert(r);
            if (auto lhs = q.get_lhs(); lhs.has_value()) {
                var_kill.insert(lhs.value());
                var_to_block[lhs.value()].insert(b.get());
                all_names.insert(lhs.value());
            }
        }
    }

    // region PrintGlobalNames
    //    std::cout << "Global names: ";
    //    for (auto &name : global_names) {
    //        std::cout << name << ", ";
    //    }
    //    std::cout << std::endl;
    // endregion

    place_phi_functions(function, id_to_immediate_dominator, id_to_dominance_frontier, var_to_block,
                        global_names, all_names);
}

void place_phi_functions(Function &function, ID2IDOM &id_to_immediate_dominator,
                         ID2DF &id_to_dominance_frontier,
                         std::map<std::string, std::set<BasicBlock *>> &var_to_blocks,
                         std::set<std::string> &global_names, std::set<std::string> &all_names) {

    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    // place phi functions
    for (auto &name : global_names) {
        std::vector<BasicBlock *> work_list;
        auto &work_list_set = var_to_blocks.at(name);
        std::copy(work_list_set.begin(), work_list_set.end(), std::back_inserter(work_list));
        std::set<int> visited_blocks;

        for (int i = 0; i < work_list.size(); ++i) {
            for (auto &d : id_to_dominance_frontier.at(work_list[i]->id)) {
                if (auto d_block = id_to_block.at(d); !d_block->has_phi_function(name)) {
                    d_block->add_phi_function(name, {});

                    if (visited_blocks.find(d_block->id) == visited_blocks.end()) {
                        work_list.push_back(d_block);
                        visited_blocks.insert(d_block->id);
                    }
                }
            }
        }
    }

    // rename variables
    std::map<std::string, int> name_to_counter;
    std::map<std::string, std::vector<int>> name_to_stack;
    for (auto &name : all_names) {
        name_to_counter[name] = 0;
        name_to_stack[name] = {};
    }

    // for new name generation
    auto new_name = [&](std::string name) {
        int i = name_to_counter.at(name);
        name_to_counter.at(name)++;
        name_to_stack.at(name).push_back(i);
        return name + "." + std::to_string(i);
    };

    // for var renaming
    std::function<void(int)> rename = [&](int block_id) {
        std::vector<std::string> pushed_names;
        auto block = id_to_block.at(block_id);

        // rename phi functions
        for (int i = 0; i < block->phi_functions; ++i) {
            auto &phi = block->quads[i];
            pushed_names.push_back(phi.dest->name);
            phi.dest->name = new_name(phi.dest->name);
        }

        // rename other operations of form 'x = y + z'
        for (int i = block->phi_functions; i < block->quads.size(); ++i) {
            auto &q = block->quads[i];
            if (auto op1 = q.get_op(0);
                op1 && op1->is_var() && name_to_stack.find(op1->value) != name_to_stack.end()) {
                q.ops[0].value += "." + std::to_string(name_to_stack.at(op1->value).back());
            }
            if (auto op2 = q.get_op(1);
                op2 && op2->is_var() && name_to_stack.find(op2->value) != name_to_stack.end()) {
                q.ops[1].value += "." + std::to_string(name_to_stack.at(op2->value).back());
            }
            if (q.dest && all_names.find(q.dest->name) != all_names.end()) {
                pushed_names.push_back(q.dest->name);
                q.dest->name = new_name(q.dest->name);
            }
        }

        // fill phi function parameters for each successor
        for (auto &s : block->successors) {
            for (int i = 0; i < s->phi_functions; ++i) {
                auto &phi = s->quads[i];
                auto name = phi.dest.value().name;
                auto name_without_dot = name.substr(0, name.find_first_of('.', 0));

                std::string next_name;
                if (auto stack = name_to_stack.find(name_without_dot);
                    stack != name_to_stack.end() && !stack->second.empty()) {
                    next_name = name_without_dot + "." + std::to_string(stack->second.back());
                } else {
                    next_name = name;
                }
                Operand op(next_name, Operand::Type::Var);
                op.phi_predecessor = block;
                phi.ops.push_back(op);
            }
        }

        // call for each successor in dominant tree
        for (auto &[id1, id2] : id_to_immediate_dominator)
            if (id2 == block_id)
                rename(id1);

        for (auto &n : pushed_names)
            name_to_stack.at(n).pop_back();
    };

    rename(function.find_entry_block()->id);
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

                // Predecessor id could change after optimizations
                if (replace_block_id.find({op.phi_predecessor->id, b->id}) == replace_block_id.end() &&
                    id_to_block.at(op.phi_predecessor->id)->successors.size() > 1) {
                    auto *pred = id_to_block.at(op.phi_predecessor->id);

                    auto split_block = std::make_unique<BasicBlock>();
                    split_block->quads.push_back(
                        Quad({}, {}, Quad::Type::Goto,
                             Dest(b->lbl_name.value(), {}, Dest::Type::JumpLabel)));

                    if (!new_blocks.empty())
                        split_block->id = new_blocks.back()->id + 1;
                    else
                        split_block->id = blocks.back()->id + 1;
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

                BasicBlock *pred;
                if (replace_block_id.find({op.phi_predecessor->id, b->id}) != replace_block_id.end()) {
                    pred = id_to_block.at(replace_block_id.at({op.phi_predecessor->id, b->id}));
                } else {
                    pred = id_to_block.at(op.phi_predecessor->id);
                }
                auto insert_pos =
                    pred->quads.back().is_jump() ? pred->quads.end() - 1 : pred->quads.end();
                pred->quads.insert(insert_pos,
                                   Quad(op.value, {}, Quad::Type::Assign, phi.dest.value()));
            }
        }

        b->quads.erase(b->quads.begin(), b->quads.begin() + b->phi_functions);
        b->phi_functions = 0;
    }

    blocks.reserve(blocks.size() + new_blocks.size());
    std::move(std::begin(new_blocks), std::end(new_blocks), std::back_inserter(blocks));
}

