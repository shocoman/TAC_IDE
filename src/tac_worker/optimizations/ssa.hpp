//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_SSA_HPP
#define TAC_PARSER_SSA_HPP

#include "../../graph_writer/graph_writer.hpp"
#include "../quad_preparation.hpp"
#include "../structure/function.hpp"
#include "value_numbering.hpp"
#include <numeric>
#include <set>

void print_dominator_tree(ID2Block &id_to_block, ID2IDOM &id_to_idom);

ID2DOM find_dominators(const BasicBlocks &blocks);

ID2IDOM find_immediate_dominators(Function &function);

ID2DF find_dominance_frontier(const BasicBlocks &blocks, ID2IDOM &id_to_immediate_dominator);

void place_phi_functions(Function &function, ID2IDOM &id_to_immediate_dominator,
                         ID2DF &id_to_dominance_frontier,
                         std::map<std::string, std::set<BasicBlock *>> &var_to_blocks,
                         std::set<std::string> &global_names, std::set<std::string> &all_names);

void convert_to_ssa(Function &function);

void remove_phi_functions(Function &function);

#endif // TAC_PARSER_SSA_HPP
