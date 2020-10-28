//
// Created by shoco on 10/7/2020.
//

#include <numeric>
#include "dataflow_graph.hpp"

void constant_folding(std::vector<std::unique_ptr<BasicBlock>> &nodes);

void liveness_analyses(const std::vector<std::unique_ptr<BasicBlock>> &nodes);

void remove_blocks_without_predecessors(std::vector<std::unique_ptr<BasicBlock>> &nodes);


void add_exit_block(std::vector<std::unique_ptr<BasicBlock>> &nodes) {
    auto entry_block = std::make_unique<BasicBlock>();
    entry_block->node_name = "Entry block";
    entry_block->id = 0;
    entry_block->add_successor(nodes.front().get());

    auto exit_node = std::make_unique<BasicBlock>();
    exit_node->node_name = "Exit block";
    exit_node->id = nodes.back()->id + 1;
    exit_node->quads.emplace_back(Quad({}, {}, Quad::Type::Return));

    // find blocks without successors (ending blocks) and connect them with exit block
    for (auto &n : nodes) {
        if (n->successors.empty()) {
            n->add_successor(exit_node.get());
        }
    }

    nodes.insert(nodes.begin(), std::move(entry_block));
    nodes.push_back(std::move(exit_node));
}


void print_loops(std::vector<std::unique_ptr<BasicBlock>> &nodes) {

    // generate ids for node names
    std::map<int, std::string> name_by_id;
    std::map<std::string, int> id_by_name;
    int counter = 0;
    for (const auto &n: nodes) {
        name_by_id[counter] = n->node_name;
        id_by_name[n->node_name] = counter;
        ++counter;
    }

    std::map<int, std::list<int>> adjacency_list;
    for (const auto &n: nodes) {
        adjacency_list[id_by_name[n->node_name]] = {};
        for (const auto &s: n->successors) {
            adjacency_list[id_by_name[n->node_name]].emplace_back(id_by_name[s->node_name]);
        }
    }

    LoopFinder l = LoopFinder(adjacency_list);
    l.find();

    std::cout << "Loops: " << std::endl;
    for (const auto &loop : l.loops) {
        for (const auto &i : loop) {
            std::cout << name_by_id.at(i) << " -> ";
        }
        std::cout << std::endl;
    }
}


void remove_blocks_without_predecessors(std::vector<std::unique_ptr<BasicBlock>> &nodes) {
    for (int i = nodes.size() - 1; i > 0; --i) {
        auto &n = nodes[i];
        if (n->predecessors.empty()) {
            n->remove_successors();
            nodes.erase(nodes.begin() + i);
        }
    }
}

[[deprecated]]
void liveness_analyses(const std::vector<std::unique_ptr<BasicBlock>> &nodes) {
    // block level liveness analyses
    struct LivenessState {
        bool live;
        int next_use;
    };

    for (auto &n: nodes) {
        std::vector<std::map<std::string, LivenessState>> block_liveness_data;
        std::map<std::string, LivenessState> block_liveness_nametable;

        // init vars used in the block
        for (auto &q : n->quads) {
            for (auto &u: q.get_used_vars()) {
                block_liveness_nametable.emplace(u, LivenessState{true, -1});
            }
        }

        for (int i = n->quads.size() - 1; i >= 0; --i) {
            std::map<std::string, LivenessState> current_nametable;
            auto &q = n->quads[i];
            auto lhs = q.get_lhs();
            auto rhs = q.get_rhs();

            // Step 1. attach info about x,y,z to i
            if (lhs.has_value()) {
                current_nametable[lhs.value()] = block_liveness_nametable.at(lhs.value());
            }
            for (auto &r: rhs) {
                current_nametable[r] = block_liveness_nametable.at(r);
            }
            // save in reversed order
            block_liveness_data.emplace(block_liveness_data.begin(), current_nametable);
            // Step 2. update name table about x (live=false)
            if (lhs.has_value()) {
                block_liveness_nametable[lhs.value()] = LivenessState{false, -69};
            }
            // Step 3. update name table about y,z (live=true, next_use=i)
            for (const auto &r: rhs) {
                block_liveness_nametable[r] = LivenessState{true, i};
            }

        }
        for (int i = 0; i < n->quads.size(); ++i) {
            std::cout << n->quads[i].fmt() << ";\t";
            auto &l = block_liveness_data[i];
            for (auto &[name, liveness] : l) {
                std::cout << "[ " << name << "; Live: " << liveness.live << "; Next use: " << liveness.next_use
                          << " ]; ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

        // remove dead quads
//        for (int i = n->quads.size()-1; i >= 0; --i) {
//            auto &q = n->quads[i];
//            if (q.dest.has_value()) {
//                std::string lhs = q.dest.value().dest_name;
//                auto &l = block_liveness_data[i];
//                if (!l.at(lhs).live) {
//                    n->quads.erase(n->quads.begin() + i);
//                }
//            }
//        }


    }
}

void constant_folding(std::vector<std::unique_ptr<BasicBlock>> &nodes) {
    for (auto &n : nodes) {
        for (auto &q : n->quads) {
            if (q.get_op(0)->is_number() && q.get_op(1)->is_number()) {
                double d = 0;
                double o1 = q.get_op(0)->get_double();
                double o2 = q.get_op(1)->get_double();
                switch (q.type) {
                    case Quad::Type::Add:
                        d = o1 + o2;
                        break;
                    case Quad::Type::Sub:
                        d = o1 - o2;
                        break;
                    case Quad::Type::Mult:
                        d = o1 * o2;
                        break;
                    case Quad::Type::Div:
                        d = o1 / o2;
                        break;
                }

                if (q.get_op(0)->is_int() && q.get_op(1)->is_int())
                    q.ops[0] = Operand(std::to_string((int) d));
                else
                    q.ops[0] = Operand(std::to_string(d));
                q.type = Quad::Type::Assign;
            }
        }
    }
}


void print_cfg(const std::vector<std::unique_ptr<BasicBlock>> &nodes, const std::string &filename) {
    DotWriter dot_writer;
    std::set<std::string> visited;
    // print edges
    for (const auto &n : nodes) {
        if (visited.find(n->node_name) == visited.end()) {
            visited.insert(n->node_name);

            std::vector<std::string> quad_lines;
            // print title for node
            if (n->lbl_name.has_value()) {
                dot_writer.set_node_name(n->node_name, n->lbl_name.value());
            }

            // print all quads as text
            for (auto &q : n->quads) {
                quad_lines.emplace_back(q.fmt());
            }
            dot_writer.set_node_text(n->node_name, quad_lines);

            // print edges
            for (auto &s : n->successors) {
//                std::cout << n->node_name << " -> " << s->node_name << std::endl;
                dot_writer.add_edge(n->node_name, s->node_name, s->lbl_name.value_or(""));
            }
        }
    }

    //    dot_writer.write_dot_to_file("mydotfile.dot");
    dot_writer.render_to_file(filename);
    system(filename.c_str());
}


void print_quads(const std::vector<Quad> &quads, std::map<int, std::string> &labels_rev) {
    for (int i = 0; i < quads.size(); i++) {
        if (auto lbl = labels_rev.find(i); lbl != labels_rev.end())
            std::cout << lbl->second << ": \n";
        std::cout << "  " << quads[i].fmt() << std::endl;
    }
}

auto get_leading_quads_indices(const std::vector<Quad> &quads,
                               std::map<int, std::string> &labels_rev) -> std::map<int, std::optional<std::string>> {
    std::map<int, std::optional<std::string>> leader_indexes = {{0, std::nullopt}};
    for (int i = 0; i < quads.size(); i++) {
        if (auto lbl = labels_rev.find(i); lbl != labels_rev.end())
            leader_indexes.insert({i, std::nullopt});
        if (quads[i].is_jump())
            leader_indexes.insert({i + 1, quads[i].dest.value().dest_name});

    }
    return leader_indexes;
}

auto get_basicblocks_from_indices(const std::vector<Quad> &quads,
                                  std::map<int, std::string> &labels_rev,
                                  std::map<int, std::optional<std::string>> &leader_indexes) -> std::vector<std::unique_ptr<BasicBlock>> {
    std::vector<std::unique_ptr<BasicBlock>> nodes;
    BasicBlock *curr_node = nullptr;
    for (int i = 0, node_number = 0; i <= quads.size(); i++) {
        // if current quad is a leader
        if (auto leader_index = leader_indexes.find(i); leader_index != leader_indexes.end()) {
            if (curr_node != nullptr) {
                curr_node->jumps_to = leader_index->second;
                nodes.emplace_back(curr_node);
            }
            curr_node = new BasicBlock();
            curr_node->id = node_number++;
            curr_node->node_name = curr_node->get_name();

            if (auto lbl = labels_rev.find(i); lbl != labels_rev.end())
                curr_node->lbl_name = lbl->second;
        }
        if (i < quads.size())
            curr_node->quads.push_back(quads[i]);
    }
    if (curr_node && !curr_node->quads.empty()) nodes.emplace_back(curr_node);
    return nodes;
}

void add_successors(std::vector<std::unique_ptr<BasicBlock>> &nodes) {
    for (int i = 0; i < nodes.size(); ++i) {
        if (i != nodes.size() - 1 && nodes[i]->allows_fallthrough()) {
            nodes[i]->successors.insert(nodes[i + 1].get());
            nodes[i + 1]->predecessors.insert(nodes[i].get());
        }
        if (nodes[i]->jumps_to.has_value()) {
            auto jump_to = nodes[i]->jumps_to.value();
            auto node = std::find_if(nodes.begin(), nodes.end(), [&jump_to](auto &e) {
                return e->lbl_name.has_value() && e->lbl_name.value() == jump_to;
            });

            nodes[i]->successors.insert(node->get());
            node->get()->predecessors.insert(nodes[i].get());
        }
    }
}

void print_nodes(const std::vector<std::unique_ptr<BasicBlock>> &nodes) {
    for (auto &n : nodes) {
        std::cout << "\tBasicBlock: " << n->node_name << "; "
                  << n->lbl_name.value_or("NONE")
                  << "; Jumps to " << n->jumps_to.value_or("NONE") << "; Successors: " << n->successors.size()
                  << " \n" << n->fmt() << std::endl;
    }
}


void live_analyses(std::vector<std::unique_ptr<BasicBlock>> &blocks) {

    struct BlockLiveState {
        std::set<std::string> UEVar;
        std::set<std::string> VarKill;
        std::set<std::string> LiveOut;
    };
    std::map<int, BlockLiveState> block_live_states;

    // save UEVar and VarKill
    for (auto &b: blocks) {
        BlockLiveState b_state;

        for (const auto &q : b->quads) {
            for (auto &r: q.get_rhs(false))
                if (b_state.VarKill.find(r) == b_state.VarKill.end())
                    b_state.UEVar.emplace(r);

            if (auto lhs = q.get_lhs(); lhs.has_value())
                b_state.VarKill.insert(lhs.value());
        }

        block_live_states[b->id] = b_state;
    }

    auto live_out = [&block_live_states](BasicBlock *b) {

        std::set<std::string> &live_out_state = block_live_states.at(b->id).LiveOut;
        auto prev_live_out_state = live_out_state;

        for (const auto &s : b->successors) {
            auto &state = block_live_states.at(s->id);

            std::set_union(live_out_state.begin(), live_out_state.end(), state.UEVar.begin(), state.UEVar.end(),
                           std::inserter(live_out_state, live_out_state.end()));

            std::set<std::string> live_without_varkill;
            std::set_difference(state.LiveOut.begin(), state.LiveOut.end(), state.VarKill.begin(), state.VarKill.end(),
                                std::inserter(live_without_varkill, live_without_varkill.end()));

            std::set_union(live_out_state.begin(), live_out_state.end(), live_without_varkill.begin(),
                           live_without_varkill.end(), std::inserter(live_out_state, live_out_state.end()));

        }

        return prev_live_out_state != live_out_state;
    };


    bool changed = true;
    int iter = 1;
    while (changed) {
        changed = false;
        for (const auto &b : blocks) {
            if (live_out(b.get()))
                changed = true;
        }


    }
    std::cout << "Iteration " << iter++ << std::endl;
    // print
    for (auto &[i, b] : block_live_states) {
        std::cout << "Liveout for BB " << i << ": ";
        for (auto &a : b.LiveOut) {
            std::cout << a << "; ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

}


void print_dominator_tree(std::map<int, BasicBlock *> &id_to_block, int entry_id,
                          std::map<int, int> &id_to_immediate_dominator) {

    DotWriter writer;

    for (auto &[block_id, block] : id_to_block) {
        for (auto &[id1, id2] : id_to_immediate_dominator) {
            if (id2 == block_id) {
                // blocks[id1] and blocks[block_id] are connected
                auto name1 = id_to_block.at(block_id)->get_name();
                auto name2 = id_to_block.at(id1)->get_name();
                writer.set_node_name(name1, name1);
                writer.set_node_name(name2, name2);
                writer.set_node_text(name1, {});
                writer.set_node_text(name2, {});
                writer.add_edge(name1, name2);
            }
        }
    }

    writer.render_to_file("dominator_tree.png");
    system("dominator_tree.png");
}


void dominators(std::vector<std::unique_ptr<BasicBlock>> &blocks) {
    std::map<int, std::set<int>> id_to_dominator;
    std::map<int, BasicBlock *> id_to_block;

    std::set<int> N;
    for (auto &b : blocks) {
        N.insert(b->id);
    }


    for (auto &b : blocks) {
        id_to_dominator[b->id] = N;
        id_to_block[b->id] = b.get();
    }

    int iterations = 0;
    bool changed = true;
    while (changed) {
        iterations++;
        changed = false;
        for (auto &b : blocks) {
            // get intersection of sets of predecessors
            std::set<int> pred_intersect;
            for (auto &pred : b->predecessors) {
                if (id_to_dominator.find(pred->id) == id_to_dominator.end()) continue;
                auto pred_dominators = id_to_dominator.at(pred->id);
                if (!pred_intersect.empty()) {
                    std::set<int> intersection;
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

    // now find immediate dominator for every node
    std::map<int, int> id_to_immediate_dominator;
    for (auto &[id, doms] : id_to_dominator) {
        auto current_node = id_to_block.at(id);
        std::set<int> visited{};
        while (true) {
            if (current_node->id != id && doms.find(current_node->id) != doms.end()) {
                id_to_immediate_dominator[id] = current_node->id;
                break;
            } else if (current_node->predecessors.empty()) {
                id_to_immediate_dominator[id] = -1;
                break;
            } else {
                for (auto &n : current_node->predecessors) {
                    if (visited.find(n->id) == visited.end()) {
                        visited.insert(n->id);
                        current_node = n;
                        break;
                    }
                }
            }
        }
    }

    // compute dominance frontier for every node
    std::map<int, std::set<int>> id_to_dominance_frontier;
    for (const auto &b : blocks) {
        id_to_dominance_frontier[b->id] = {};
    }
    for (const auto &b : blocks) {
        if (b->predecessors.size() > 1) {
            for (const auto &pred: b->predecessors) {
                int runner_id = pred->id;
                while (runner_id != id_to_immediate_dominator.at(b->id)) {
                    id_to_dominance_frontier[runner_id].insert(b->id);
                    runner_id = id_to_immediate_dominator.at(runner_id);
                }
            }
        }
    }

    std::cout << "-- PRINT --" << std::endl;
    for (auto &[id, doms] : id_to_dominator) {
        std::cout << id << "(" << id_to_block.at(id)->node_name << "): ";
        for (auto &d : doms) {
            std::cout << d << ", ";
        }
        std::cout << "\t IDom: " << id_to_immediate_dominator.at(id);

        std::cout << "; DomFrontier: ";
        for (auto &df : id_to_dominance_frontier.at(id)) {
            std::cout << id_to_block.at(df)->get_name() << ", ";
        }

        std::cout << std::endl;
    }
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "-- END_PRINT --" << std::endl;


    // finding global names
    std::map<std::string, std::set<BasicBlock *>> var_to_block;
    std::set<std::string> global_names;
    for (auto &b: blocks) {
        std::set<std::string> var_kill;
        for (const auto &q : b->quads) {
            for (auto &r: q.get_rhs(false))
                if (var_kill.find(r) == var_kill.end())
                    global_names.insert(r);
            if (auto lhs = q.get_lhs(); lhs.has_value()) {
                var_kill.insert(lhs.value());
                var_to_block[lhs.value()].insert(b.get());
            }
        }
    }

    // placing phi functions
    for (auto &name : global_names) {
        std::vector<BasicBlock *> work_list;
        auto &work_list_set = var_to_block.at(name);
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
    for (auto &name: global_names) {
        name_to_counter[name] = 0;
        name_to_stack[name] = {};
    }


    auto new_name = [&](std::string name) {
        int i = name_to_counter.at(name);
        name_to_counter.at(name)++;
        name_to_stack.at(name).push_back(i);
        return name + "." + std::to_string(i);
    };


    std::function<void(int)> rename = [&](int block_id) {
        std::vector<std::string> pushed_names;
        auto block = id_to_block.at(block_id);

        // rename phi functions
        for (int i = 0; i < block->phi_functions; ++i) {
            auto &phi = block->quads[i];
            pushed_names.push_back(phi.dest->dest_name);
            phi.dest->dest_name = new_name(phi.dest->dest_name);
        }

        // rename other operations of form 'x = y + z'
        for (int i = block->phi_functions; i < block->quads.size(); ++i) {
            auto &q = block->quads[i];
            if (auto op1 = q.get_op(0); op1 && op1->is_var() && name_to_stack.find(op1->value) != name_to_stack.end()) {
                q.ops[0].value += "." + std::to_string(name_to_stack.at(op1->value).back());
            }
            if (auto op2 = q.get_op(1); op2 && op2->is_var() && name_to_stack.find(op2->value) != name_to_stack.end()) {
                q.ops[1].value += "." + std::to_string(name_to_stack.at(op2->value).back());
            }
            if (q.dest && global_names.find(q.dest->dest_name) != global_names.end()) {
                pushed_names.push_back(q.dest->dest_name);
                q.dest->dest_name = new_name(q.dest->dest_name);
            }
        }

        // fill phi function parameters for every successor
        for (auto &s : block->successors) {
            for (int i = 0; i < s->phi_functions; ++i) {
                auto &phi = s->quads[i];
                auto name = phi.dest.value().dest_name;
                auto name_without_dot = name.substr(0, name.find_first_of('.', 0));

                std::string next_name;
                if (name_to_stack.find(name_without_dot) != name_to_stack.end() &&
                    !name_to_stack.at(name_without_dot).empty()) {
                    next_name = name_without_dot + "." + std::to_string(name_to_stack.at(name_without_dot).back());
                } else {
                    next_name = name;
                }
                Operand op(next_name, Operand::Type::Var);
                op.payload = block->get_name();
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

    int entry_block_id = 0;
    rename(entry_block_id);


    print_dominator_tree(id_to_block, 0, id_to_immediate_dominator);
}


void make_cfg(std::map<std::string, int> &&labels, std::vector<Quad> &&quads) {
    // revert map of labels
    std::map<int, std::string> labels_rev;
    for (auto &[a, b] : labels) {
        labels_rev.emplace(b, a);
    }

//    print_quads(quads, labels_rev);

    // gather indexes of leading blocks
    auto leader_indexes = get_leading_quads_indices(quads, labels_rev);
    auto blocks = get_basicblocks_from_indices(quads, labels_rev, leader_indexes);
    add_successors(blocks);

//    for (auto &[id, str] : leader_indexes) {
//        std::cout << "ID: " << id << "; JUMPS TO: " << str.value_or("NOWHERE") << std::endl;
//    }
//    std::cout << std::endl;
//    for (auto &b : blocks) {
//        std::cout << "ID: " << b->id << "; JUMPS TO: " << b->jumps_to.value_or("NOWHERE") << std::endl;
//    }

//    print_nodes(blocks);
//    print_cfg(blocks, "before.png");


//    remove_blocks_without_predecessors(blocks);
//    add_exit_block(blocks);
//    constant_folding(blocks);
//    liveness_analyses(blocks);
//    print_loops(blocks);


//    print_cfg(blocks, "before.png");
//
//    for (auto &n : blocks) {
//        ValueNumberTableStack t;
//        t.push_table();
//        local_value_numbering(n->quads, t);
//    }

//    live_analyses(blocks);

//    superlocal_value_numbering(blocks);


    dominators(blocks);

    print_cfg(blocks, "after.png");
}
