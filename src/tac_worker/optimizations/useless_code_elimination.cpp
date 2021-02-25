//
// Created by shoco on 1/22/2021.
//

#include "useless_code_elimination.hpp"

void remove_noncritical_operations(Function &f) {
    BasicBlocks &blocks = f.basic_blocks;
    ID2Block &id_to_block = f.id_to_block;

    f.reverse_graph();
    auto id_to_rev_idom = find_immediate_dominators(f);
    auto reverse_df = find_dominance_frontier(f.basic_blocks, id_to_rev_idom);
    f.reverse_graph();

    using Place = std::pair<int, int>;

    struct VarInfo {
        bool critical = false;
        Place defined_at = {-1, -1};
    };

    std::map<Place, VarInfo> using_info;
    std::unordered_map<std::string, Place> name_to_place;

    for (const auto &b : blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];
            Place place = {b->id, q_index};

            using_info[place].defined_at = place;
            if (q.is_jump())
                name_to_place[*b->lbl_name] = place;
            else
                name_to_place[q.dest->name] = place;
        }
    }

    // mark critical quads/operations
    std::vector<Place> work_list;
    for (const auto &b : blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];

            if (Quad::is_critical(q.type)) {
                Place place = {b->id, q_index};
                using_info.at(place).critical = true;
                work_list.push_back(place);
            }
        }
    }

    while (!work_list.empty()) {
        auto [block_id, quad_i] = work_list.back();
        work_list.pop_back();
        auto &q = f.id_to_block.at(block_id)->quads[quad_i];

        for (const auto &op : q.ops) {
            if (op.is_var() && q.type != Quad::Type::Call) {
                auto &defined_place = name_to_place.at(op.value);
                auto &var_use_info = using_info.at(defined_place);
                if (!var_use_info.critical) {
                    var_use_info.critical = true;
                    work_list.push_back(defined_place);
                }
            }
        }

        for (auto &b_id : reverse_df.at(block_id)) {
            int dom_block_num = -1;
            for (int i = 0; i < blocks.size(); ++i)
                if (blocks[i]->id == b_id)
                    dom_block_num = i;
            auto dom_block = id_to_block.at(b_id);
            int jump_quad_num = dom_block->quads.size() - 1;

            Place place{dom_block_num, jump_quad_num};
            if (!using_info.at(place).critical) {
                using_info.at(place).critical = true;
                work_list.push_back(place);
            }
        }
    }

    // Sweep Pass (remove non critical quads/operations)
    for (const auto &b : blocks) {
        for (int q_index = b->quads.size() - 1; q_index >= 0; --q_index) {
            auto &q = b->quads[q_index];

            Place place{b->id, q_index};
            if (!using_info.at(place).critical) {
                if (q.is_conditional_jump()) {
                    auto closest_post_dominator = id_to_block.at(id_to_rev_idom.at(b->id));
                    q.type = Quad::Type::Goto;
                    q.dest = Dest(*closest_post_dominator->lbl_name, {}, Dest::Type::JumpLabel);
                    b->remove_successors();
                    b->add_successor(closest_post_dominator);
                } else {
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
                    b->quads.pop_back();
                    // copy operations
                    b->quads.insert(b->quads.end(), succ->quads.begin(), succ->quads.end());
                    // copy successors
                    b->remove_successors();
                    for (auto &s : succ->successors) {
                        s->predecessors.erase(succ);
                        b->add_successor(s);
                    }
                    // remove successor
                    block_ids_to_remove.insert(succ->id);
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

    function.update_block_ids();
}
