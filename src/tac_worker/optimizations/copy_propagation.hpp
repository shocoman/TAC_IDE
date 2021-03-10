//
// Created by shoco on 2/27/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP

#include <map>
#include <set>

#include "tac_worker/structure/function.hpp"
#include "tac_worker/optimizations/data_flow_analyses/data_flow_framework.hpp"
#include "tac_worker/optimizations/data_flow_analyses/data_flow_analyses.hpp"

void copy_propagation_on_ssa(Function &f);
void copy_propagation_on_non_ssa(Function &f);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP
