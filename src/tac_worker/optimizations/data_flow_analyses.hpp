//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_DATA_FLOW_ANALYSES_HPP
#define TAC_PARSER_DATA_FLOW_ANALYSES_HPP

#include "../../graph_writer/graph_writer.hpp"
#include "../optimization_runner.hpp"
#include "ssa.hpp"
#include "useless_code_elimination.hpp"
#include <numeric>

void liveness_analyses_on_block(const BasicBlocks &nodes);

void liveness_analyses_engineering_compiler(Function &function);
void liveness_analyses_dragon_book(Function &function);

void reaching_definitions(Function &function);

void available_expressions(Function &function);

void anticipable_expressions(Function &function);

#endif // TAC_PARSER_DATA_FLOW_ANALYSES_HPP
