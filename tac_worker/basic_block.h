//
// Created by shoco on 10/22/2020.
//

#ifndef TAC_PARSER_BASIC_BLOCK_H
#define TAC_PARSER_BASIC_BLOCK_H

#include <string>
#include <optional>
#include <vector>
#include <set>
#include <map>

#include "quadruple.hpp"

struct BasicBlock {
    int id;
    std::string node_name;
    std::optional<std::string> lbl_name;
    std::vector<Quad> quads;
    std::optional<std::string> jumps_to;
    std::set<BasicBlock *> successors;
    std::set<BasicBlock *> predecessors;

    std::string get_name() {
        return "BB " + std::to_string(id);
    }

    std::string fmt() const {
        std::string out;
        for (auto &q : quads)
            out += q.fmt() + "\n";
        return out;
    }

    void add_successor(BasicBlock *s) {
        successors.emplace(s);
        s->predecessors.emplace(this);
    }

    void remove_successors() {
        for (auto &s: successors) {
            s->predecessors.erase(this);
        }
        successors.erase(successors.begin(), successors.end());
    }

    bool allows_fallthrough() {
        return quads.back().type != Quad::Type::Goto
               && quads.back().type != Quad::Type::Return;
    }
};


#endif //TAC_PARSER_BASIC_BLOCK_H
