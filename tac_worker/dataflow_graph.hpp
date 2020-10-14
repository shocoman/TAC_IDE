#ifndef TAC_PARSER_DATAFLOW_GRAPH_HPP
#define TAC_PARSER_DATAFLOW_GRAPH_HPP

#include <set>
#include <list>
#include <map>

#include <algorithm>
#include <memory>

#include "quadruple.hpp"
#include "../DotWriter/DotWriter.h"
#include "../tac_worker/LoopFinder.cpp"

struct Node {
    int id;
    std::string node_name;
    std::optional<std::string> lbl_name;
    std::vector<Quadruple> quads;
    std::optional<std::string> jumps_to;
    std::set<Node *> successors;
    std::set<Node *> predecessors;

    std::string get_name() {
        return "Node " + std::to_string(id);
    }

    std::string fmt() const
    {
        std::string out;
        for (auto &q : quads)
            out += q.fmt() + "\n";
        return out;
    }

    void add_successor(Node *s)
    {
        successors.emplace(s);
        s->predecessors.emplace(this);
    }

    void remove_successors()
    {
        for (auto &s: successors) {
            s->predecessors.erase(this);
        }
        successors.erase(successors.begin(), successors.end());
    }

    bool allows_fallthrough()
    {
        return quads.back().operation != OperationType::Goto
               && quads.back().operation != OperationType::Return;
    }
};


void print_nodes(const std::vector<std::unique_ptr<Node>> &nodes);

void add_successors(std::vector<std::unique_ptr<Node>> &nodes);


auto get_basicblocks_from_indices(const std::vector<Quadruple> &quads, std::map<int, std::string> &labels_rev,
                                  std::map<int, std::optional<std::string>> &leader_indexes)
-> std::vector<std::unique_ptr<Node>>;

auto get_leading_quads_indices(const std::vector<Quadruple> &quads,
                               std::map<int, std::string> &) -> std::map<int, std::optional<std::string>>;

void print_quads(const std::vector<Quadruple> &quads, std::map<int, std::string> &labels_rev);


void print_cfg(const std::vector<std::unique_ptr<Node>> &nodes, std::string filename);

void make_cfg(std::map<std::string, int> &&labels, std::vector<Quadruple> &&quads);

#endif //TAC_PARSER_DATAFLOW_GRAPH_HPP
