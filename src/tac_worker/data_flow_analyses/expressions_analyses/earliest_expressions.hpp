//
// Created by shoco on 2/10/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_EXPRESSIONS_ANALYSES_EARLIEST_EXPRESSIONS_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_EXPRESSIONS_ANALYSES_EARLIEST_EXPRESSIONS_HPP

#include "tac_worker/data_flow_analyses/data_flow_analyses.hpp"
#include "tac_worker/structure/function.hpp"
#include "utilities.hpp"

std::map<std::pair<int, int>, std::set<Expression>> earliest_expressions(Function &f);

std::pair<std::map<int, std::set<Expression>>, std::map<std::pair<int, int>, std::set<Expression>>>
later_placement_expressions(Function &function);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_EXPRESSIONS_ANALYSES_EARLIEST_EXPRESSIONS_HPP
