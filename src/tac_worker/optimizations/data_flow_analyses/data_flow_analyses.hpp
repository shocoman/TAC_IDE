//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_DATA_FLOW_ANALYSES_HPP
#define TAC_PARSER_DATA_FLOW_ANALYSES_HPP

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "tac_worker/optimizations/data_flow_analyses/data_flow_framework.hpp"
#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/utilities.hpp"
#include "tac_worker/optimizations/data_flow_analyses/set_utilities.hpp"
#include "tac_worker/structure/function.hpp"

void liveness_analyses_on_block(const BasicBlocks &nodes);
void liveness_analyses_engineering_compiler(Function &function);
void liveness_analyses_dragon_book(Function &function);
void reaching_definitions(Function &function);

std::pair<ID2EXPRS, ID2EXPRS> AvailableExpressions(Function &f);
std::pair<ID2EXPRS, ID2EXPRS> AnticipableExpressions(Function &f);

template <typename T>
void print_analyses_result_on_graph(
    Function &f, std::pair<std::map<int, std::set<T>>, std::map<int, std::set<T>>> input,
    std::string title) {
    std::unordered_map<int, std::string> above, below;
    auto &[IN, OUT] = input;
    for (auto &[id, in] : IN)
        above.emplace(id, "IN: " + print_into_string_with(in, print_expression));
    for (auto &[id, out] : OUT)
        below.emplace(id, "OUT: " + print_into_string_with(out, print_expression));
    f.print_cfg(title + ".png", above, below, title);
}

#endif // TAC_PARSER_DATA_FLOW_ANALYSES_HPP
