//
// Created by shoco on 10/22/2020.
//

#ifndef TAC_PARSER_BASIC_BLOCK_HPP
#define TAC_PARSER_BASIC_BLOCK_HPP

#include <fmt/ranges.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "quadruple/quadruple.hpp"

struct BasicBlock {
    enum class Type { Entry, Normal, Exit };

    int id;
    Type type = Type::Normal;
    std::optional<std::string> label_name;
    std::vector<Quad> quads;
    std::unordered_set<BasicBlock *> successors, predecessors;
    int phi_functions = 0;

    std::string get_name() const;
    std::string fmt() const;

    int append_quad(Quad q);
    void add_successor(BasicBlock *s);
    void remove_successor(BasicBlock *s);
    void remove_successors();
    void remove_predecessors();

    BasicBlock *get_fallthrough_successor();
    BasicBlock *get_jumped_to_successor();
    bool allows_fallthrough();

    bool has_phi_function(std::string name);
    Quad &get_phi_function(std::string name);
    void add_phi_function(std::string phi_name, const std::vector<std::string> &ops);
    void update_phi_positions();
    void print_phi_nodes();
};

using BasicBlocks = std::vector<std::unique_ptr<BasicBlock>>;
using ID2Block = std::unordered_map<int, BasicBlock *>;
using ID2IDOM = std::unordered_map<int, int>;                     // id to immediate dominator
using ID2DOMS = std::unordered_map<int, std::unordered_set<int>>; // id to dominators
using ID2DF = std::unordered_map<int, std::unordered_set<int>>;   // id to dominance frontier

#endif // TAC_PARSER_BASIC_BLOCK_HPP
