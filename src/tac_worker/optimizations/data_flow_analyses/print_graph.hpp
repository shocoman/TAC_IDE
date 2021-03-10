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
#include "../lazy_code_motion.hpp"
#include "dominators.hpp"
#include "graph_writer/graph_writer.hpp"
#include "tac_worker/structure/function.hpp"

void print_depth_first_search_tree(Function &f);
void print_dominator_tree(Function &f);
void print_postdominator_tree(Function &f);
void print_control_dependence(Function &f);
void print_anticipable_expressions(Function &f);
void print_available_expressions(Function &f);
void print_ue_de_and_killed_expressions(Function &f);
void print_live_variable(Function &f);
void print_lazy_code_motion_graphs(Function &f);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_PRINT_GRAPH_HPP
