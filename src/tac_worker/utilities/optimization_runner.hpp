#ifndef TAC_PARSER_OPTIMIZATION_RUNNER_HPP
#define TAC_PARSER_OPTIMIZATION_RUNNER_HPP

#include "tac_worker/optimizations/sparse_simple_constant_propagation.hpp"
#include "tac_worker/optimizations/ssa.hpp"
#include "tac_worker/optimizations/useless_code_elimination.hpp"
#include "tac_worker/optimizations/value_numbering.hpp"
#include "tac_worker/structure/function.hpp"

void optimize(Function &function);

#endif // TAC_PARSER_OPTIMIZATION_RUNNER_HPP
