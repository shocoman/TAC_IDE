//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_SPARSE_SIMPLE_CONSTANT_PROPAGATION_HPP
#define TAC_PARSER_SPARSE_SIMPLE_CONSTANT_PROPAGATION_HPP

#include "../optimization_runner.hpp"
#include "live_variable_analyses.hpp"

void sparse_simple_constant_propagation(BasicBlocks &blocks);

#endif // TAC_PARSER_SPARSE_SIMPLE_CONSTANT_PROPAGATION_HPP
