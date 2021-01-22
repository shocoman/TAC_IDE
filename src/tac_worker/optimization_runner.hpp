#ifndef TAC_PARSER_OPTIMIZATION_RUNNER_HPP
#define TAC_PARSER_OPTIMIZATION_RUNNER_HPP

#include "../_garbage_bin/loop_finder.cpp"
#include "optimizations/value_numbering.hpp"
#include "optimizations/ssa.hpp"
#include "structure/function.hpp"

void optimize(Function &function);

#endif // TAC_PARSER_OPTIMIZATION_RUNNER_HPP
