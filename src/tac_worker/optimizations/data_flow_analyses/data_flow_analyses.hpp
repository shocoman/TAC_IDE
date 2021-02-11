//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_DATA_FLOW_ANALYSES_HPP
#define TAC_PARSER_DATA_FLOW_ANALYSES_HPP

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <tuple>
#include <vector>


#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/utilities.hpp"
#include "tac_worker/optimizations/data_flow_analyses/set_utilities.hpp"
#include "tac_worker/optimizations/data_flow_analyses/data_flow_framework.hpp"
#include "tac_worker/structure/function.hpp"

void liveness_analyses_on_block(const BasicBlocks &nodes);
void liveness_analyses_engineering_compiler(Function &function);
void liveness_analyses_dragon_book(Function &function);
void reaching_definitions(Function &function);

std::pair<ID2EXPRS, ID2EXPRS> AvailableExpressions(Function& f);
std::pair<ID2EXPRS, ID2EXPRS> AnticipableExpressions(Function &f);

#endif // TAC_PARSER_DATA_FLOW_ANALYSES_HPP
