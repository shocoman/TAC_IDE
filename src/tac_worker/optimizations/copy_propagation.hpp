//
// Created by shoco on 2/27/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP

#include <map>

#include "tac_worker/structure/function.hpp"

void copy_propagation_on_ssa(Function &f);

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP
