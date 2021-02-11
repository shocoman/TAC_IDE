//
// Created by shoco on 2/11/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP

#include <map>
#include <set>

#include "data_flow_analyses/data_flow_analyses.hpp"
#include "data_flow_analyses/expressions_analyses/utilities.hpp"
#include "tac_worker/structure/function.hpp"


std::pair<ID2EXPRS, ID2EXPRS> AvailableExpressionsLazyCodeMotion(Function& f);
ID2EXPRS EarliestExpressions(ID2EXPRS &AntIn, ID2EXPRS &AvailIn);
std::pair<ID2EXPRS, ID2EXPRS> PostponableExpressions(Function &f);
ID2EXPRS LatestExpressions(Function &f);
std::pair<ID2EXPRS, ID2EXPRS> UsedExpressions(Function &f);

void lazy_code_motion(Function &f);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP
