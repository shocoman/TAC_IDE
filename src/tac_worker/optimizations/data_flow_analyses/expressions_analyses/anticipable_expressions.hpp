//
// Created by shoco on 2/8/2021.
//

#ifndef TAC_PARSER_ANTICIPABLE_EXPRESSIONS_HPP
#define TAC_PARSER_ANTICIPABLE_EXPRESSIONS_HPP

#include <map>

#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/utilities.hpp"
#include "tac_worker/optimizations/data_flow_analyses/set_utilities.hpp"
#include "tac_worker/structure/function.hpp"

std::pair<ID2EXPRS, ID2EXPRS> anticipable_expressions(Function &function);

#endif // TAC_PARSER_ANTICIPABLE_EXPRESSIONS_HPP
