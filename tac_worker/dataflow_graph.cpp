//
// Created by shoco on 10/7/2020.
//

#include <numeric>
#include "dataflow_graph.hpp"



void constant_folding(BasicBlocks &nodes);

void liveness_analyses(const BasicBlocks &nodes);

void remove_blocks_without_predecessors(BasicBlocks &nodes);

BasicBlock *find_root_node(BasicBlocks &blocks) {
    for (auto &b:blocks)
        if (b->predecessors.empty())
            return b.get();
    return nullptr;
}


void add_entry_and_exit_block(BasicBlocks &nodes) {
    // assume there is already one and only one entry block
    // you dont need to add another one
//    auto entry_block = std::make_unique<BasicBlock>();
//    entry_block->node_name = "Entry block";
//    entry_block->id = -1;
//    entry_block->add_successor(nodes.front().get());
//    nodes.insert(nodes.begin(), std::move(entry_block));

    // find blocks without successors (ending blocks) and connect them with exit block
    // if there are more than 1
    std::vector<BasicBlock *> ending_blocks;
    for (auto &n : nodes)
        if (n->successors.empty())
            ending_blocks.emplace_back(n.get());

    if (ending_blocks.size() > 1) {
        auto exit_block = std::make_unique<BasicBlock>();
        exit_block->node_name = "Exit block";
        exit_block->id = nodes.back()->id + 1;
        exit_block->lbl_name = "EXIT_BLOCK";
        exit_block->quads.push_back(Quad("0", {}, Quad::Type::Return));

        for (auto &e : ending_blocks) {
//            Dest dest(exit_block->lbl_name.value(), {}, Dest::Type::JumpLabel);
//            e->quads.push_back( Quad({}, {}, Quad::Type::Goto, dest) );
            e->add_successor(exit_block.get());
        }

        nodes.push_back(std::move(exit_block));
    }
}

void add_missing_jumps(BasicBlocks &blocks) {
    for (int i = 0; i < blocks.size() - 1; ++i) {
        auto &last_q = blocks[i]->quads.back();
        if (!last_q.is_jump() && !blocks[i]->successors.empty()) {
            Dest dest((*blocks[i]->successors.begin())->lbl_name.value(), {}, Dest::Type::JumpLabel);
            Quad jump({}, {}, Quad::Type::Goto, dest);
            blocks[i]->quads.push_back(jump);
        }
    }
}

void reverse_graph(BasicBlocks &blocks) {
    for (auto &b : blocks)
        std::swap(b->successors, b->predecessors);
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

    std::cout << "LOOPS: " << std::endl;
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

BasicBlocks get_basic_blocks_from_indices(const std::vector<Quad> &quads,
                                          std::map<int, std::string> &labels_rev,
                                          std::map<int, std::optional<std::string>> &leader_indexes) {
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

void add_initial_successors(BasicBlocks &nodes) {
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

void print_blocks(const BasicBlocks &blocks) {
    for (auto &n : blocks) {
        std::cout << "\tBasicBlock: " << n->node_name << "; "
                  << n->lbl_name.value_or("NONE")
                  << "; Jumps to " << n->jumps_to.value_or("NONE")
                  << "; Successors: " << n->successors.size()
                  << "; Predecessors: " << n->predecessors.size()
                  << " \n" << n->fmt();
    }
}


void live_analyses(BasicBlocks &blocks) {

    struct BlockLiveState {
        std::set<std::string> UEVar; // upward exposed variables
        std::set<std::string> VarKill; // killed (re-assigned) variables
        std::set<std::string> LiveOut; // 'live' variables
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
    int iter = 0;
    while (changed) {
        iter++;
        changed = false;
        for (const auto &b : blocks) {
            if (live_out(b.get()))
                changed = true;
        }
    }

    // print
    std::cout << "Iterations: " << iter << std::endl;
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
                        Dest dest(split_block->lbl_name.value(), {}, Dest::Type::JumpLabel);
                        pred->quads.push_back(Quad({}, {}, Quad::Type::Goto, dest));
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

        b->quads.erase(b->quads.begin(), b->quads.begin() + b->phi_functions);
        b->phi_functions = 0;
    }

    blocks.reserve(blocks.size() + new_blocks.size());
    std::move(std::begin(new_blocks), std::end(new_blocks), std::back_inserter(blocks));
}


ID2DOM find_dominators(const BasicBlocks &blocks) {
    std::map<int, std::set<int>> id_to_dominator;

    std::set<int> N;
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

    return id_to_dominator;
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

    for (auto &b : blocks) {
        if (b->predecessors.empty())
            postorder_traversal(b.get());
    }
    for (auto &b : blocks) {
        postorder_traversal(b.get());
    }

    for (auto &b : blocks) {
        block_id_to_rpo[b->id] = counter - 1 - block_id_to_rpo.at(b->id);
        b->node_name = b->get_name() + "; RPO: " + std::to_string(block_id_to_rpo.at(b->id));
    }
    return block_id_to_rpo;
}

ID2IDOM find_immediate_dominators(BasicBlocks &blocks, ID2Block &id_to_block) {
    std::map<int, int> id_to_rpo = generate_reverse_post_order_numbers(blocks);
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

    auto entry_node_id = find_root_node(blocks)->id;
//    auto entry_node_id = rpo_to_id.at(0);
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
            while (preds != b->predecessors.end() && id_to_rpo.at((*preds)->id) > rpo)
                std::advance(preds, 1);
            if (preds == b->predecessors.end()) continue;
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

    bool graph_is_irreducible = iterations > 2;
//    std::cout << __PRETTY_FUNCTION__ << std::endl;
//    std::cout << "Iterations: " << iterations << std::endl;
//    for (auto &[id, idom_id] : id_to_idom) {
//        std::cout << id_to_block.at(id)->node_name << " -> " << id_to_block.at(idom_id)->node_name << std::endl;
//    }

    // entry node have no idom
    id_to_idom.erase(entry_node_id);
    return id_to_idom;
}

ID2DF find_dominance_frontier(const BasicBlocks &blocks, ID2IDOM &id_to_immediate_dominator) {
    std::map<int, std::set<int>> id_to_dominance_frontier;
    for (const auto &b : blocks)
        id_to_dominance_frontier[b->id] = {};
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
    return id_to_dominance_frontier;
}


void place_phi_functions(BasicBlocks &blocks, ID2Block &id_to_block, ID2IDOM &id_to_immediate_dominator,
                         ID2DF &id_to_dominance_frontier,
                         std::map<std::string, std::set<BasicBlock *>> &var_to_blocks,
                         std::set<std::string> &global_names,
                         std::set<std::string> &all_names) {

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
    for (auto &name: all_names) {
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
            if (auto op1 = q.get_op(0); op1 && op1->is_var() && name_to_stack.find(op1->value) != name_to_stack.end()) {
                q.ops[0].value += "." + std::to_string(name_to_stack.at(op1->value).back());
            }
            if (auto op2 = q.get_op(1); op2 && op2->is_var() && name_to_stack.find(op2->value) != name_to_stack.end()) {
                q.ops[1].value += "." + std::to_string(name_to_stack.at(op2->value).back());
            }
            if (q.dest && all_names.find(q.dest->name) != all_names.end()) {
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

    rename(find_root_node(blocks)->id);
}

void convert_to_ssa(BasicBlocks &blocks, ID2Block &id_to_block) {

    ID2DOM id_to_dominators = find_dominators(blocks);
    ID2IDOM id_to_immediate_dominator = find_immediate_dominators(blocks, id_to_block);
    ID2DF id_to_dominance_frontier = find_dominance_frontier(blocks, id_to_immediate_dominator);

//    std::cout << "-- PRINT --" << std::endl;
//    for (auto &[id, doms] : id_to_dominators) {
//        std::cout << id << "(" << id_to_block.at(id)->node_name << "): ";
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


    // find global names
    std::set<std::string> all_names;
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
                all_names.insert(lhs.value());
            }
        }
    }

//    std::cout << "Global names: ";
//    for (auto &name : global_names) {
//        std::cout << name << ", ";
//    }
//    std::cout << std::endl;

    place_phi_functions(blocks, id_to_block, id_to_immediate_dominator, id_to_dominance_frontier, var_to_block,
                        global_names, all_names);
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


std::map<int, int> generate_post_order_numbers(BasicBlocks &blocks) {
    reverse_graph(blocks);
    std::map<int, int> id_to_post_order = generate_reverse_post_order_numbers(blocks);
    reverse_graph(blocks);

    return id_to_post_order;
}


void useless_code_elimination(BasicBlocks &blocks, ID2Block &id_to_block, ID2DF &id_to_rev_df, ID2IDOM &id_to_idom) {

    struct Place {
        int block_num;
        int quad_num;

        bool operator<(const Place &o) const {
            return std::tie(block_num, quad_num) < std::tie(o.block_num, o.quad_num);
//            return (block_num < o.quad_num) || (block_num == o.block_num && quad_num < o.quad_num);
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
            for (int i = 0; i < blocks.size(); ++i) if (blocks[i]->id == b_id) dom_block_num = i;
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
            if (block_it == id_to_block.end()) continue;
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
                    for (auto &s : succ->successors) b->add_successor(s);
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


void make_cfg(std::map<std::string, int> &&labels, std::vector<Quad> &&quads) {
    // revert map of labels
    std::map<int, std::string> labels_rev;
    for (auto &[a, b] : labels) labels_rev.emplace(b, a);

//    print_quads(quads, labels_rev);

    // gather indexes of leading blocks
    auto leader_indexes = get_leading_quads_indices(quads, labels_rev);
    auto blocks = get_basic_blocks_from_indices(quads, labels_rev, leader_indexes);
    add_initial_successors(blocks);
    add_missing_jumps(blocks);
    add_entry_and_exit_block(blocks);

    ID2Block id_to_block;
    for (auto &b : blocks) id_to_block[b->id] = b.get();

//    for (auto &[id, str] : leader_indexes) {
//        std::cout << "ID: " << id << "; JUMPS TO: " << str.value_or("NOWHERE") << std::endl;
//    }
//    std::cout << std::endl;
//    for (auto &b : blocks) {
//        std::cout << "ID: " << b->id << "; JUMPS TO: " << b->jumps_to.value_or("NOWHERE") << std::endl;
//    }

//    print_blocks(blocks);
//    print_cfg(blocks, "before.png");

//    remove_blocks_without_predecessors(blocks);
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


//    convert_to_ssa(blocks, id_to_block);
//    print_cfg(blocks, "before.png");
//    sparse_simple_constant_propagation(blocks);
//    remove_phi_functions(blocks, id_to_block, 0);

    reverse_graph(blocks);
    auto id_to_rev_idom = find_immediate_dominators(blocks, id_to_block);
    ID2DF revDF = find_dominance_frontier(blocks, id_to_rev_idom);
    reverse_graph(blocks);

//    for (auto &[id, df] : revDF) {
//        std::cout << "DF: " << id_to_block.at(id)->node_name << " || ";
//        for (auto &d : df) {
//            std::cout << id_to_block.at(d)->node_name << ", ";
//        }
//        std::cout << std::endl;
//    }

    convert_to_ssa(blocks, id_to_block);
//    sparse_simple_constant_propagation(blocks);


//    useless_code_elimination(blocks, id_to_block, revDF, id_to_rev_idom);

    // find_immediate_dominators(blocks, id_to_block);
//    print_dominator_tree(id_to_block, 0, id_to_idom);

    print_cfg(blocks, "before.png");

    ID2IDOM id_to_idom = find_immediate_dominators(blocks, id_to_block);

    dominator_based_value_numbering(blocks, id_to_block, id_to_idom);

    print_cfg(blocks, "after.png");
}

