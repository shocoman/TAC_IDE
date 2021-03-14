//
// Created by shoco on 1/22/2021.
//

#include "ssa.hpp"

void convert_to_ssa(Function &function) {
    //    auto [LiveIn, LiveOut] = live_variable_analyses(function);

    // find global names
    std::map<std::string, std::set<BasicBlock *>> var_to_block;
    std::set<std::string> global_names, all_names;
    for (auto &b : function.basic_blocks) {
        std::set<std::string> var_kill;
        for (const auto &q : b->quads) {
            for (auto &op : q.get_rhs(false))
                //                if (var_kill.find(op) == var_kill.end())
                global_names.insert(op);
            if (auto lhs = q.get_lhs(); lhs.has_value()) {
                var_kill.insert(lhs.value());
                var_to_block[lhs.value()].insert(b.get());
                all_names.insert(lhs.value());
            }
        }
    }

    //    fmt::print("Global names: {}\n", global_names);

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

    auto [LiveIn, LiveOut] = live_variable_analyses(function);
    // place phi functions
    for (auto &name : global_names) {
        std::vector<BasicBlock *> work_list;
        auto &work_list_set = var_to_blocks.at(name);
        std::copy(work_list_set.begin(), work_list_set.end(), std::back_inserter(work_list));
        std::set<int> visited_blocks;

        for (int i = 0; i < work_list.size(); ++i) {
            for (auto &d : id_to_dominance_frontier.at(work_list[i]->id)) {
                if (LiveIn.at(d).count(name) == 0)
                    continue;

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
    char delim = '_';
    ID2IDOM id_to_idom = get_immediate_dominators(function);

    std::map<std::string, int> name_to_counter;
    std::map<std::string, std::vector<int>> name_to_stack;
    for (auto &name : global_names) {
        name_to_counter[name] = 0;
        name_to_stack[name] = {};
    }

    auto MakeNewSSAName = [&](auto &name) {
        int i = name_to_counter.at(name);
        name_to_counter.at(name)++;
        name_to_stack.at(name).push_back(i);
        return fmt::format("{}{}{}", name, delim, i);
    };

    std::function<void(int)> RenameVars = [&](int block_id) {
        std::vector<std::string> pushed_names;
        auto block = function.id_to_block.at(block_id);

        // rename phi functions
        for (int i = 0; i < block->phi_functions; ++i) {
            auto &phi = block->quads[i];
            pushed_names.push_back(phi.dest->name);
            phi.dest->name = MakeNewSSAName(phi.dest->name);
        }

        // rename other operations of form 'x = y + z'
        for (int i = block->phi_functions; i < block->quads.size(); ++i) {
            auto &q = block->quads[i];
            auto op1 = q.get_op(0), op2 = q.get_op(1);

            for (auto &op : q.ops)
                if (op.is_var() && name_to_stack.count(op.value) > 0)
                    op.value += fmt::format("{}{}", delim, name_to_stack.at(op.value).back());

            if (q.dest && global_names.count(q.dest->name) > 0) {
                pushed_names.push_back(q.dest->name);
                q.dest->name = MakeNewSSAName(q.dest->name);
            }
        }

        // fill phi function parameters for each successor
        for (auto &s : block->successors) {
            for (int i = 0; i < s->phi_functions; ++i) {
                auto &phi = s->quads[i];
                auto name = phi.dest->name;
                auto name_without_suffix = name.substr(0, name.find_first_of(delim, 0));

                auto stack = name_to_stack.find(name_without_suffix);
                std::string next_name =
                    (stack != name_to_stack.end() && !stack->second.empty())
                        ? name_without_suffix + fmt::format("{}{}", delim, stack->second.back())
                        : name;

                Operand op(next_name, Operand::Type::Var, block);
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

    RenameVars(function.get_entry_block()->id);
}

void convert_from_ssa(Function &f) {
    NewNameGenerator new_name_generator(f);

    auto live_out = live_variable_analyses(f).second;
    auto id_to_idom = get_immediate_dominators(f);
    std::map<std::string, std::vector<std::string>> stacks;

    auto ScheduleCopies = [&](BasicBlock *b) {
        auto name = b->get_name();
        // Pass 1: Initialize the data structures
        using SrcDest = std::pair<std::string, std::string>;
        std::set<SrcDest> copy_set, worklist;
        std::map<std::string, std::string> map;
        std::set<std::string> used_by_another;

        for (auto &s : b->successors)
            for (int i = 0; i < s->phi_functions; ++i) {
                auto &phi = s->quads[i];
                std::string dest = phi.dest->name;
                std::string src;
                for (auto &op : phi.ops)
                    if (op.phi_predecessor->id == b->id)
                        src = op.value;
                assert(!src.empty());

                copy_set.emplace(src, dest);
                map[src] = src;
                map[dest] = dest;
                used_by_another.insert(src);
            }

        // Pass 2: Set up the worklist of initial copies
        auto copy_copy_set = copy_set;
        for (auto [src, dest] : copy_copy_set)
            if (used_by_another.count(dest) == 0) {
                worklist.emplace(src, dest);
                copy_set.erase({src, dest});
            }

        // Pass 3: Iterate over the worklist, inserting copies
        while (!worklist.empty() || !copy_set.empty()) {
            while (!worklist.empty()) {
                auto [src, dest] = *worklist.begin();
                worklist.erase({src, dest});

                if (live_out.at(b->id).count(dest) > 0) {
                    // insert copy from 'dest' to new temp 't' at phi-node defining 'dest'
                    auto copy_op = Quad(Operand(dest), {}, Quad::Type::Assign);
                    auto new_t = new_name_generator.make_new_name();
                    copy_op.dest = Dest(new_t, {}, Dest::Type::Var);
                    for (auto &block : f.basic_blocks)
                        for (int i = 0; i < block->phi_functions; ++i)
                            if (block->quads[i].type == Quad::Type::PhiNode &&
                                block->quads[i].dest->name == dest)
                                block->quads.insert(block->quads.begin() + block->phi_functions,
                                                    copy_op);
                    stacks[dest].push_back(new_t);
                }

                auto copy_op = Quad(Operand(map.at(src)), {}, Quad::Type::Assign);
                copy_op.dest = Dest(dest, {}, Dest::Type::Var);
                b->append_quad(copy_op);
                map[src] = dest;

                for (auto &[_src, _dest] : copy_set)
                    if (src == _dest)
                        worklist.emplace(_src, _dest);
            }

            if (!copy_set.empty()) {
                auto [src, dest] = *copy_set.begin();
                copy_set.erase({src, dest});

                auto copy_op = Quad(Operand(dest), {}, Quad::Type::Assign);
                auto new_t = new_name_generator.make_new_name();
                copy_op.dest = Dest(new_t, {}, Dest::Type::Var);
                b->append_quad(copy_op);
                map[dest] = new_t;
                worklist.emplace(src, dest);
            }
        }
    };

    std::function<void(BasicBlock *)> InsertCopies = [&](BasicBlock *b) {
        auto stack_copy = stacks;
        for (auto &q : b->quads)
            for (auto &op : q.ops)
                if (q.type != Quad::Type::Call && !stacks[op.value].empty()) {
                    if (stacks[op.value].back() != q.dest->name)
                        op.value = stacks[op.value].back();
                }

        ScheduleCopies(b);

        // call for each child in dominator tree
        for (auto &[child_id, parent_id] : id_to_idom)
            if (parent_id == b->id)
                InsertCopies(f.id_to_block.at(child_id));

        // pop each pushed name
        stacks = stack_copy;
    };

    InsertCopies(f.get_entry_block());

    // remove phis
    for (auto &b : f.basic_blocks) {
        b->quads.erase(b->quads.begin(), b->quads.begin() + b->phi_functions);
        b->phi_functions = 0;
    }
}