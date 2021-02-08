//
// Created by shoco on 2/8/2021.
//

#ifndef TAC_PARSER_AVAILABLE_EXPRESSIONS_HPP
#define TAC_PARSER_AVAILABLE_EXPRESSIONS_HPP

#include <map>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "tac_worker/optimizations/data_flow_analyses/utilities.hpp"
#include "tac_worker/structure/function.hpp"
#include "tac_worker/print_utility.hpp"

void available_expressions(Function &function);

#endif // TAC_PARSER_AVAILABLE_EXPRESSIONS_HPP
