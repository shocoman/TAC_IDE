//
// Created by shoco on 2/11/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP

#include <map>
#include <set>

#include "../data_flow_analyses/data_flow_analyses.hpp"
#include "../data_flow_analyses/expressions_analyses/utilities.hpp"
#include "../structure/function.hpp"
#include "../utilities/new_name_generator.hpp"

#include <tuple>

struct LazyCodeMotionDriver {
    struct IntermediateResults {
        std::set<Expression> all_expressions;
        ID2EXPRS id_to_ue_exprs, id_to_killed_exprs;
        std::pair<ID2EXPRS, ID2EXPRS> used_expressions, anticipable_expressions, available_expressions,
            postponable_expressions;
        ID2EXPRS latest_expressions, earliest_expressions;

        std::vector<std::string> replaced, placed;
    } ir;

    Function f;

    LazyCodeMotionDriver(Function &f_) : f(f_) {
        ir.all_expressions = get_all_expressions(f);

        // from "all_expressions"
        std::tie(ir.id_to_ue_exprs, ir.id_to_killed_exprs) =
            get_upward_exposed_and_killed_expressions(f);

        // from "all_expressions", "id_to_ue_exprs" and "id_to_killed_exprs"
        ir.anticipable_expressions = get_anticipable_expressions(f);

        // from "ir.anticipable_expressions" and "ir.id_to_killed_exprs"
        ir.available_expressions = get_available_expressions_lazy_code_motion();

        // from "ir.anticipable_expressions" and "ir.available_expressions"
        ir.earliest_expressions = get_earliest_expressions();

        // from "ir.earliest_expressions" and "ir.id_to_ue_exprs"
        ir.postponable_expressions = get_postponable_expressions();

        // from "ir.earliest_expressions" and "ir.postponable_expressions"
        ir.latest_expressions = get_latest_expressions();

        // from "ir.latest_expressions" and "ir.id_to_ue_exprs"
        ir.used_expressions = get_used_expressions();
    }

    void run_lazy_code_motion() {
        // uniquely name every expression
        NewNameGenerator new_name_generator(f);
        std::map<Expression, std::string> expr_to_name;
        for (auto &expr : ir.all_expressions)
            expr_to_name[expr] = new_name_generator.make_new_name();

        for (auto &b : f.basic_blocks) {
            if (b->quads.empty())
                continue;

            auto for_expr_placement = intersection_of_sets(
                std::vector{ir.latest_expressions.at(b->id), ir.used_expressions.second.at(b->id)});

            auto latest_complement = ir.all_expressions;
            for (auto &e : ir.latest_expressions.at(b->id))
                latest_complement.erase(e);
            auto for_name_replacement = intersection_of_sets(std::vector{
                ir.id_to_ue_exprs.at(b->id),
                union_of_sets(std::vector{latest_complement, ir.used_expressions.second.at(b->id)})});

            for (auto &expr : ir.all_expressions) {
                if (for_name_replacement.count(expr) > 0) {
                    auto &[lhs, type, rhs] = expr;

                    for (auto &q : b->quads) {
                        if (!q.is_binary() || q.dest->name == expr_to_name.at(expr))
                            continue;

                        if (expr == std::tuple(*q.get_op(0), q.type, *q.get_op(1))) {
                            q.clear_op(1);
                            q.type = Quad::Type::Assign;
                            q.ops[0] = Operand(expr_to_name.at(expr), Operand::Type::Var);

                            std::cout << "Replaced: " << q.fmt() << std::endl;
                            ir.replaced.push_back(q.fmt());
                        }
                    }
                }

                if (for_expr_placement.count(expr) > 0) {
                    auto &[lhs, type, rhs] = expr;
                    auto new_quad = Quad(lhs, rhs, type);
                    new_quad.dest = Dest(expr_to_name.at(expr), {}, Dest::Type::Var);
                    // if block doesn't contain it already
                    if (std::find(b->quads.begin(), b->quads.end(), new_quad) == b->quads.end())
                        b->quads.insert(b->quads.begin() + b->phi_functions, new_quad);

                    std::cout << "Put: " << new_quad.fmt() << std::endl;
                    ir.placed.push_back(new_quad.fmt());
                }
            }
        }
    }

    std::pair<ID2EXPRS, ID2EXPRS> get_available_expressions_lazy_code_motion() {
        return data_flow_framework<Expression>(
            f, Flow::Forwards, Meet::Intersection, ir.all_expressions,
            [&](ID2EXPRS &IN, ID2EXPRS &OUT, int id) {
                auto X = IN.at(id);
                for (auto &expr : ir.anticipable_expressions.first.at(id))
                    X.insert(expr);
                for (auto &expr : ir.id_to_killed_exprs.at(id))
                    X.erase(expr);
                return X;
            });
    }

    ID2EXPRS get_earliest_expressions() {
        auto X = ir.anticipable_expressions.first;
        for (auto &[id, exprs] : ir.available_expressions.first) {
            auto &x = X.at(id);
            for (auto &e : exprs)
                x.erase(e);
        }
        return X;
    }

    std::pair<ID2EXPRS, ID2EXPRS> get_postponable_expressions() {
        return data_flow_framework<Expression>(f, Flow::Forwards, Meet::Intersection, ir.all_expressions,
                                               [&](ID2EXPRS &IN, ID2EXPRS &OUT, int id) {
                                                   auto X = IN.at(id);
                                                   for (auto &expr : ir.earliest_expressions.at(id))
                                                       X.insert(expr);
                                                   for (auto &expr : ir.id_to_ue_exprs.at(id))
                                                       X.erase(expr);
                                                   return X;
                                               });
    }

    ID2EXPRS get_latest_expressions() {
        ID2EXPRS latest_expressions;
        for (auto &b : f.basic_blocks) {
            std::vector<std::set<Expression>> successors_exprs;
            std::transform(b->successors.begin(), b->successors.end(),
                           std::back_inserter(successors_exprs), [&](auto s) {
                               return union_of_sets(
                                   std::vector{ir.earliest_expressions.at(s->id),
                                               ir.postponable_expressions.first.at(s->id)});
                           });

            auto complementary = ir.all_expressions;
            for (auto &e : intersection_of_sets(successors_exprs))
                complementary.erase(e);

            latest_expressions[b->id] = intersection_of_sets(
                std::vector{union_of_sets(std::vector{ir.earliest_expressions[b->id],
                                                      ir.postponable_expressions.first[b->id]}),
                            union_of_sets(std::vector{ir.id_to_ue_exprs.at(b->id), complementary})});
        }

        return latest_expressions;
    }

    std::pair<ID2EXPRS, ID2EXPRS> get_used_expressions() {
        return data_flow_framework<Expression>(f, Flow::Backwards, Meet::Union, {},
                                               [&](ID2EXPRS &IN, ID2EXPRS &OUT, int id) {
                                                   auto X = OUT.at(id);
                                                   for (auto &expr : ir.id_to_ue_exprs.at(id))
                                                       X.insert(expr);
                                                   for (auto &expr : ir.latest_expressions.at(id))
                                                       X.erase(expr);
                                                   return X;
                                               });
    }
};

static void run_lazy_code_motion(Function &f) {
    LazyCodeMotionDriver lazy_code_motion_driver(f);
    lazy_code_motion_driver.run_lazy_code_motion();
    f = lazy_code_motion_driver.f;
}

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP
