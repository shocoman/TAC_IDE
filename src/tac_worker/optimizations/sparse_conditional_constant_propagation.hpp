//
// Created by shoco on 2/12/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_SPARSE_CONDITIONAL_CONSTANT_PROPAGATION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_SPARSE_CONDITIONAL_CONSTANT_PROPAGATION_HPP

#include <map>
#include <set>
#include <fmt/ranges.h>

#include "tac_worker/structure/function.hpp"
#include "tac_worker/optimizations/value_numbering.hpp"

void sparse_conditional_constant_propagation(Function &f);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_SPARSE_CONDITIONAL_CONSTANT_PROPAGATION_HPP
