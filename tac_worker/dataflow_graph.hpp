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
#include "../tac_worker/local_value_numbering.h"




void print_nodes(const std::vector<std::unique_ptr<BasicBlock>> &nodes);

void add_successors(std::vector<std::unique_ptr<BasicBlock>> &nodes);


auto get_basicblocks_from_indices(const std::vector<Quad> &quads, std::map<int, std::string> &labels_rev,
                                  std::map<int, std::optional<std::string>> &leader_indexes)
-> std::vector<std::unique_ptr<BasicBlock>>;

auto get_leading_quads_indices(const std::vector<Quad> &quads,
                               std::map<int, std::string> &) -> std::map<int, std::optional<std::string>>;

void print_quads(const std::vector<Quad> &quads, std::map<int, std::string> &labels_rev);


void print_cfg(const std::vector<std::unique_ptr<BasicBlock>> &nodes, const std::string &filename);

void make_cfg(std::map<std::string, int> &&labels, std::vector<Quad> &&quads);

#endif //TAC_PARSER_DATAFLOW_GRAPH_HPP
