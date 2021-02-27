//
// Created by shoco on 2/8/2021.
//

#include "anticipable_expressions.hpp"

std::pair<ID2EXPRS, ID2EXPRS> anticipable_expressions(Function &function) {
    // backward data-flow problem
    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_po = function.get_post_ordering();
    auto id_po_pairs = std::vector<std::pair<int, int>>(id_to_po.begin(), id_to_po.end());
    // sort by po
    std::sort(id_po_pairs.begin(), id_po_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    // universal set
    std::set<Expression> all_expressions = get_all_expressions_set(function);

    // region Print all expressions
    std::cout << "Expressions: " << std::endl;
    for (auto &expr : all_expressions)
        std::cout << print_expression(expr) << std::endl;
    // endregion

    auto [id_to_ue_exprs, id_to_killed] = get_upward_exposed_and_killed_expressions(function);

    // region Print UpwardExposed and Killed Expressions to Console
    for (auto &[id, e_gen] : id_to_ue_exprs) {
        std::cout << id_to_block.at(id)->get_name();
        std::cout << "UE: " + print_into_string_with(e_gen, print_expression) << std::endl;
        std::cout << "KILLED: " + print_into_string_with(id_to_killed.at(id), print_expression)
                  << std::endl;
    }
    // endregion
    // region Print UpwardExposed and Killed Expressions CFG
//    {
//        std::unordered_map<int, std::string> above, below;
//        for (auto &[id, e_gen] : id_to_ue_exprs) {
//            above.emplace(id, "UE: " + print_into_string_with(e_gen, print_expression));
//            above[id] += "<BR/>KILLED: " + print_into_string_with(id_to_killed.at(id), print_expression);
//        }
//        std::string title = "Upward Exposed and Killed Expressions<BR/>";
//        title += "All Expressions: " + print_into_string_with(all_expressions, print_expression);
//        function.print_cfg("lala2.png", above, below, title);
//    }
    // endregion

    std::map<int, std::set<Expression>> in_sets, out_sets;

    // OUT[ENTRY] = 0 (empty set);
    // for (each basic block B other than EXIT) OUT[B] = U (universal set);
    auto exit_node = function.find_exit_block();
    in_sets[exit_node->id] = {};
    for (auto &b : blocks)
        if (b->id != exit_node->id)
            in_sets[b->id] = all_expressions;

    // while (changes to any OUT occur)
    bool changed = true;
    while (changed) {
        changed = false;

        // for (each basic block other than EXIT)
        // iterate in post order
        for (auto &[id, rpo] : id_po_pairs) {
            if (id == exit_node->id) // skip exit block
                continue;
            auto block = id_to_block.at(id);

            // OUT[B] = && of successors' OUTs;
            std::vector<std::set<Expression>> pred_outs;
            for (auto &pred : block->successors)
                pred_outs.push_back(in_sets.at(pred->id));
            auto OUT = intersection_of_sets(std::move(pred_outs));
            out_sets[id] = OUT;

            // IN[B] = e_gen U (OUT[B] - e_kill);
            for (auto &killed : id_to_killed.at(id))
                OUT.erase(killed);

            auto IN = union_of_sets(std::vector{id_to_ue_exprs.at(id), OUT});
            if (IN != in_sets.at(id))
                changed = true;
            in_sets[id] = IN;
        }
    }

    // region Print Anticipable Expressions to Console
    std::cout << "Anticipable Expressions" << std::endl;
    for (auto &[id, out] : out_sets) {
        std::cout << id_to_block.at(id)->get_name() << std::endl;
        std::cout << "\tIN: " + print_into_string_with(in_sets.at(id), print_expression) << std::endl;
        std::cout << "\tOUT: " + print_into_string_with(out, print_expression) << std::endl;
    }
    // endregion
    // region Print Anticipable Expressions CFG
//    std::unordered_map<int, std::string> above, below;
//    for (auto &[id, out] : out_sets) {
//        above.emplace(id, "IN: " + print_into_string_with(in_sets.at(id), print_expression));
//        below.emplace(id, "OUT: " + print_into_string_with(out, print_expression));
//    }
//    std::string title = "Anticipable Expressions<BR/>";
//    title += "All Expressions: " + print_into_string_with(all_expressions, print_expression);
//    function.print_cfg("ant1.png", above, below, title);
    // endregion
    return {in_sets, out_sets};
}