//
// Created by shoco on 1/22/2021.
//

#include "useless_code_elimination.hpp"
void UselessCodeEliminationDriver::compute_reverse_dominance_frontier() {
    f.reverse_graph();
    ir.id_to_ipostdom = get_immediate_dominators(f);
    ir.reverse_df = get_dominance_frontier(f, ir.id_to_ipostdom);
    f.reverse_graph();
}

void UselessCodeEliminationDriver::remove_noncritical_operations() {
    compute_reverse_dominance_frontier();
    ir.use_def_graph.emplace(f);

    // mark critical quads/operations
    std::vector<UseDefGraph::Location> work_list;
    for (const auto &b : f.basic_blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];
            UseDefGraph::Location location = {b->id, q_index};

            // every unconditional jump is critical
            if (q.type == Quad::Type::Goto)
                ir.critical_operations.insert(location);

            if (Quad::is_critical(q.type)) {
                ir.critical_operations.insert(location);
                work_list.push_back(location);
            }
        }
    }

    while (!work_list.empty()) {
        auto [block_id, quad_i] = work_list.back();
        work_list.pop_back();
        auto &q = f.get_quad({block_id, quad_i});

        // mark quad's operands as critical
        for (auto &op : q.get_rhs_names(false)) {
            auto use = UseDefGraph::Use{{block_id, quad_i}, op};
            auto &definitions = ir.use_def_graph->use_to_definitions.at(use);
            for (auto &def : definitions)
                if (ir.critical_operations.insert(def.location).second)
                    work_list.push_back(def.location);
        }

        for (auto &b_id : ir.reverse_df.at(block_id)) {
            auto &b = f.id_to_block.at(b_id);
            if (b->quads.empty())
                continue;

            UseDefGraph::Location jump_position{b_id, b->quads.size() - 1};
            if (ir.critical_operations.insert(jump_position).second)
                work_list.push_back(jump_position);
        }
    }

    // id of blocks that contain at least one marked instruction
    for (auto [block, quad] : ir.critical_operations)
        ir.marked_blocks.insert(block);

    // Sweep Pass (remove non critical quads/operations)
    for (const auto &b : f.basic_blocks) {
        for (int q_index = b->quads.size() - 1; q_index >= 0; --q_index) {
            auto &q = b->quads[q_index];

            UseDefGraph::Location location{b->id, q_index};
            bool op_is_not_critical = ir.critical_operations.count(location) == 0;
            bool quad_is_self_copy = q.type == Quad::Type::Assign && q.dest->name == q.ops[0].value;
            if (op_is_not_critical || quad_is_self_copy) {
                if (q.is_conditional_jump()) {
                    // replace quad with jump to closest "marked" postdominator
                    int post_dom_id = b->id;
                    do {
                        post_dom_id = ir.id_to_ipostdom.at(post_dom_id);
                    } while (ir.marked_blocks.count(post_dom_id) == 0);

                    auto closest_post_dominator = f.id_to_block.at(post_dom_id);
                    q.type = Quad::Type::Goto;
                    q.dest = Dest(closest_post_dominator->lbl_name.value(), Dest::Type::JumpLabel);
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

void UselessCodeEliminationDriver::remove_unreachable_blocks() {
    // Mark reachable blocks
    auto root = f.get_entry_block();
    std::unordered_set<int> reachable_ids;
    std::function<void(BasicBlock(*))> MarkReachableBlocks = [&](BasicBlock *b) {
        if (reachable_ids.insert(b->id).second)
            for (auto &succ : b->successors)
                MarkReachableBlocks(succ);
    };
    MarkReachableBlocks(root);

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

void UselessCodeEliminationDriver::merge_basic_blocks() {
    // temporary remove entry and exit block (hack)
    auto RemoveBlock = [&](auto b_type) {
        auto block = std::find_if(f.basic_blocks.begin(), f.basic_blocks.end(),
                                  [b_type](auto &b) { return b->type == b_type; });
        if (block != f.basic_blocks.end()) {
            f.id_to_block.erase(block->get()->id);
            block->get()->remove_successors();
            block->get()->remove_predecessors();
            f.basic_blocks.erase(block);
        }
    };

    RemoveBlock(BasicBlock::Type::Entry);
    RemoveBlock(BasicBlock::Type::Exit);

    // Clean Pass (merge blocks, etc)
    bool changed = true;
    auto CleanPass = [&](const std::map<int, int> &postorder_to_id) {
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

                // combine two blocks into one (merge 'b' into 'succ')
                else if (succ->predecessors.size() == 1) {
                    // update predecessors' jump operations
                    succ->lbl_name = b->lbl_name;
                    // remove last jump
                    if (b->quads.back().type == Quad::Type::Goto)
                        b->quads.pop_back();
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

        CleanPass(postorder_to_id);
    }

    f.add_entry_and_exit_block();
}

Function &UselessCodeEliminationDriver::run() {
    remove_noncritical_operations();
    remove_unreachable_blocks();
    merge_basic_blocks();

    f.update_block_ids();

    return f;
}
