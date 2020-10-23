//
// Created by shoco on 10/7/2020.
//

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
            if (q.op1.is_number() && q.op2.is_number()) {
                double d = 0;
                double o1 = q.op1.get_double();
                double o2 = q.op2.get_double();
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

                if (q.op1.is_int() && q.op2.is_int())
                    q.op1 = Operand(std::to_string((int) d));
                else
                    q.op1 = Operand(std::to_string(d));
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
        if (auto lbl = labels_rev.find(i); lbl != labels_rev.end()) leader_indexes.insert({i, std::nullopt});
        switch (quads[i].type) {
            case Quad::Type::IfFalse:
            case Quad::Type::IfTrue:
            case Quad::Type::Goto:
                leader_indexes.insert({i + 1, quads[i].dest.value().dest_name});
        }
    }
    return leader_indexes;
}

auto get_basicblocks_from_indices(const std::vector<Quad> &quads,
                                  std::map<int, std::string> &labels_rev,
                                  std::map<int, std::optional<std::string>> &leader_indexes) -> std::vector<std::unique_ptr<BasicBlock>> {
    std::vector<std::unique_ptr<BasicBlock>> nodes;
    BasicBlock *curr_node = nullptr;
    for (int i = 0, node_number = 1; i < quads.size(); i++) {
        // if current quad is a leader
        if (auto leader_index = leader_indexes.find(i); leader_index != leader_indexes.end()) {
            if (curr_node != nullptr) {
                curr_node->jumps_to = leader_index->second;
                nodes.emplace_back(curr_node);
            }
            curr_node = new BasicBlock();
            curr_node->id = node_number;
            curr_node->node_name = "BasicBlock " + std::to_string(node_number++);

            if (auto lbl = labels_rev.find(i); lbl != labels_rev.end())
                curr_node->lbl_name = lbl->second;
        }
        curr_node->quads.push_back(quads[i]);
    }
    if (curr_node) nodes.emplace_back(curr_node);
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

//    print_nodes(blocks);
//    print_cfg(blocks, "before.png");


    // remove blocks without predecessors (except the first one)
    remove_blocks_without_predecessors(blocks);
//    add_exit_block(blocks);
//    constant_folding(blocks);
//    liveness_analyses(blocks);
//    print_loops(blocks);


//    print_cfg(blocks, "before.png");

//    for (auto &n : blocks) {
//        ValueNumberTableStack t;
//        t.push_table();
//        local_value_numbering(n->quads, t);
//    }

    live_analyses(blocks);

//    superlocal_value_numbering(blocks);

    print_cfg(blocks, "after.png");
}
