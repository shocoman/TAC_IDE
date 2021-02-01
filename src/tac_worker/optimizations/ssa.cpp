//
// Created by shoco on 1/22/2021.
//

#include "ssa.hpp"

void convert_to_ssa(Function &function) {
    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    ID2DOM id_to_dominators = find_dominators(blocks);
    ID2IDOM id_to_immediate_dominator = find_immediate_dominators(function);
    ID2DF id_to_dominance_frontier = find_dominance_frontier(blocks, id_to_immediate_dominator);

    // region PrintDominators
    //    std::cout << "-- PRINT --" << std::endl;
    //    for (auto &[id, doms] : id_to_dominators) {
    //        std::cout << id << " (" << id_to_block.at(id)->node_name << "): ";
    //        for (auto &d : doms) {
    //            std::cout << d << ", ";
    //        }
    //        if (id_to_immediate_dominator.find(id) != id_to_immediate_dominator.end())
    //            std::cout << "\t IDom: " << id_to_immediate_dominator.at(id);
    //
    //        std::cout << "; DomFrontier: ";
    //        for (auto &df : id_to_dominance_frontier.at(id)) {
    //            std::cout << id_to_block.at(df)->get_name() << ", ";
    //        }
    //
    //        std::cout << std::endl;
    //    }
    //    std::cout << "-- END_PRINT --" << std::endl;
    // endregion

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
                op.predecessor_id = block->id;
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

    rename(function.find_root_node()->id);
}

ID2DF find_dominance_frontier(const BasicBlocks &blocks, ID2IDOM &id_to_immediate_dominator) {
    std::unordered_map<int, std::unordered_set<int>> id_to_dominance_frontier;
    for (const auto &b : blocks)
        id_to_dominance_frontier[b->id] = {};
    for (const auto &b : blocks) {
        if (b->predecessors.size() > 1) {
            for (const auto &pred : b->predecessors) {
                int runner_id = pred->id;

                while (runner_id != id_to_immediate_dominator.at(b->id)) {
                    id_to_dominance_frontier[runner_id].insert(b->id);
                    runner_id = id_to_immediate_dominator.at(runner_id);
                }
            }
        }
    }
    return id_to_dominance_frontier;
}

ID2IDOM find_immediate_dominators(Function &function) {
    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    std::unordered_map<int, int> id_to_rpo = function.get_reverse_post_ordering();

    // We use std::map here for its sorting capabilities
    // rpo_to_id will contain ids sorted in reverse post order.
    // It's important for the algorithm (convergence)
    std::map<int, int> rpo_to_id;
    for (auto &[id, rpo] : id_to_rpo)
        rpo_to_id.emplace(rpo, id);

    ID2IDOM id_to_idom;
    auto intersect = [&](BasicBlock *i, BasicBlock *j) {
        auto finger1 = i->id;
        auto finger2 = j->id;
        while (finger1 != finger2) {
            while (id_to_rpo.at(finger1) > id_to_rpo.at(finger2))
                finger1 = id_to_idom.at(finger1);
            while (id_to_rpo.at(finger2) > id_to_rpo.at(finger1))
                finger2 = id_to_idom.at(finger2);
        }
        return id_to_block.at(finger1);
    };

    auto entry_node_id = function.find_root_node()->id;
    id_to_idom[entry_node_id] = entry_node_id;

    bool changed = true;
    int iterations = 0;
    while (changed) {
        iterations++;
        changed = false;

        for (auto &[rpo, id] : rpo_to_id) {
            auto &b = id_to_block.at(id);
            if (b->id == entry_node_id)
                continue;

            auto preds = b->predecessors.begin();
            while (preds != b->predecessors.end() &&
                   id_to_idom.find((*preds)->id) == id_to_idom.end())
                std::advance(preds, 1);
            if (preds == b->predecessors.end())
                continue;
            auto new_idom = *preds;

            for (std::advance(preds, 1); preds != b->predecessors.end(); std::advance(preds, 1)) {
                if (id_to_idom.find((*preds)->id) != id_to_idom.end())
                    new_idom = intersect(*preds, new_idom);
            }

            if (id_to_idom.find(b->id) == id_to_idom.end() ||
                id_to_idom.at(b->id) != new_idom->id) {
                id_to_idom[b->id] = new_idom->id;
                changed = true;
            }
        }
    }

    bool graph_is_irreducible = iterations > 2;
    // region Print IDom set
    //    std::cout << __PRETTY_FUNCTION__ << std::endl;
    //    std::cout << "Iterations: " << iterations << std::endl;
    //    for (auto &[id, idom_id] : id_to_idom) {
    //        std::cout << id_to_block.at(id)->get_name() << " -> " <<
    //        id_to_block.at(idom_id)->get_name()
    //                  << std::endl;
    //    }
    // endregion

    // entry node has no idom
    id_to_idom.erase(entry_node_id);
    return id_to_idom;
}

ID2DOM find_dominators(const BasicBlocks &blocks) {
    std::unordered_map<int, std::unordered_set<int>> id_to_dominator;

    std::unordered_set<int> N;
    for (auto &b : blocks)
        N.insert(b->id);

    for (auto &b : blocks)
        id_to_dominator[b->id] = N;

    int iterations = 0;
    bool changed = true;
    while (changed) {
        iterations++;
        changed = false;
        for (auto &b : blocks) {
            // get intersection of sets of predecessors
            std::unordered_set<int> pred_intersect;
            for (auto &pred : b->predecessors) {
                if (id_to_dominator.find(pred->id) == id_to_dominator.end())
                    continue;
                auto pred_dominators = id_to_dominator.at(pred->id);
                if (!pred_intersect.empty()) {
                    std::unordered_set<int> intersection;
                    std::set_intersection(pred_intersect.begin(), pred_intersect.end(),
                                          pred_dominators.begin(), pred_dominators.end(),
                                          std::inserter(intersection, intersection.end()));
                    pred_intersect = intersection;
                } else {
                    pred_intersect = pred_dominators;
                }
            }

            pred_intersect.insert(b->id);
            if (pred_intersect != id_to_dominator[b->id]) {
                id_to_dominator[b->id] = pred_intersect;
                changed = true;
            }
        }
    }

    return id_to_dominator;
}

void remove_phi_functions(Function &function) {
    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    BasicBlocks new_blocks;
    for (auto &b : blocks) {
        std::map<std::pair<int, int>, int> replace_block_id;
        for (int i = 0; i < b->phi_functions; ++i) {
            auto &phi = b->quads[i];
            for (auto &op : phi.ops) {

                // Predecessor id could change after optimizations
                if (replace_block_id.find({op.predecessor_id, b->id}) == replace_block_id.end() &&
                    id_to_block.at(op.predecessor_id)->successors.size() > 1) {
                    auto *pred = id_to_block.at(op.predecessor_id);

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
                    replace_block_id[{op.predecessor_id, b->id}] = split_block->id;

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
                if (replace_block_id.find({op.predecessor_id, b->id}) != replace_block_id.end()) {
                    pred = id_to_block.at(replace_block_id.at({op.predecessor_id, b->id}));
                } else {
                    pred = id_to_block.at(op.predecessor_id);
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

void print_dominator_tree(ID2Block &id_to_block, ID2IDOM &id_to_idom) {
    GraphWriter writer;
    for (const auto &[id1, id2] : id_to_idom) {
        // make a connection between blocks[id1] and blocks[block_id]
        const auto name1 = id_to_block.at(id2)->node_name;
        const auto name2 = id_to_block.at(id1)->node_name;
        writer.set_node_name(name1, name1);
        writer.set_node_name(name2, name2);
        writer.set_node_text(name1, {});
        writer.set_node_text(name2, {});
        writer.add_edge(name1, name2);
    }

    writer.render_to_file("dominator_tree.png");
    system("feh dominator_tree.png &");
}
