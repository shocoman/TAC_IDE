//
// Created by shoco on 1/22/2021.
//

#include "useless_code_elimination.hpp"

void remove_noncritical_operations(Function &f) {
    BasicBlocks &blocks = f.basic_blocks;
    ID2Block &id_to_block = f.id_to_block;

    f.reverse_graph();
    auto id_to_ipostdom = get_immediate_dominators(f);
    auto reverse_df = get_dominance_frontier(f, id_to_ipostdom);
    f.reverse_graph();

    using Position = std::pair<int, int>;

    std::set<Position> critical_operations;
    std::unordered_map<std::string, Position> name_to_position;

    // record positions for each operation
    for (const auto &b : blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];

            auto op_name = q.is_jump() ? b->get_name() : q.dest->name;
            name_to_position.emplace(op_name, Position{b->id, q_index});
        }
    }

    // mark critical quads/operations
    std::vector<Position> work_list;
    for (const auto &b : blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];
            Position pos = {b->id, q_index};

            // every unconditional jump is critical
            if (q.type == Quad::Type::Goto)
                critical_operations.insert(pos);

            if (Quad::is_critical(q.type)) {
                critical_operations.insert(pos);
                work_list.push_back(pos);
            }
        }
    }

    while (!work_list.empty()) {
        auto [block_id, quad_i] = work_list.back();
        work_list.pop_back();
        auto &q = f.id_to_block.at(block_id)->quads.at(quad_i);

        if (q.dest.has_value()) {
            std::string dest = q.dest->name;
            fmt::format("\n");
        }

        for (auto &op : q.get_rhs(false)) {
            auto &defined_position = name_to_position.at(op);
            if (critical_operations.insert(defined_position).second) {
                work_list.push_back(defined_position);
            }
        }

        for (auto &b_id : reverse_df.at(block_id)) {
            auto &b = id_to_block.at(b_id);
            if (b->quads.empty())
                continue;

            Position jump_position{b_id, b->quads.size() - 1};
            if (critical_operations.insert(jump_position).second)
                work_list.push_back(jump_position);
        }
    }

    // id of blocks that contain at least one marked instruction
    std::set<int> marked_blocks;
    for (auto [block, quad] : critical_operations)
        marked_blocks.insert(block);

    // Sweep Pass (remove non critical quads/operations)
    for (const auto &b : blocks) {
        for (int q_index = b->quads.size() - 1; q_index >= 0; --q_index) {
            auto &q = b->quads[q_index];

            Position position{b->id, q_index};
            bool op_is_not_critical = critical_operations.find(position) == critical_operations.end();
            if (op_is_not_critical) {
                if (q.is_conditional_jump()) {
                    // replace with jump to closest "marked" postdominator
                    int post_dom_id = b->id;
                    do {
                        post_dom_id = id_to_ipostdom.at(post_dom_id);
                    } while (marked_blocks.count(post_dom_id) == 0);

                    auto closest_post_dominator = id_to_block.at(post_dom_id);
                    q.type = Quad::Type::Goto;
                    q.dest = Dest(closest_post_dominator->lbl_name.value(), {}, Dest::Type::JumpLabel);
                    b->remove_successors();
                    b->add_successor(closest_post_dominator);
                } else if (q.type != Quad::Type::Goto) {
                    if (b->quads[q_index].type == Quad::Type::PhiNode)
                        --b->phi_functions;
                    b->quads.erase(b->quads.begin() + q_index);
                }
            }
        }
    }
}

void remove_unreachable_blocks(Function &f) {
    // Mark reachable blocks
    auto root = f.find_entry_block();
    std::unordered_set<int> reachable_ids;
    std::function<void(BasicBlock(*))> reachable_walker = [&](BasicBlock *b) {
        if (reachable_ids.insert(b->id).second)
            for (auto &succ : b->successors)
                reachable_walker(succ);
    };
    reachable_walker(root);

    // Remove unreachable blocks
    for (int i = f.basic_blocks.size() - 1; i >= 0; --i) {
        auto &b = f.basic_blocks[i];
        if (reachable_ids.count(b->id) == 0) {
            b->remove_successors();
            b->remove_predecessors();
            f.id_to_block.erase(b->id);
            f.basic_blocks.erase(f.basic_blocks.begin() + i);
        }
    }
}

void merge_basic_blocks(Function &f) {
    // temporary remove entry and exit block (hack)
    auto RemoveBlock = [&](std::string name) {
        auto block = std::find_if(f.basic_blocks.begin(), f.basic_blocks.end(),
                                        [name](auto &b) { return b->get_name() == name; });
        if (block != f.basic_blocks.end()) {
            f.id_to_block.erase(block->get()->id);
            block->get()->remove_successors();
            block->get()->remove_predecessors();
            f.basic_blocks.erase(block);
        }
    };

    RemoveBlock("Entry");
    RemoveBlock("Exit");

    // Clean Pass (merge blocks, etc)
    bool changed = true;
    auto one_pass = [&](const std::map<int, int> &postorder_to_id) {
        std::unordered_set<int> block_ids_to_remove;

        // walk through blocks in post order
        for (auto &[postorder, id] : postorder_to_id) {
            if (f.id_to_block.count(id) == 0 || f.id_to_block.at(id)->quads.empty())
                continue;

            auto &b = f.id_to_block.at(id);

            // replace branch with jump
            if (b->successors.size() == 1 && b->quads.back().is_conditional_jump()) {
                b->quads.back().type = Quad::Type::Goto;
                b->quads.back().clear_op(1);
                changed = true;
            }

            if (b->successors.size() == 1) {
                auto succ = *b->successors.begin();

                // remove empty block (has only 1 successor with goto)
                if (b->quads.size() == 1 && b->quads.back().is_jump()) {
                    for (auto &pred : b->predecessors) {
                        // replace successor to the predecessors
                        pred->successors.erase(b);
                        pred->add_successor(succ);
                        // correct jump operation
                        auto &jump = pred->quads.back();
                        if (jump.dest->name == b->lbl_name.value())
                            jump.dest->name = succ->lbl_name.value();
                    }
                    b->remove_successors();
                    b->remove_predecessors();
                    block_ids_to_remove.insert(id);
                    changed = true;
                }

                // combine two blocks into one
                else if (succ->predecessors.size() == 1) {
                    // remove last jump
                    if (b->quads.back().type == Quad::Type::Goto) {
                        b->quads.pop_back();
                    }
                    // copy operations
                    succ->quads.insert(succ->quads.end(), b->quads.begin(), b->quads.end());
                    succ->update_phi_positions();
                    // copy predecessors
                    for (auto &p : b->predecessors)
                        p->add_successor(succ);
                    b->remove_successors();
                    b->remove_predecessors();
                    // remove successor
                    block_ids_to_remove.insert(b->id);

                    changed = true;
                }

                // hoist a branch
                // if successor is empty and ends with conditional branch
                // copy branch from it
                else if (succ->quads.size() == 1 && succ->quads.back().is_conditional_jump()) {
                    b->quads.back() = succ->quads.back();
                    b->remove_successors();
                    for (auto &s : succ->successors)
                        b->add_successor(s);
                    changed = true;
                }
            }
        }

        // update phis ???

        // erase removed blocks
        for (int i = f.basic_blocks.size() - 1; i >= 0; --i) {
            auto &b = f.basic_blocks[i];
            if (block_ids_to_remove.count(b->id) > 0) {
                b->remove_predecessors();
                b->remove_successors();
                f.id_to_block.erase(b->id);
                f.basic_blocks.erase(f.basic_blocks.begin() + i);
            }
        }
    };

    while (changed) {
        changed = false;

        std::map<int, int> postorder_to_id;
        for (auto &[id, po] : f.get_post_ordering())
            postorder_to_id[po] = id;

        one_pass(postorder_to_id);
    }
}

void useless_code_elimination(Function &function) {
    remove_noncritical_operations(function);
    remove_unreachable_blocks(function);
    merge_basic_blocks(function);

    function.add_entry_and_exit_block();
    function.update_block_ids();
}
