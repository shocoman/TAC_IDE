//
// Created by shoco on 2/11/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP

#include <map>
#include <set>

#include "../data_flow_analyses/data_flow_analyses.hpp"
#include "../data_flow_analyses/expressions_analyses/utilities.hpp"
#include "../structure/function.hpp"
#include "../utilities/new_name_generator.hpp"


std::pair<ID2EXPRS, ID2EXPRS> available_expressions_lazy_code_motion(Function& f);
ID2EXPRS earliest_expressions(ID2EXPRS &AntIn, ID2EXPRS &AvailIn);
std::pair<ID2EXPRS, ID2EXPRS> postponable_expressions(Function &f);
ID2EXPRS latest_expressions(Function &f);
std::pair<ID2EXPRS, ID2EXPRS> used_expressions(Function &f);

void lazy_code_motion(Function &f);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP
