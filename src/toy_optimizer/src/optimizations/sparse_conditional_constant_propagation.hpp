//
// Created by shoco on 2/12/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_SPARSE_CONDITIONAL_CONSTANT_PROPAGATION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_SPARSE_CONDITIONAL_CONSTANT_PROPAGATION_HPP

#include <fmt/ranges.h>
#include <map>
#include <set>

#include "../optimizations/value_numbering.hpp"
#include "../structure/function.hpp"

void sparse_conditional_constant_propagation(Function &f);
void remove_useless_blocks(Function &f,  std::unordered_set<int> &useless_blocks);

void print_sccp_result_graph(Function &f, std::set<std::pair<int, int>> &executed_edges,
                             std::unordered_set<int> &useless_blocks,
                             std::set<std::pair<int, int>> &changed_places);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_SPARSE_CONDITIONAL_CONSTANT_PROPAGATION_HPP
