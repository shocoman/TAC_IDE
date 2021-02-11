//
// Created by shoco on 2/8/2021.
//

#include "available_expressions.hpp"

std::pair<ID2EXPRS, ID2EXPRS> available_expressions(Function &function) {
    // forward data-flow problem
    // Algorithm 9.17 : Available expressions.
    // INPUT: A flow graph with e_kill and e_gen computed for each block B. The initial block is
    // B_1. OUTPUT: IN[B] and OUT[B], the set of expressions available at the entry and exit of each
    // block B of the flow graph. Method: OUT[ENTRY] = 0 (empty set); for (each basic block B other
    // than ENTRY) OUT[B] = U (universal set); while (changes to any OUT occur)
    //      for (each basic block other than ENTRY) {
    //          IN[B] = && of predecessors' OUTs;
    //          OUT[B] = e_gen U (IN[B] - e_kill);
    //      }

    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_rpo = function.get_reverse_post_ordering();
    auto id_rpo_pairs = std::vector<std::pair<int, int>>(id_to_rpo.begin(), id_to_rpo.end());
    // sort by rpo
    std::sort(id_rpo_pairs.begin(), id_rpo_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    std::set<Expression> all_expressions = get_all_expressions_set(function);

    // region Print expressions
    std::cout << "Expressions: " << std::endl;
    for (auto &expr : all_expressions)
        std::cout << print_expression(expr) << std::endl;
    // endregion

    auto [id_to_de_exprs, id_to_killed_exprs] = get_downward_exposed_and_killed_expressions(function);

    // region Print e_gen and e_kill sets
    for (auto &[id, e_gen] : id_to_de_exprs) {
        std::cout << id_to_block.at(id)->get_name();
        std::cout << "DE: " + print_into_string_with(e_gen, print_expression) << std::endl;
        std::cout << "KILLED: " + print_into_string_with(id_to_killed_exprs.at(id), print_expression)
                  << std::endl;
    }
    // endregion
    // region Print DownwardExposed and Killed Expressions
    {
        std::unordered_map<int, std::string> above, below;
        for (auto &[id, e_gen] : id_to_de_exprs) {
            above.emplace(id, "DE: " + print_into_string_with(e_gen, print_expression));
            above[id] +=
                "<BR/>KILLED: " + print_into_string_with(id_to_killed_exprs.at(id), print_expression);
        }
        std::string title = "Downward Exposed and Killed Expressions<BR/>";
        title += "All Expressions: " + print_into_string_with(all_expressions, print_expression);
        function.print_cfg("lala2.png", above, below, title);
    }
    // endregion

    std::map<int, std::set<Expression>> out_sets;
    std::map<int, std::set<Expression>> in_sets;

    // OUT[ENTRY] = 0 (empty set);
    // for (each basic block B other than ENTRY) OUT[B] = U (universal set);
    auto entry_node = function.find_entry_block();
    out_sets[entry_node->id] = {};
    for (auto &b : blocks)
        if (b->id != entry_node->id)
            out_sets[b->id] = all_expressions;

    // while (changes to any OUT occur)
    bool changed = true;
    while (changed) {
        changed = false;

        // for (each basic block other than ENTRY)
        // iterate in reverse post order
        for (auto &[id, rpo] : id_rpo_pairs) {
            if (id == entry_node->id) // skip entry block
                continue;
            auto block = id_to_block.at(id);

            // IN[B] = && of predecessors' OUTs;
            std::vector<std::set<Expression>> pred_outs;
            for (auto &pred : block->predecessors)
                pred_outs.push_back(out_sets.at(pred->id));
            auto IN = intersection_of_sets(std::move(pred_outs));
            in_sets[id] = IN;

            // OUT[B] = e_gen U (IN[B] - e_kill);
            auto &e_kill = id_to_killed_exprs.at(id);
            for (auto &killed : e_kill)
                IN.erase(killed);

            auto &e_gen = id_to_de_exprs.at(id);
            auto OUT = union_of_sets(std::vector{e_gen, IN});
            if (OUT != out_sets.at(id))
                changed = true;
            out_sets[id] = OUT;
        }
    }

    // region Available Expressions Print
    std::cout << "Available Expressions" << std::endl;
    for (auto &[id, in] : in_sets) {
        std::cout << id_to_block.at(id)->get_name() << std::endl;
        std::cout << "\tIN: " + print_into_string_with(in, print_expression) << std::endl;
        std::cout << "\tOUT: " + print_into_string_with(out_sets.at(id), print_expression) << std::endl;
    }
    // endregion
    // region Print CFG
    std::unordered_map<int, std::string> above, below;
    for (auto &[id, in] : in_sets) {
        above.emplace(id, "IN: " + print_into_string_with(in, print_expression));
        below.emplace(id, "OUT: " + print_into_string_with(out_sets.at(id), print_expression));
    }
    std::string title = "Available Expressions<BR/>";
    title += "All Expressions: " + print_into_string_with(all_expressions, print_expression);
    function.print_cfg("avail1.png", above, below, title);
    // endregion

    return {in_sets, out_sets};
}
