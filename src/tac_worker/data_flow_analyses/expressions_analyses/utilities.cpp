//
// Created by shoco on 2/9/2021.
//

#include "utilities.hpp"

std::set<Expression> get_all_expressions(Function &f) {
    std::set<Expression> all_expressions;
    for (auto &b : f.basic_blocks)
        for (auto &q : b->quads)
            if (q.is_binary())
                all_expressions.emplace(*q.get_op(0), q.type, *q.get_op(1));
    return all_expressions;
}

std::string print_expression(Expression expr) {
    auto &[lhs, type, rhs] = expr;
    return escape_string(Quad(lhs, rhs, type).fmt(true));
}

std::pair<ID2EXPRS, ID2EXPRS> get_upward_exposed_and_killed_expressions(Function &f) {
    auto all_expressions = get_all_expressions(f);

    ID2EXPRS id_to_ue_exprs, id_to_killed_exprs;
    for (auto &b : f.basic_blocks) {
        std::set<Expression> ue_exprs; // upward exposed expressions (used before killed)
        std::set<Expression> killed_exprs;
        std::set<std::string> redefined_exprs;

        for (auto &q : b->quads) {
            // check generated expressions
            if (q.is_binary()) {
                auto lhs = *q.get_op(0);
                auto rhs = *q.get_op(1);
                if (redefined_exprs.count(lhs.value) == 0 && redefined_exprs.count(rhs.value) == 0)
                    ue_exprs.insert({lhs, q.type, rhs});
            }

            // check killed expressions
            if (q.is_assignment()) {
                auto def = q.dest->name;
                redefined_exprs.insert(def);
                for (auto &expr : all_expressions) {
                    auto &[lhs, type, rhs] = expr;
                    if (def == lhs.value || def == rhs.value)
                        killed_exprs.insert(expr);
                }
            }
        }

        id_to_ue_exprs[b->id] = ue_exprs;
        id_to_killed_exprs[b->id] = killed_exprs;
    }

    return {id_to_ue_exprs, id_to_killed_exprs};
}

std::pair<ID2EXPRS, ID2EXPRS> get_downward_exposed_and_killed_expressions(Function &f) {
    auto all_expressions = get_all_expressions(f);

    // Calculate e_gen and e_kill sets for each block
    std::map<int, std::set<Expression>> id_to_e_gen, id_to_e_kill;
    for (auto &b : f.basic_blocks) {
        // downward exposed expressions (non-killed or redefined in block)
        std::set<Expression> e_gen, e_kill;

        for (auto &q : b->quads) {
            // check generated expressions
            if (q.is_binary())
                e_gen.insert({*q.get_op(0), q.type, *q.get_op(1)});

            // check killed expressions
            if (q.is_assignment()) {
                auto def = q.dest->name;
                for (auto &expr : all_expressions) {
                    auto &[lhs, type, rhs] = expr;
                    if (def == lhs.value || def == rhs.value) {
                        e_kill.insert(expr);
                        e_gen.erase(expr);
                    }
                }
            }
        }

        id_to_e_gen[b->id] = e_gen;
        id_to_e_kill[b->id] = e_kill;
    }

    return {id_to_e_gen, id_to_e_kill};
}

std::string split_long_string(std::string str, int max_length) {
    std::string delimiter = "<BR/>";
    for (int j = 0, i = max_length; i < str.size(); i += max_length, j += delimiter.length()) {
        int pos = str.find(',', i+j);
        if (pos >= str.size()-1) break;
        str.insert(pos + 1, delimiter);
    }
    return str;
}
