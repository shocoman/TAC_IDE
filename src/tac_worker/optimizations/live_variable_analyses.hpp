//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_LIVE_VARIABLE_ANALYSES_HPP
#define TAC_PARSER_LIVE_VARIABLE_ANALYSES_HPP

#include "../../graph_writer/graph_writer.hpp"
#include "../optimization_runner.hpp"
#include "ssa.hpp"
#include "useless_code_elimination.hpp"
#include <numeric>

void liveness_analyses(const BasicBlocks &nodes);

void live_analyses(BasicBlocks &blocks);

#endif // TAC_PARSER_LIVE_VARIABLE_ANALYSES_HPP
