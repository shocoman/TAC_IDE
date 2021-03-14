//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_USELESS_CODE_ELIMINATION_HPP
#define TAC_PARSER_USELESS_CODE_ELIMINATION_HPP

#include <set>
#include <map>

#include "../structure/function.hpp"
#include "../data_flow_analyses/dominators.hpp"
#include "../data_flow_analyses/use_def_graph.hpp"

void remove_noncritical_operations(Function &f);
void remove_unreachable_blocks(Function &f);
void merge_basic_blocks(Function &f);

void useless_code_elimination(Function &function);

#endif // TAC_PARSER_USELESS_CODE_ELIMINATION_HPP