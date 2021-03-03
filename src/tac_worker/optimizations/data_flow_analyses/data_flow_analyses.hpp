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

using ID2Defs = std::map<int, std::set<std::string>>;

void liveness_analyses_on_block(const BasicBlocks &nodes);
std::pair<ID2Defs, ID2Defs> live_variable_analyses(Function &function);
void reaching_definitions(Function &function);

std::pair<ID2EXPRS, ID2EXPRS> available_expressions(Function &f);
std::pair<ID2EXPRS, ID2EXPRS> anticipable_expressions(Function &f);

template <template <typename...> typename Map, template <typename> typename Set, typename SetType,
          typename F>
void print_analyses_result_on_graph(Function &f, Map<int, Set<SetType>> IN, Map<int, Set<SetType>> OUT,
                                    std::string title, F func, std::string IN_name = "IN",
                                    std::string OUT_name = "OUT") {
    static_assert(std::is_same<Map<int, Set<SetType>>, std::map<int, Set<SetType>>>::value ||
                      std::is_same<Map<int, Set<SetType>>, std::unordered_map<int, Set<SetType>>>::value,
                  "Function can only get std::map or std::unordered_map.");
    static_assert(std::is_same<Set<SetType>, std::set<SetType>>::value ||
                      std::is_same<Set<SetType>, std::unordered_set<SetType>>::value,
                  "Function can only get std::set or std::unordered_set as value for map.");

    std::unordered_map<int, std::string> above, below;
    for (auto &[id, in] : IN)
        above.emplace(id, fmt::format("{}: {}", IN_name, print_into_string_with(in, func)));
    for (auto &[id, out] : OUT)
        below.emplace(id, fmt::format("{}: {}", OUT_name, print_into_string_with(out, func)));
    // replace spaces in title
    auto title_wo_spaces = title;
    for (auto &ch : title_wo_spaces)
        if (std::isspace(ch))
            ch = '_';
    f.print_cfg(title_wo_spaces + ".png", above, below, title);
}

#endif // TAC_PARSER_DATA_FLOW_ANALYSES_HPP
