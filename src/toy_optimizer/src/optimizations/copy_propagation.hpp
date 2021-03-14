//
// Created by shoco on 2/27/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP

#include <map>
#include <set>

#include "../structure/function.hpp"
#include "../data_flow_analyses/data_flow_framework.hpp"
#include "../data_flow_analyses/data_flow_analyses.hpp"

void copy_propagation_on_ssa(Function &f);
void copy_propagation_on_non_ssa(Function &f);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP
