//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_SSA_HPP
#define TAC_PARSER_SSA_HPP

#include <numeric>
#include <set>

#include "../../graph_writer/graph_writer.hpp"
#include "../quad_preparation.hpp"
#include "../structure/function.hpp"
#include "data_flow_analyses/dominators.hpp"
#include "value_numbering.hpp"

void place_phi_functions(Function &function,
                         std::map<std::string, std::set<BasicBlock *>> &var_to_blocks,
                         std::set<std::string> &global_names);
void rename_variables(Function &function, std::set<std::string> &global_names);

void convert_to_ssa(Function &function);

void convert_from_ssa(Function &function);

#endif // TAC_PARSER_SSA_HPP
