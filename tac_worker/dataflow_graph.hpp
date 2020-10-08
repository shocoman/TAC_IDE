#ifndef TAC_PARSER_DATAFLOW_GRAPH_HPP
#define TAC_PARSER_DATAFLOW_GRAPH_HPP

#include "quadruple.hpp"
#include "../DotWriter/DotWriter.h"
#include <set>
#include <algorithm>

struct Node {
    std::string node_name;
    std::optional<std::string> lbl_name;
    std::vector<Quadruple> quads;
    std::optional<std::string> jumps_to;
    std::set<Node*> successors;

    std::string fmt() {
        std::string out;
        for (auto &q : quads)
            out += q.fmt() + "\n";
        return out;
    }
};


void print_nodes(const std::vector<std::unique_ptr<Node>> &nodes);
void add_successors(std::vector<std::unique_ptr<Node>> &nodes);
std::vector<std::unique_ptr<Node>> get_nodes_from_indices(const driver &drv, std::map<int, std::string>&,std::map<int, std::optional<std::string>>&);
std::map<int, std::optional<std::string>> get_leading_quads_indices(const driver&, std::map<int, std::string>&);
void print_quads(const driver &drv, std::map<int, std::string> &labels_rev);


void func(driver &drv) {
    // revert map of labels
    std::map<int, std::string> labels_rev;
    for (auto &[a, b] : drv.labels) {
        labels_rev.emplace(b, a);
    }

    print_quads(drv, labels_rev);

    // gather indexes of leading blocks
    auto leader_indexes = get_leading_quads_indices(drv, labels_rev);
    auto nodes = get_nodes_from_indices(drv, labels_rev, leader_indexes);
    add_successors(nodes);
    print_nodes(nodes);




//    std::cout << std::endl;
//    for (auto &[i, name] : leader_indexes) {
//        std::cout << i  << ": " << name.value_or("NOPE") << std::endl;
//    }

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
                std::cout << n->node_name << " -> " << s->node_name << std::endl;
                dot_writer.add_edge(n->node_name, s->node_name, s->lbl_name.value_or(""));
            }
        }
    }

    dot_writer.save_to_file("mydotfile.dot");
}




void print_quads(const driver &drv, std::map<int, std::string> &labels_rev) {
    for (int i = 0; i < drv.quadruples.size(); i++) {
        if (auto lbl = labels_rev.find(i); lbl != labels_rev.end())
            std::cout << lbl->second << ": \n";
        std::cout << "  " << drv.quadruples[i].fmt() << std::endl;
    }
}

std::map<int, std::optional<std::string>>
get_leading_quads_indices(const driver &drv, std::map<int, std::string> &labels_rev) {
    std::map<int, std::optional<std::string>> leader_indexes = {{0, std::nullopt}};
    for (int i = 0; i < drv.quadruples.size(); i++) {
        if (auto lbl = labels_rev.find(i); lbl != labels_rev.end()) leader_indexes.insert({i, std::nullopt});
         switch (drv.quadruples[i].operation) {
                case OperationType::IfFalse:
                case OperationType::IfTrue:
                case OperationType::Goto:
                    leader_indexes.insert({i + 1, drv.quadruples[i].dest.value().dest_name});
            }
    }
    return leader_indexes;
}

std::vector<std::unique_ptr<Node>>
get_nodes_from_indices(const driver &drv,
                       std::map<int, std::string> &labels_rev,
                       std::map<int, std::optional<std::string>> &leader_indexes) {
    std::vector<std::unique_ptr<Node>> nodes;
    Node *curr_node = nullptr;
    for (int i = 0, node_number = 1; i < drv.quadruples.size(); i++) {
        // if current quad is a leader
        if (auto leader_index = leader_indexes.find(i); leader_index != leader_indexes.end()) {
            if (curr_node != nullptr) {
                curr_node->jumps_to = leader_index->second;
                nodes.emplace_back(curr_node);
            }
            curr_node = new Node();;
            curr_node->node_name = "Node " + std::to_string(node_number++);

            if (auto lbl = labels_rev.find(i); lbl != labels_rev.end())
                curr_node->lbl_name = lbl->second;
        }
        curr_node->quads.push_back(drv.quadruples[i]);
    }
    if (curr_node) nodes.emplace_back(curr_node);
    return nodes;
}

void add_successors(std::vector<std::unique_ptr<Node>> &nodes) {
    for (int i = 0; i < nodes.size(); ++i) {
        if (i != nodes.size() - 1)
            nodes[i]->successors.insert(nodes[i + 1].get());
        if (nodes[i]->jumps_to.has_value()) {
            auto jump_to = nodes[i]->jumps_to.value();
            auto node = std::find_if(nodes.begin(), nodes.end(),[&jump_to](auto &e) {
                                            return e->lbl_name.has_value() && e->lbl_name.value() == jump_to; });

            nodes[i]->successors.insert(node->get());
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

#endif //TAC_PARSER_DATAFLOW_GRAPH_HPP
