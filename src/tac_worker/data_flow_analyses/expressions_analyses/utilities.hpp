//
// Created by shoco on 2/9/2021.
//

#ifndef TAC_PARSER_EXPRESSION_UTILITIES_HPP
#define TAC_PARSER_EXPRESSION_UTILITIES_HPP

#include <map>
#include <set>
#include <tuple>

#include "tac_worker/structure/function.hpp"

using Expression = std::tuple<Operand, Quad::Type, Operand>;
using ID2EXPRS = std::map<int, std::set<Expression>>;

std::set<Expression> get_all_expressions(Function &f);

std::string print_expression(Expression expr);
std::string split_long_string(std::string str, int max_length);

std::pair<ID2EXPRS, ID2EXPRS> get_upward_exposed_and_killed_expressions(Function &f);
std::pair<ID2EXPRS, ID2EXPRS> get_downward_exposed_and_killed_expressions(Function &f);

#endif // TAC_PARSER_EXPRESSION_UTILITIES_HPP
