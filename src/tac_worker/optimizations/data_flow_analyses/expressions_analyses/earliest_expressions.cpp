//
// Created by shoco on 2/10/2021.
//

#include "earliest_expressions.hpp"

std::map<std::pair<int, int>, std::set<Expression>> earliest_expressions(Function &f) {
    // Earliest(i, j) = AntIn(j) ∩ !AvailOut(i) ∩ (ExprKill(i) ∪ !AntOut(i))
    auto all_expressions = get_all_expressions_set(f);
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
        auto i_block = f.id_to_block.at(i), j_block = f.id_to_block.at(j);

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
        std::cout << "Edge (" << i_block->get_name() << ", " << j_block->get_name() << ": "
                  << print_into_string_with(earliest_ij, print_expression) << std::endl;
    }
    return earliest_exprs;
}

void later_placement_expressions(Function &function) {
    // forward data-flow problem
    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_rpo = function.get_reverse_post_ordering();
    auto id_rpo_pairs = std::vector<std::pair<int, int>>(id_to_rpo.begin(), id_to_rpo.end());
    // sort by po
    std::sort(id_rpo_pairs.begin(), id_rpo_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    // universal set
    std::set<Expression> all_expressions = get_all_expressions_set(function);

    //    // region Print all expressions
    //    std::cout << "Expressions: " << std::endl;
    //    for (auto &expr : all_expressions)
    //        std::cout << print_expression(expr) << std::endl;
    //    // endregion

    auto [id_to_ue_exprs, id_to_killed] = get_upward_exposed_and_killed_expressions(function);
    auto earliest = earliest_expressions(function);

    //    // region Print UpwardExposed and Killed Expressions to Console
    //    for (auto &[id, e_gen] : id_to_ue_exprs) {
    //        std::cout << id_to_block.at(id)->get_name();
    //        std::cout << "UE: " + print_into_string_with(e_gen, print_expression) << std::endl;
    //        std::cout << "KILLED: " + print_into_string_with(id_to_killed.at(id), print_expression)
    //                  << std::endl;
    //    }
    //    // endregion
    //    // region Print UpwardExposed and Killed Expressions CFG
    //    {
    //        std::unordered_map<int, std::string> above, below;
    //        for (auto &[id, e_gen] : id_to_ue_exprs) {
    //            above.emplace(id, "UE: " + print_into_string_with(e_gen, print_expression));
    //            above[id] += "<BR/>KILLED: " + print_into_string_with(id_to_killed.at(id),
    //            print_expression);
    //        }
    //        std::string title = "Upward Exposed and Killed Expressions<BR/>";
    //        title += "All Expressions: " + print_into_string_with(all_expressions, print_expression);
    //        function.print_cfg("lala2.png", above, below, title);
    //    }
    //    // endregion

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

    //    // region Print Anticipable Expressions to Console
    //    std::cout << "Anticipable Expressions" << std::endl;
    //    for (auto &[id, out] : Later) {
    //        std::cout << id_to_block.at(id)->get_name() << std::endl;
    //        std::cout << "\tIN: " + print_into_string_with(LaterIn.at(id), print_expression) <<
    //        std::endl; std::cout << "\tOUT: " + print_into_string_with(out, print_expression) <<
    //        std::endl;
    //    }
    //    // endregion
    //    // region Print Anticipable Expressions CFG
    //    std::unordered_map<int, std::string> above, below;
    //    for (auto &[id, out] : Later) {
    //        above.emplace(id, "IN: " + print_into_string_with(LaterIn.at(id), print_expression));
    //        below.emplace(id, "OUT: " + print_into_string_with(out, print_expression));
    //    }
    //    std::string title = "Anticipable Expressions<BR/>";
    //    title += "All Expressions: " + print_into_string_with(all_expressions, print_expression);
    //    function.print_cfg("lala.png", above, below, title);
    //    // endregion
    //    return {LaterIn, Later};
}
