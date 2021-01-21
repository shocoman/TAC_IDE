#ifndef TAC_PARSER_DATAFLOW_GRAPH_HPP
#define TAC_PARSER_DATAFLOW_GRAPH_HPP

#include <list>
#include <map>
#include <set>

#include <algorithm>
#include <memory>

#include "../graph_writer/graph_writer.h"
#include "../tac_worker/local_value_numbering.h"
#include "../tac_worker/loop_finder.cpp"
#include "basic_block.h"
#include "quadruple.hpp"

void print_blocks(const std::vector<std::unique_ptr<BasicBlock>> &blocks);

void add_initial_successors(std::vector<std::unique_ptr<BasicBlock>> &nodes);

auto get_basic_blocks_from_indices(const std::vector<Quad> &quads,
                                   std::map<int, std::string> &labels_rev,
                                   std::map<int, std::optional<std::string>> &leader_indices)
    -> std::vector<std::unique_ptr<BasicBlock>>;

auto get_leading_quads_indices(const std::vector<Quad> &quads, std::map<int, std::string> &)
    -> std::map<int, std::optional<std::string>>;

void print_quads(const std::vector<Quad> &quads, std::map<int, std::string> &labels_rev);

void print_cfg(const std::vector<std::unique_ptr<BasicBlock>> &nodes, const std::string &filename);

void make_cfg(std::map<std::string, int> &&labels, std::vector<Quad> &&quads);

#endif // TAC_PARSER_DATAFLOW_GRAPH_HPP
