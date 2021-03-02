//
// Created by shoco on 2/5/2021.
//

#ifndef TAC_PARSER_DOMINATORS_HPP
#define TAC_PARSER_DOMINATORS_HPP

#include <map>
#include <numeric>
#include <set>

#include "graph_writer/graph_writer.hpp"
#include "set_utilities.hpp"
#include "tac_worker/structure/function.hpp"

ID2DF get_dominance_frontier(Function &function, ID2IDOM &id_to_immediate_dominator);
ID2IDOM get_immediate_dominators(Function &function);
ID2DOMS get_dominators(Function &function);

bool is_dominated_by(const ID2DOMS &id_to_doms, int a_id, int b_id);
int get_common_dominator_id(int Z_id, int B_id, const ID2IDOM &id_to_idom, const ID2DOMS &id_to_doms);

#endif // TAC_PARSER_DOMINATORS_HPP
