//
// Created by shoco on 3/2/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_PRINT_GRAPH_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_PRINT_GRAPH_HPP

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "data_flow_analyses.hpp"
#include "dominators.hpp"
#include "../utilities/graph_writer/graph_writer.hpp"
#include "../optimizations/lazy_code_motion.hpp"
#include "../structure/function.hpp"

std::vector<char> print_depth_first_search_tree(Function &f);
std::vector<char> print_dominator_tree(Function &f);
std::vector<char> print_postdominator_tree(Function &f);
std::vector<char> print_control_dependence(Function &f);
std::vector<char> print_anticipable_expressions(Function &f);
std::vector<char> print_available_expressions(Function &f);
std::vector<char> print_ue_de_and_killed_expressions(Function &f);
std::vector<char> print_live_variable(Function &f);
void print_lazy_code_motion_graphs(Function &f);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_PRINT_GRAPH_HPP
