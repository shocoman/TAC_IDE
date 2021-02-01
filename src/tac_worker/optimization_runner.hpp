#ifndef TAC_PARSER_OPTIMIZATION_RUNNER_HPP
#define TAC_PARSER_OPTIMIZATION_RUNNER_HPP

#include "optimizations/value_numbering.hpp"
#include "optimizations/ssa.hpp"
#include "optimizations/sparse_simple_constant_propagation.hpp"
#include "optimizations/useless_code_elimination.hpp"
#include "structure/function.hpp"

void optimize(Function &function);

#endif // TAC_PARSER_OPTIMIZATION_RUNNER_HPP
