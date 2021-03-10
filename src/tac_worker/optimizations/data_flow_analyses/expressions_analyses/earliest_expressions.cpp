//
// Created by shoco on 2/10/2021.
//

#include "earliest_expressions.hpp"

std::map<std::pair<int, int>, std::set<Expression>> earliest_expressions(Function &f) {
    // Earliest(i, j) = AntIn(j) ∩ !AvailOut(i) ∩ (ExprKill(i) ∪ !AntOut(i))
    auto all_expressions = get_all_expressions(f);
    auto [ant_in, ant_out] = anticipable_expressions(f);
    auto [avail_in, avail_out] = available_expressions(f);
    auto [ue_exprs, killed_exprs] = get_upward_exposed_and_killed_expressions(f);

    // collect all edges
    std::set<std::pair<int, int>> all_edges;
    for (auto &b : f.basic_blocks)
        for (auto &succ : b->successors)
            all_edges.emplace(b->id, succ->id);

    std::map<std::pair<int, int>, std::set<Expression>> earliest_exprs;
    for (auto &[i, j] : all_edges) {
        auto AntIn_j = ant_in.at(j);

        auto NOT_AvailOut_i = all_expressions;
        for (auto &expr : avail_out.at(i))
            NOT_AvailOut_i.erase(expr);

        auto NOT_AntOut_i = all_expressions;
        for (auto &expr : ant_out.at(i))
            NOT_AntOut_i.erase(expr);
        for (auto &expr : killed_exprs.at(i))
            NOT_AntOut_i.insert(expr);

        auto earliest_ij = intersection_of_sets(std::vector{AntIn_j, NOT_AvailOut_i, NOT_AntOut_i});
        earliest_exprs.emplace(std::pair{i, j}, earliest_ij);
    }
    return earliest_exprs;
}

std::pair<std::map<int, std::set<Expression>>, std::map<std::pair<int, int>, std::set<Expression>>>
later_placement_expressions(Function &function) {
    // forward data-flow problem
    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_rpo = function.get_reverse_post_ordering();
    auto id_rpo_pairs = std::vector<std::pair<int, int>>(id_to_rpo.begin(), id_to_rpo.end());
    // sort by po
    std::sort(id_rpo_pairs.begin(), id_rpo_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    std::set<Expression> all_expressions = get_all_expressions(function);

    auto [id_to_ue_exprs, id_to_killed] = get_upward_exposed_and_killed_expressions(function);
    auto earliest = earliest_expressions(function);

    std::map<int, std::set<Expression>> LaterIn;
    std::map<std::pair<int, int>, std::set<Expression>> Later;

    auto entry_node = function.find_entry_block();
    LaterIn[entry_node->id] = {};
    for (auto &b : blocks)
        if (b->id != entry_node->id)
            LaterIn[b->id] = all_expressions;

    bool changed = true;
    while (changed) {
        changed = false;

        for (auto &[id, rpo] : id_rpo_pairs) {
            if (id == entry_node->id) // skip entry block
                continue;
            auto block = id_to_block.at(id);

            for (auto &pred : block->predecessors) {
                int i = pred->id, j = id;

                auto later_ins = LaterIn.at(i);
                for (auto &e : id_to_ue_exprs.at(i))
                    later_ins.erase(e);
                for (auto &e : earliest.at({i, j}))
                    later_ins.insert(e);

                if (later_ins != Later[{i, j}])
                    changed = true;
                Later[{i, j}] = later_ins;
            }

            std::vector<std::set<Expression>> preds;
            for (auto &pred : block->predecessors)
                preds.push_back(Later.at({pred->id, id}));
            auto NextLaterIn = intersection_of_sets(std::move(preds));
            LaterIn[id] = NextLaterIn;
        }
    }

    std::cout << "Later:" << std::endl;
    for (auto &[ids, exprs] : Later) {
        auto [i, j] = ids;
        auto i_block = function.id_to_block.at(i), j_block = function.id_to_block.at(j);
        std::cout << "Edge (" << i_block->get_name() << ", " << j_block->get_name() << ": "
                  << print_into_string_with(exprs, print_expression) << std::endl;
    }

    std::cout << "LaterIN:" << std::endl;
    for (auto &[id, exprs] : LaterIn) {
        std::cout << "Node: " << function.id_to_block.at(id)->get_name()
                  << print_into_string_with(exprs, print_expression) << std::endl;
    }

    return {LaterIn, Later};
}
