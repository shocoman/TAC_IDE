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

void place_phi_functions(Function &function, ID2IDOM &id_to_immediate_dominator,
                         ID2DF &id_to_dominance_frontier,
                         std::map<std::string, std::set<BasicBlock *>> &var_to_blocks,
                         std::set<std::string> &global_names, std::set<std::string> &all_names);

void convert_to_ssa(Function &function);

void remove_phi_functions(Function &function);

#endif // TAC_PARSER_SSA_HPP
