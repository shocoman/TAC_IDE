//
// Created by shoco on 2/8/2021.
//

#include "anticipable_expressions.hpp"

void anticipable_expressions(Function &function) {
    // backward data-flow problem
    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_po = function.get_post_ordering();
    auto id_po_pairs = std::vector<std::pair<int, int>>(id_to_po.begin(), id_to_po.end());
    // sort by po
    std::sort(id_po_pairs.begin(), id_po_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    // Collect all expressions (universal set)
    using Expression = std::tuple<Operand, Quad::Type, Operand>;
    std::set<Expression> all_expressions;
    for (auto &b : blocks) {
        for (auto &q : b->quads) {
            if (q.is_binary()) {
                Expression expr = {*q.get_op(0), q.type, *q.get_op(1)};
                all_expressions.insert(expr);
            }
        }
    }

    auto print_expression = [](Expression expr, bool print = false, bool with_endl = false) {
        auto &[lhs, type, rhs] = expr;
        auto fmt = Quad(lhs, rhs, type).fmt(true);
        if (print) {
            std::cout << fmt;
            if (with_endl)
                std::cout << std::endl;
            else
                std::cout << ", ";
        }
        return fmt;
    };

    // region Print all expressions
    std::cout << "Expressions: " << std::endl;
    for (auto &expr : all_expressions)
        print_expression(expr, true);
    // endregion

    // Calculate e_gen and e_kill sets for each block
    std::map<int, std::set<Expression>> id_to_e_gen;
    std::map<int, std::set<Expression>> id_to_e_kill;
    for (auto &b : blocks) {
        std::set<Expression> e_gen; // upward exposed expressions (used before killed)
        std::set<Expression> e_kill;
        std::set<std::string> redefined;

        for (auto &q : b->quads) {
            // check generated expressions
            if (q.is_binary()) {
                auto lhs = *q.get_op(0);
                auto rhs = *q.get_op(1);
                Expression expr = {lhs, q.type, rhs};
                if (redefined.count(lhs.value) == 0 && redefined.count(rhs.value) == 0) {
                    e_gen.insert(expr);
                }
            }

            // check killed expressions
            if (q.is_assignment()) {
                auto def = q.dest->name;
                redefined.insert(def);
                for (auto &expr : all_expressions) {
                    auto &[lhs, type, rhs] = expr;
                    if (def == lhs.value || def == rhs.value)
                        e_kill.insert(expr);
                }
            }
        }

        id_to_e_gen[b->id] = e_gen;
        id_to_e_kill[b->id] = e_kill;
    }

    // region Print UpwardExposed and Killed Expressions to Console
    for (auto &[id, e_gen] : id_to_e_gen) {
        std::cout << id_to_block.at(id)->get_name();
        std::cout << "UE: " + print_into_string_with(e_gen, print_expression) << std::endl;
        std::cout << "KILLED: " + print_into_string_with(id_to_e_kill.at(id), print_expression)
                  << std::endl;
    }
    // endregion
    // region Print UpwardExposed and Killed Expressions CFG
    {
        std::unordered_map<int, std::string> above, below;
        for (auto &[id, e_gen] : id_to_e_gen) {
            above.emplace(id, "UE: " + print_into_string_with(e_gen, print_expression));
            above[id] += "<BR/>KILLED: " + print_into_string_with(id_to_e_kill.at(id), print_expression);
        }
        std::string title = "Upward Exposed and Killed Expressions<BR/>";
        title += "All Expressions: " + print_into_string_with(all_expressions, print_expression);
        function.print_cfg("lala2.png", above, below, title);
    }
    // endregion

    std::map<int, std::set<Expression>> in_sets;
    std::map<int, std::set<Expression>> out_sets;

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
            auto &e_kill = id_to_e_kill.at(id);
            for (auto &killed : e_kill)
                OUT.erase(killed);

            auto &e_gen = id_to_e_gen.at(id);
            auto IN = union_of_sets(std::vector{e_gen, OUT});
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
    std::unordered_map<int, std::string> above, below;
    for (auto &[id, out] : out_sets) {
        above.emplace(id, "IN: " + print_into_string_with(in_sets.at(id), print_expression));
        below.emplace(id, "OUT: " + print_into_string_with(out, print_expression));
    }
    std::string title = "Anticipable Expressions<BR/>";
    title += "All Expressions: " + print_into_string_with(all_expressions, print_expression);
    function.print_cfg("lala.png", above, below, title);
    // endregion
}