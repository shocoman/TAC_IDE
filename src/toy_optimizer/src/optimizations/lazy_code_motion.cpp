//
// Created by shoco on 2/11/2021.
//

#include "lazy_code_motion.hpp"

void LazyCodeMotionDriver::run() {
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
                new_quad.dest = Dest(expr_to_name.at(expr), Dest::Type::Var);
                // if block doesn't contain it already
                if (std::find(b->quads.begin(), b->quads.end(), new_quad) == b->quads.end())
                    b->quads.insert(b->quads.begin() + b->phi_functions, new_quad);

                std::cout << "Put: " << new_quad.fmt() << std::endl;
                ir.placed.push_back(new_quad.fmt());
            }
        }
    }
}

std::pair<ID2EXPRS, ID2EXPRS> LazyCodeMotionDriver::get_available_expressions_lazy_code_motion() {
    return data_flow_framework<Expression>(f, Flow::Forwards, Meet::Intersection, ir.all_expressions,
                                           [&](ID2EXPRS &IN, ID2EXPRS &OUT, int id) {
                                               auto X = IN.at(id);
                                               for (auto &expr : ir.anticipable_expressions.first.at(id))
                                                   X.insert(expr);
                                               for (auto &expr : ir.id_to_killed_exprs.at(id))
                                                   X.erase(expr);
                                               return X;
                                           });
}

ID2EXPRS LazyCodeMotionDriver::get_earliest_expressions() {
    auto &ant_in = ir.anticipable_expressions.first;
    auto &avail_in = ir.available_expressions.first;
    auto X = ant_in;
    for (auto &[id, exprs] : avail_in) {
        auto &x = X.at(id);
        for (auto &e : exprs)
            x.erase(e);
    }
    return X;
}

std::pair<ID2EXPRS, ID2EXPRS> LazyCodeMotionDriver::get_postponable_expressions() {
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

ID2EXPRS LazyCodeMotionDriver::get_latest_expressions() {
    auto &post_in = ir.postponable_expressions.first;
    ID2EXPRS latest_expressions;
    for (auto &b : f.basic_blocks) {
        std::vector<std::set<Expression>> successors_exprs;
        std::transform(
            b->successors.begin(), b->successors.end(), std::back_inserter(successors_exprs),
            [&](auto s) {
                return union_of_sets(std::vector{ir.earliest_expressions.at(s->id), post_in.at(s->id)});
            });

        auto complementary = ir.all_expressions;
        for (auto &e : intersection_of_sets(successors_exprs))
            complementary.erase(e);

        latest_expressions[b->id] = intersection_of_sets(
            std::vector{union_of_sets(std::vector{ir.earliest_expressions[b->id], post_in[b->id]}),
                        union_of_sets(std::vector{ir.id_to_ue_exprs.at(b->id), complementary})});
    }

    return latest_expressions;
}

std::pair<ID2EXPRS, ID2EXPRS> LazyCodeMotionDriver::get_used_expressions() {
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

void LazyCodeMotionDriver::preprocess() {
    ir.all_expressions = get_all_expressions(f);

    // from "ir.all_expressions"
    std::tie(ir.id_to_ue_exprs, ir.id_to_killed_exprs) = get_upward_exposed_and_killed_expressions(f);

    // from "ir.all_expressions", "ir.id_to_ue_exprs" and "ir.id_to_killed_exprs"
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

void LazyCodeMotionDriver::print_lazy_code_motion_graphs() {
    auto &[AntIn, AntOut] = ir.anticipable_expressions;
    auto &[AvailIn, AvailOut] = ir.available_expressions;
    auto &earliest_exprs = ir.earliest_expressions;
    auto &latest_exprs = ir.latest_expressions;
    auto &[PostIn, PostOut] = ir.postponable_expressions;
    auto &[UsedIn, UsedOut] = ir.used_expressions;

    print_analysis_result_on_graph(f, AntIn, AntOut, "Anticipable Expressions (1)", print_expression);
    print_analysis_result_on_graph(f, AvailIn, AvailOut, "Available Expressions (lcm) (2)",
                                   print_expression);
    print_analysis_result_on_graph(f, earliest_exprs, {}, "Earliest Expressions (3)", print_expression,
                                   "Earliest");
    print_analysis_result_on_graph(f, PostIn, PostOut, "Postponable Expressions (4)", print_expression);
    print_analysis_result_on_graph(f, latest_exprs, {}, "Latest Expressions (5)", print_expression,
                                   "Latest");
    print_analysis_result_on_graph(f, UsedIn, UsedOut, "Used Expressions (6)", print_expression);
    print_analysis_result_on_graph(f, ir.id_to_ue_exprs, ir.id_to_killed_exprs,
                                   "UE and Killed Expressions", print_expression, "UE", "Kill");
}
