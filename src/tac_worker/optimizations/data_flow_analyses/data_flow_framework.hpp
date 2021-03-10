//
// Created by shoco on 2/10/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_DATA_FLOW_FRAMEWORK_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_DATA_FLOW_FRAMEWORK_HPP

#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/utilities.hpp"
#include "tac_worker/optimizations/data_flow_analyses/set_utilities.hpp"
#include "tac_worker/structure/function.hpp"
#include <map>
#include <set>

enum class Flow { Forwards, Backwards };
enum class Meet { Union, Intersection };

template <typename T, typename F>
std::pair<std::map<int, std::set<T>>, std::map<int, std::set<T>>>
data_flow_framework(Function &f, Flow flow, Meet meet_operator, std::set<T> init_value,
                    F transfer_function) {

    auto &blocks = f.basic_blocks;
    auto &id_to_block = f.id_to_block;

    auto [visit_order, boundary_node] =
        flow == Flow::Forwards ? std::pair(f.get_reverse_post_ordering(), f.get_entry_block())
                               : std::pair(f.get_post_ordering(), f.get_exit_block());

    auto sorted_visit_order = std::vector<std::pair<int, int>>(visit_order.begin(), visit_order.end());
    std::sort(sorted_visit_order.begin(), sorted_visit_order.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    std::map<int, std::set<T>> A, B;

    // Initialization
    A[boundary_node->id] = {};
    for (auto &b : blocks)
        if (b->id != boundary_node->id)
            A[b->id] = init_value;

    // Main loop
    bool changed = true;
    while (changed) {
        changed = false;

        for (auto &[id, _] : sorted_visit_order) {
            if (id == boundary_node->id) // skip boundary block
                continue;
            auto block = id_to_block.at(id);

            std::vector<std::set<T>> preds;
            auto join_edges = flow == Flow::Forwards ? block->predecessors : block->successors;
            for (auto &pred : join_edges)
                preds.push_back(A.at(pred->id));
            auto b =
                meet_operator == Meet::Intersection ? intersection_of_sets(preds) : union_of_sets(preds);
            B[id] = b;

            auto a = flow == Flow::Forwards ? transfer_function(B, A, id) : transfer_function(A, B, id);
            if (a != A[id])
                changed = true;
            A[id] = a;
        }
    }

    return flow == Flow::Forwards ? std::pair{B, A} : std::pair{A, B};
}

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_DATA_FLOW_FRAMEWORK_HPP
