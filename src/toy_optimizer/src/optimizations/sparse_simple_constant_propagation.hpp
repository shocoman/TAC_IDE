//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_SPARSE_SIMPLE_CONSTANT_PROPAGATION_HPP
#define TAC_PARSER_SPARSE_SIMPLE_CONSTANT_PROPAGATION_HPP

#include "../data_flow_analyses/data_flow_analyses.hpp"
#include "value_numbering.hpp"

void sparse_simple_constant_propagation(Function &location);

#endif // TAC_PARSER_SPARSE_SIMPLE_CONSTANT_PROPAGATION_HPP
