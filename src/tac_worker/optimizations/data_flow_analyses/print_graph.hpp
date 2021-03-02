//
// Created by shoco on 3/2/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_PRINT_GRAPH_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_PRINT_GRAPH_HPP

#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#include "graph_writer/graph_writer.hpp"
#include "tac_worker/structure/function.hpp"
#include "dominators.hpp"

void print_depth_first_search_tree(Function &f);
void print_dominator_tree(Function &f);
void print_postdominator_tree(Function &f);
void print_control_dependence(Function &f);


#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_PRINT_GRAPH_HPP
