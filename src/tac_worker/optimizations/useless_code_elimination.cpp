//
// Created by shoco on 1/22/2021.
//

#include "useless_code_elimination.hpp"

void useless_code_elimination(Function &function, ID2DF &id_to_rev_df, ID2IDOM &id_to_idom) {

    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    struct Place {
        int block_num;
        int quad_num;

        bool operator<(const Place &o) const {
            return std::tie(block_num, quad_num) < std::tie(o.block_num, o.quad_num);
            //            return (block_num < o.quad_num) || (block_num == o.block_num && quad_num <
            //            o.quad_num);
        }
    };

    struct VarInfo {
        bool critical = false;

        Place defined_at = {-1, -1};
        std::vector<Place> used_at{};
    };

    std::map<Place, VarInfo> using_info;
    std::map<std::string, Place> name_to_place;

    for (auto b_index = 0; b_index < blocks.size(); ++b_index) {
        auto &b = blocks[b_index];
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];
            Place place{b_index, q_index};

            using_info[place].defined_at = place;
            if (q.is_jump()) {
                name_to_place[*b->lbl_name] = place;
            } else {
                name_to_place[q.dest->name] = place;
            }
        }
    }

    // mark critical quads/operations
    std::set<Place> work_list;
    for (auto b_index = 0; b_index < blocks.size(); ++b_index) {
        auto &b = blocks[b_index];
        for (int i = 0; i < b->quads.size(); ++i) {
            auto &q = b->quads[i];

            if (Quad::is_critical(q.type)) {
                Place place{b_index, i};
                using_info.at(place).critical = true;
                work_list.insert(place);
            }
        }
    }

    while (!work_list.empty()) {
        auto next_place = *work_list.begin();
        work_list.erase(next_place);
        auto &q = blocks[next_place.block_num]->quads[next_place.quad_num];

        for (const auto &op : q.ops) {
            if (op.is_var()) {
                auto &defined_place = name_to_place.at(op.value);
                auto &var_use_info = using_info.at(defined_place);
                if (!var_use_info.critical) {
                    var_use_info.critical = true;
                    work_list.insert(defined_place);
                }
            }
        }

        auto &current_block = blocks[next_place.block_num];
        for (auto &b_id : id_to_rev_df.at(current_block->id)) {
            int dom_block_num = -1;
            for (int i = 0; i < blocks.size(); ++i)
                if (blocks[i]->id == b_id)
                    dom_block_num = i;
            auto dom_block = id_to_block.at(b_id);
            int jump_quad_num = dom_block->quads.size() - 1;

            Place place{dom_block_num, jump_quad_num};
            if (!using_info.at(place).critical) {
                using_info.at(place).critical = true;
                work_list.insert(place);
            }
        }
    }

    // Sweep Pass (remove not critical quads/operations)
    for (auto b_index = 0; b_index < blocks.size(); ++b_index) {
        auto &b = blocks[b_index];
        for (int q_index = b->quads.size() - 1; q_index >= 0; --q_index) {
            auto &q = b->quads[q_index];

            Place place{b_index, q_index};
            if (!using_info.at(place).critical) {
                if (q.is_jump()) {
                    if (!using_info.at(place).critical) {
                        auto closest_post_dominator = id_to_block.at(id_to_idom.at(b->id));
                        q.type = Quad::Type::Goto;
                        q.dest = Dest(*closest_post_dominator->lbl_name, {}, Dest::Type::JumpLabel);
                        b->remove_successors();
                        b->add_successor(closest_post_dominator);
                    }
                } else {
                    b->quads.erase(b->quads.begin() + q_index);
                }
            }
        }
    }

    // Mark unreachable blocks
    auto root = find_root_node(blocks);
    std::set<int> reachable_ids;
    std::function<void(BasicBlock(*))> reachable_walker = [&](BasicBlock *b) {
        if (reachable_ids.insert(b->id).second)
            for (auto &succ : b->successors)
                reachable_walker(succ);
    };
    reachable_walker(root);

    // Remove unreachable blocks
    for (int i = blocks.size() - 1; i >= 0; --i) {
        auto &b = blocks[i];
        if (reachable_ids.find(b->id) == reachable_ids.end()) {
            b->node_name += " (REMOVE) ";

            b->remove_successors();
            b->remove_predecessors();
            id_to_block.erase(b->id);
            blocks.erase(blocks.begin() + i);
        }
    }

    print_cfg(blocks, "before_clean.png");

    // Clean Pass
    bool changed = true;
    auto one_pass = [&](const std::map<int, int> &postorder_to_id) {
        std::set<int> block_ids_to_remove;

        // walk through blocks in post order
        for (auto &[postorder, id] : postorder_to_id) {
            //            if (changed) break;

            auto block_it = id_to_block.find(id);
            if (block_it == id_to_block.end())
                continue;
            auto &b = block_it->second;

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
        for (int i = blocks.size() - 1; i >= 0; --i) {
            auto &b = blocks[i];
            if (block_ids_to_remove.find(b->id) != block_ids_to_remove.end()) {
                b->remove_predecessors();
                b->remove_successors();
                id_to_block.erase(b->id);
                blocks.erase(blocks.begin() + i);
            }
        }
    };

    int iter = 0;
    while (changed) {
        iter++;
        std::map<int, int> postorder_to_id;
        for (auto &[id, rpo] : generate_post_order_numbers(blocks))
            postorder_to_id[rpo] = id;

        changed = false;
        one_pass(postorder_to_id);
    }

    print_blocks(blocks);
}