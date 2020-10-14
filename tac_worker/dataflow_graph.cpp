//
// Created by shoco on 10/7/2020.
//

#include "dataflow_graph.hpp"

void constant_folding(std::vector<std::unique_ptr<Node>> &nodes);

void liveness_analyses(const std::vector<std::unique_ptr<Node>> &nodes);

void remove_blocks_without_predecessors(std::vector<std::unique_ptr<Node>> &nodes);


void add_exit_block(std::vector<std::unique_ptr<Node>> &nodes) {
    auto exit_node = std::make_unique<Node>();
    exit_node->node_name = "Exit block";
    exit_node->quads.emplace_back(Quadruple({}, {}, OperationType::Return));

    // find blocks without successors (ending blocks) and connect them with exit block
    for (auto &n : nodes) {
        if (n->successors.empty()) {
            n->add_successor(exit_node.get());
        }
    }

    nodes.emplace_back(std::move(exit_node));
}

void make_cfg(std::map<std::string, int> &&labels, std::vector<Quadruple> &&quads) {
    // revert map of labels
    std::map<int, std::string> labels_rev;
    for (auto &[a, b] : labels) {
        labels_rev.emplace(b, a);
    }

//    print_quads(quads, labels_rev);

    // gather indexes of leading blocks
    auto leader_indexes = get_leading_quads_indices(quads, labels_rev);
    auto nodes = get_basicblocks_from_indices(quads, labels_rev, leader_indexes);
    add_successors(nodes);

//    print_nodes(nodes);
//    print_cfg(nodes, "before.png");


    // remove blocks without predecessors (except the first one)
    remove_blocks_without_predecessors(nodes);
    add_exit_block(nodes);
//    constant_folding(nodes);
    liveness_analyses(nodes);


    print_cfg(nodes, "after.png");
}

void remove_blocks_without_predecessors(std::vector<std::unique_ptr<Node>> &nodes) {
    for (int i = nodes.size() - 1; i > 0; --i) {
        auto &n = nodes[i];
        if (n->predecessors.empty()) {
            n->remove_successors();
            nodes.erase(nodes.begin() + i);
        }
    }
}

void liveness_analyses(const std::vector<std::unique_ptr<Node>> &nodes) {
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

void constant_folding(std::vector<std::unique_ptr<Node>> &nodes) {
    for (auto &n : nodes) {
        for (auto &q : n->quads) {
            if (q.operand_1.is_number() && q.operand_2.is_number()) {
                double d = 0;
                double o1 = q.operand_1.get_double();
                double o2 = q.operand_2.get_double();
                switch (q.operation) {
                    case OperationType::Add:
                        d = o1 + o2;
                        break;
                    case OperationType::Sub:
                        d = o1 - o2;
                        break;
                    case OperationType::Mult:
                        d = o1 * o2;
                        break;
                    case OperationType::Div:
                        d = o1 / o2;
                        break;
                }

                if (q.operand_1.is_int() && q.operand_2.is_int())
                    q.operand_1 = Operand(std::to_string((int) d));
                else
                    q.operand_1 = Operand(std::to_string(d));
                q.operation = OperationType::Copy;
            }
        }
    }
}


void print_cfg(const std::vector<std::unique_ptr<Node>> &nodes, std::string filename) {
    DotWriter dot_writer;
    std::set<std::string> visited;
    // print edges
    for (auto &n : nodes) {
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


void print_quads(const std::vector<Quadruple> &quads, std::map<int, std::string> &labels_rev) {
    for (int i = 0; i < quads.size(); i++) {
        if (auto lbl = labels_rev.find(i); lbl != labels_rev.end())
            std::cout << lbl->second << ": \n";
        std::cout << "  " << quads[i].fmt() << std::endl;
    }
}

auto get_leading_quads_indices(const std::vector<Quadruple> &quads,
                               std::map<int, std::string> &labels_rev) -> std::map<int, std::optional<std::string>> {
    std::map<int, std::optional<std::string>> leader_indexes = {{0, std::nullopt}};
    for (int i = 0; i < quads.size(); i++) {
        if (auto lbl = labels_rev.find(i); lbl != labels_rev.end()) leader_indexes.insert({i, std::nullopt});
        switch (quads[i].operation) {
            case OperationType::IfFalse:
            case OperationType::IfTrue:
            case OperationType::Goto:
                leader_indexes.insert({i + 1, quads[i].dest.value().dest_name});
        }
    }
    return leader_indexes;
}

auto get_basicblocks_from_indices(const std::vector<Quadruple> &quads,
                                  std::map<int, std::string> &labels_rev,
                                  std::map<int, std::optional<std::string>> &leader_indexes) -> std::vector<std::unique_ptr<Node>> {
    std::vector<std::unique_ptr<Node>> nodes;
    Node *curr_node = nullptr;
    for (int i = 0, node_number = 1; i < quads.size(); i++) {
        // if current quad is a leader
        if (auto leader_index = leader_indexes.find(i); leader_index != leader_indexes.end()) {
            if (curr_node != nullptr) {
                curr_node->jumps_to = leader_index->second;
                nodes.emplace_back(curr_node);
            }
            curr_node = new Node();
            curr_node->node_name = "Node " + std::to_string(node_number++);

            if (auto lbl = labels_rev.find(i); lbl != labels_rev.end())
                curr_node->lbl_name = lbl->second;
        }
        curr_node->quads.push_back(quads[i]);
    }
    if (curr_node) nodes.emplace_back(curr_node);
    return nodes;
}

void add_successors(std::vector<std::unique_ptr<Node>> &nodes) {
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

void print_nodes(const std::vector<std::unique_ptr<Node>> &nodes) {
    for (auto &n : nodes) {
        std::cout << "\tNode: " << n->node_name << "; "
                  << n->lbl_name.value_or("NONE")
                  << "; Jumps to " << n->jumps_to.value_or("NONE") << "; Successors: " << n->successors.size()
                  << " \n" << n->fmt() << std::endl;
    }
}