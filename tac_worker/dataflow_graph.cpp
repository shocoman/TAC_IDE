//
// Created by shoco on 10/7/2020.
//

#include <numeric>
#include "dataflow_graph.hpp"

using BasicBlocks = std::vector<std::unique_ptr<BasicBlock>>;
using ID2Block = std::map<int, BasicBlock *>;
using ID2IDOM = std::map<int, int>;


void constant_folding(BasicBlocks &nodes);

void liveness_analyses(const BasicBlocks &nodes);

void remove_blocks_without_predecessors(BasicBlocks &nodes);


void add_exit_block(BasicBlocks &nodes) {
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


void print_loops(BasicBlocks &nodes) {

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


void remove_blocks_without_predecessors(BasicBlocks &nodes) {
    for (int i = nodes.size() - 1; i > 0; --i) {
        auto &n = nodes[i];
        if (n->predecessors.empty()) {
            n->remove_successors();
            nodes.erase(nodes.begin() + i);
        }
    }
}

[[deprecated]]
void liveness_analyses(const BasicBlocks &nodes) {
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
//                std::string lhs = q.dest.value().name;
//                auto &l = block_liveness_data[i];
//                if (!l.at(lhs).live) {
//                    n->quads.erase(n->quads.begin() + i);
//                }
//            }
//        }


    }
}

void constant_folding(BasicBlocks &nodes) {
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


void print_cfg(const BasicBlocks &nodes, const std::string &filename) {
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
            leader_indexes.insert({i + 1, quads[i].dest.value().name});

    }
    return leader_indexes;
}

auto get_basicblocks_from_indices(const std::vector<Quad> &quads,
                                  std::map<int, std::string> &labels_rev,
                                  std::map<int, std::optional<std::string>> &leader_indexes) -> BasicBlocks {
    BasicBlocks nodes;
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

void add_successors(BasicBlocks &nodes) {
    for (int i = 0; i < nodes.size(); ++i) {
        if (i != nodes.size() - 1 && nodes[i]->allows_fallthrough()) {
            nodes[i]->add_successor(nodes[i + 1].get());
        }
        if (nodes[i]->jumps_to.has_value()) {
            auto jump_to = nodes[i]->jumps_to.value();
            auto node = std::find_if(nodes.begin(), nodes.end(), [&jump_to](auto &e) {
                return e->lbl_name.has_value() && e->lbl_name.value() == jump_to;
            });

            nodes[i]->add_successor(node->get());
        }
    }
}

void print_nodes(const BasicBlocks &nodes) {
    for (auto &n : nodes) {
        std::cout << "\tBasicBlock: " << n->node_name << "; "
                  << n->lbl_name.value_or("NONE")
                  << "; Jumps to " << n->jumps_to.value_or("NONE") << "; Successors: " << n->successors.size()
                  << " \n" << n->fmt() << std::endl;
    }
}


void live_analyses(BasicBlocks &blocks) {

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


void print_dominator_tree(ID2Block &id_to_block, int entry_id, ID2IDOM &id_to_idom) {

    DotWriter writer;
    for (auto &[id1, id2] : id_to_idom) {
        // make a connection between blocks[id1] and blocks[block_id]
        auto name1 = id_to_block.at(id2)->node_name;
        auto name2 = id_to_block.at(id1)->node_name;
        writer.set_node_name(name1, name1);
        writer.set_node_name(name2, name2);
        writer.set_node_text(name1, {});
        writer.set_node_text(name2, {});
        writer.add_edge(name1, name2);
    }

    writer.render_to_file("dominator_tree.png");
    system("dominator_tree.png");
}


void remove_phi_functions(BasicBlocks &blocks,
                          std::map<int, BasicBlock *> &id_to_block, int entry_id) {

    BasicBlocks new_blocks;

    for (auto &b : blocks) {
        std::map<std::pair<int, int>, int> replace_block_id;
        for (int i = 0; i < b->phi_functions; ++i) {
            auto &phi = b->quads[i];
            for (auto &op : phi.ops) {
                if (replace_block_id.find({op.predecessor_id, b->id}) == replace_block_id.end()
                    && id_to_block.at(op.predecessor_id)->successors.size() > 1) {
                    auto *pred = id_to_block.at(op.predecessor_id);

                    auto split_block = std::make_unique<BasicBlock>();
                    split_block->quads.push_back(Quad({}, {}, Quad::Type::Goto,
                                                      Dest(b->lbl_name.value(), {}, Dest::Type::JumpLabel)));

                    if (!new_blocks.empty()) split_block->id = new_blocks.back()->id + 1;
                    else split_block->id = blocks.back()->id + 1;
                    split_block->node_name = split_block->get_name();
                    split_block->lbl_name = split_block->get_name();

                    id_to_block[split_block->id] = split_block.get();
                    replace_block_id[{op.predecessor_id, b->id}] = split_block->id;

                    pred->successors.erase(b.get());
                    b->predecessors.erase(pred);

                    if (pred->quads.back().dest->name == b->lbl_name) {
                        pred->quads.back().dest->name = split_block->lbl_name.value();
                    } else {
                        pred->quads.push_back(Quad({}, {}, Quad::Type::Goto,
                                                   Dest(split_block->lbl_name.value(), {}, Dest::Type::JumpLabel)));
                    }

                    pred->add_successor(split_block.get());
                    split_block->add_successor(b.get());

                    new_blocks.push_back(std::move(split_block));
                }

                BasicBlock *pred = nullptr;
                if (replace_block_id.find({op.predecessor_id, b->id}) != replace_block_id.end()) {
                    pred = id_to_block.at(replace_block_id.at({op.predecessor_id, b->id}));
                } else {
                    pred = id_to_block.at(op.predecessor_id);
                }
                auto insert_pos = pred->quads.back().is_jump() ? pred->quads.end() - 1 : pred->quads.end();
                pred->quads.insert(insert_pos, Quad(op.value, {}, Quad::Type::Assign, phi.dest.value()));
            }
        }


//        for (int i = 0; i < b->phi_functions; ++i) {
//            auto &phi = b->quads[i];
//            for (auto &op : phi.ops) {
//                auto *pred = id_to_block.at(op.predecessor_id);
//
//                auto insert_pos = pred->quads.back().is_jump() ? pred->quads.end() - 1 : pred->quads.end();
//                if (pred->successors.size() == 1) {
//                    pred->quads.insert(insert_pos, Quad(op.value, {}, Quad::Type::Assign, phi.dest.value()));
//                } else if (pred->successors.size() > 1) {
//
//                    auto split_block = std::make_unique<BasicBlock>();
//                    if (!new_blocks.empty()) split_block->id = new_blocks.back()->id + 1;
//                    else split_block->id = blocks.back()->id + 1;
//                    split_block->node_name = split_block->get_name();
//                    id_to_block[split_block->id] = split_block.get();
//
//
//                    pred->successors.erase(b.get());
//                    b->predecessors.erase(pred);
//
//                    pred->add_successor(split_block.get());
//                    split_block->add_successor(b.get());
////
////                    split_block->quads.push_back(Quad(op.value, {}, Quad::Type::Assign, phi.dest.value()));
//                    new_blocks.push_back(std::move(split_block));
//                }
//            }
//        }

        b->quads.erase(b->quads.begin(), b->quads.begin() + b->phi_functions);
        b->phi_functions = 0;

    }

//    blocks.insert(blocks.end(), new_blocks.begin(), new_blocks.end());
    blocks.reserve(blocks.size() + new_blocks.size());
    std::move(std::begin(new_blocks), std::end(new_blocks), std::back_inserter(blocks));
}


void dominators(BasicBlocks &blocks, ID2Block &id_to_block) {
    std::map<int, std::set<int>> id_to_dominator;

    std::set<int> N;
    for (auto &b : blocks) {
        N.insert(b->id);
    }

    for (auto &b : blocks) {
        id_to_dominator[b->id] = N;
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

//            for (auto &r: q.get_rhs(false))
//                if (var_kill.find(r) == var_kill.end())
//                    global_names.insert(r);
//
//            if (auto lhs = q.get_lhs(); lhs.has_value()) {
//                global_names.insert(lhs.value());
//                var_to_block[lhs.value()].insert(b.get());
//            }
            for (auto &r: q.get_rhs(false))
                if (var_kill.find(r) == var_kill.end())
                    global_names.insert(r);
            if (auto lhs = q.get_lhs(); lhs.has_value()) {
                var_kill.insert(lhs.value());
                var_to_block[lhs.value()].insert(b.get());
            }
        }
    }

    std::cout << "Global names: ";
    for (auto &name : global_names) {
        std::cout << name << ", ";
    }
    std::cout << std::endl;

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
            pushed_names.push_back(phi.dest->name);
            phi.dest->name = new_name(phi.dest->name);
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
            if (q.dest && global_names.find(q.dest->name) != global_names.end()) {
                pushed_names.push_back(q.dest->name);
                q.dest->name = new_name(q.dest->name);
            }
        }

        // fill phi function parameters for every successor
        for (auto &s : block->successors) {
            for (int i = 0; i < s->phi_functions; ++i) {
                auto &phi = s->quads[i];
                auto name = phi.dest.value().name;
                auto name_without_dot = name.substr(0, name.find_first_of('.', 0));

                std::string next_name;
                if (name_to_stack.find(name_without_dot) != name_to_stack.end() &&
                    !name_to_stack.at(name_without_dot).empty()) {
                    next_name = name_without_dot + "." + std::to_string(name_to_stack.at(name_without_dot).back());
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

    int entry_block_id = 0;
    rename(entry_block_id);


//    print_dominator_tree(id_to_block, 0, id_to_immediate_dominator);

//    print_cfg(blocks, "before.png");
//    remove_phi_functions(blocks, id_to_block, 0);
//    print_cfg(blocks, "after.png");
}


void sparse_simple_constant_propagation(BasicBlocks &blocks) {
    struct Place {
        int block_num;
        int quad_num;
    };

    struct VarInfo {
        enum class ValueType {
            Bottom, Constant, Top
        };

        Place defined_at = {-1, -1};
        std::vector<Place> used_at{};

        ValueType value_type = ValueType::Top;
        Operand constant;
    };


    using ValType = VarInfo::ValueType;
    using VarName = std::string;
    std::map<VarName, VarInfo> usingInfo;

    for (auto b_index = 0; b_index < blocks.size(); ++b_index) {
        auto &b = blocks[b_index];
        for (int i = 0; i < b->quads.size(); ++i) {
            auto &q = b->quads[i];
            if (!q.dest.has_value() || q.dest->type == Dest::Type::None) continue;

            Place place{b_index, i};
//            if (!q.is_jump())
            usingInfo[q.dest->name].defined_at = place;

            if (Quad::is_foldable(q.type))
                constant_folding(q);

            for (const auto &op : q.ops) {
                if (op.is_var())
                    usingInfo[op.value].used_at.push_back(place);
            }
        }
    }


    // initialization phase
    std::vector<std::string> work_list;
    for (auto &[name, useInfo] : usingInfo) {
        if (useInfo.defined_at.block_num == -1) continue;

        auto &q = blocks.at(useInfo.defined_at.block_num)->quads.at(useInfo.defined_at.quad_num);

        if (q.type == Quad::Type::PhiNode) {
            useInfo.value_type = ValType::Top;
        } else if (q.type == Quad::Type::Assign && q.get_op(0)->is_constant()) {
            useInfo.value_type = ValType::Constant;
            useInfo.constant = q.get_op(0).value();
        } else {
            useInfo.value_type = ValType::Top;
        }

        if (useInfo.value_type != ValType::Top) {
            work_list.push_back(name);
        }
    }


    // propagation phase
    while (!work_list.empty()) {
        std::string name = work_list.back();
        work_list.pop_back();

        for (auto &[block_num, quad_num] : usingInfo.at(name).used_at) {
            auto &q = blocks.at(block_num)->quads.at(quad_num);
            if (q.is_jump()) continue;

            auto &lhs = usingInfo.at(q.dest->name);
            if (lhs.value_type != ValType::Bottom) {
                auto tmp = std::pair{lhs.value_type, lhs.constant};
                auto &lhs_q = blocks.at(lhs.defined_at.block_num)->quads.at(lhs.defined_at.quad_num);

                lhs.value_type = ValType::Top;
                lhs.constant = Operand();

                // interpretation over lattice
                if (lhs_q.type == Quad::Type::PhiNode) {
                    for (auto &op : lhs_q.ops) {
                        if (lhs.value_type == ValType::Bottom) break;

                        auto &var = usingInfo.at(op.value);
                        if (var.value_type == ValType::Top) {}
                        else if (var.value_type == ValType::Constant
                                 && (lhs.value_type == ValType::Top ||
                                     lhs.value_type == ValType::Constant &&
                                     lhs.constant.value == var.constant.value)) {
                            lhs.value_type = var.value_type;
                            lhs.constant = var.constant;
                        } else {
                            lhs.value_type = ValType::Bottom;
                        }
                    }
                } else {
                    //  if (lhs_q.ops.size() >= 2)

                    auto fst = lhs_q.get_op(0);
                    auto snd = lhs_q.get_op(1);
                    if (fst && fst->is_var() && usingInfo.at(fst->value).value_type == ValType::Constant)
                        fst = Operand(usingInfo.at(fst->value).constant);
                    if (snd && snd->is_var() && usingInfo.at(snd->value).value_type == ValType::Constant)
                        snd = Operand(usingInfo.at(snd->value).constant);

                    auto tmp_q = Quad();
                    tmp_q.ops = {fst.value_or(Operand()), snd.value_or(Operand())};
                    tmp_q.type = lhs_q.type;
                    auto prev = tmp_q;
                    constant_folding(tmp_q);

                    // SOMETHING CHANGED, UPDATE LHS
                    if (tmp_q != prev) {
                        lhs.value_type = ValType::Top;
                        lhs.constant = Operand();

                        // presume that lhs is unary
                        if (tmp_q.ops[0].is_constant()) {
                            lhs.value_type = ValType::Constant;
                            lhs.constant = tmp_q.ops[0];
                        } else {
                            auto saved = usingInfo.at(tmp_q.ops[0].value);
                            lhs.value_type = saved.value_type;
                            lhs.constant = saved.constant;
                        }
                    }
                }

                if (tmp != std::pair{lhs.value_type, lhs.constant}) {
                    work_list.push_back(q.dest->name);
                }
            }
        }
    }



//    for (auto &[name, useInfo] : usingInfo) {
//        std::cout << "Var: " << name << " defined at: (" << useInfo.defined_at.block_num << "; "
//                  << useInfo.defined_at.quad_num << "); " << "used at: ";
//        for (auto &u : useInfo.used_at) {
//            std::cout << "(" << u.block_num << "; " << u.quad_num << "), ";
//        }
//        std::cout << " ValType: " << (int) useInfo.value_type << "; " << useInfo.constant.value << std::endl;
//    }

    for (auto &[name, useInfo] : usingInfo) {
        if (useInfo.value_type == ValType::Constant) {
            auto &q = blocks.at(useInfo.defined_at.block_num)->quads.at(useInfo.defined_at.quad_num);
            q.type = Quad::Type::Assign;
            q.ops[0] = useInfo.constant;
            q.clear_op(1);
        }
    }

    // update phi function positions
    for (auto &b : blocks) {
        b->phi_functions = 0;
        for (int i = 0; i < b->quads.size(); ++i) {
            auto &q = b->quads[i];
            if (q.type == Quad::Type::PhiNode) {
                std::swap(b->quads[i], b->quads[b->phi_functions]);
                b->phi_functions++;
            }
        }
    }
}


std::map<int, int> generate_reverse_post_order_numbers(BasicBlocks &blocks) {
    std::map<int, int> block_id_to_rpo;
    int counter = 0;

    std::function<void(BasicBlock *)> postorder_traversal = [&](BasicBlock *b) {
        if (block_id_to_rpo.find(b->id) == block_id_to_rpo.end()) {
            block_id_to_rpo[b->id] = 0;
            for (auto &s : b->successors)
                postorder_traversal(s);
            block_id_to_rpo[b->id] = counter++;
        }
    };
    postorder_traversal(blocks.front().get());

//    for (auto &[id, rpo] : block_id_to_rpo)
//        std::cout << id << " -> " << (counter - 1 - rpo) << std::endl;

    for (auto &b : blocks) {
        block_id_to_rpo[b->id] = counter - 1 - block_id_to_rpo.at(b->id);
        b->node_name = b->get_name() + "; RPO: " + std::to_string(block_id_to_rpo.at(b->id));
    }
    return block_id_to_rpo;
}


ID2IDOM modified_dominator_algorithm(BasicBlocks &blocks, ID2Block &id_to_block, std::map<int, int> &id_to_rpo) {
    std::map<int, int> rpo_to_id;
    for (auto &[id, rpo] : id_to_rpo) rpo_to_id.emplace(rpo, id);
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

    auto entry_node_id = rpo_to_id.at(0);
    id_to_idom[entry_node_id] = entry_node_id;
    bool changed = true;
    int iterations = 0;
    while (changed) {
        iterations++;
        changed = false;
        for (auto &[rpo, id] : rpo_to_id) {

            auto &b = id_to_block.at(id);
            if (b->predecessors.empty()) continue;

            auto preds = b->predecessors.begin();
            auto new_idom = *preds;
            for (std::advance(preds, 1); preds != b->predecessors.end(); std::advance(preds, 1)) {
                auto p = *preds;
                if (id_to_idom.find(p->id) != id_to_idom.end())
                    new_idom = intersect(p, new_idom);
            }

            if (id_to_idom.find(b->id) == id_to_idom.end() || id_to_idom.at(b->id) != new_idom->id) {
                id_to_idom[b->id] = new_idom->id;
                changed = true;
            }
        }
    }

//    std::cout << "Iterations: " << iterations << std::endl;
//    for (auto &[id, idom_id] : id_to_idom) {
//        std::cout << id_to_block.at(id)->node_name << " -> " << id_to_block.at(idom_id)->node_name << std::endl;
//    }

    return id_to_idom;
}

void make_cfg(std::map<std::string, int> &&labels, std::vector<Quad> &&quads) {
    // revert map of labels
    std::map<int, std::string> labels_rev;
    for (auto &[a, b] : labels) labels_rev.emplace(b, a);

//    print_quads(quads, labels_rev);

    // gather indexes of leading blocks
    auto leader_indexes = get_leading_quads_indices(quads, labels_rev);
    auto blocks = get_basicblocks_from_indices(quads, labels_rev, leader_indexes);
    add_successors(blocks);

    ID2Block id_to_block;
    for (auto &b : blocks) id_to_block[b->id] = b.get();

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


//    dominators(blocks, id_to_block);

//    print_cfg(blocks, "before.png");

//    sparse_simple_constant_propagation(blocks);
//
//    remove_phi_functions(blocks, id_to_block, 0);


    auto id_to_rpo = generate_reverse_post_order_numbers(blocks);
    auto id_to_idom = modified_dominator_algorithm(blocks, id_to_block, id_to_rpo);

    print_dominator_tree(id_to_block, 0, id_to_idom);
    print_cfg(blocks, "after.png");
}

