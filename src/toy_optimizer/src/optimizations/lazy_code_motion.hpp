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
        ID2EXPRS latest_expressions, earliest_expressions;
        std::pair<ID2EXPRS, ID2EXPRS> used_expressions, anticipable_expressions, available_expressions,
            postponable_expressions;

        std::vector<std::string> replaced, placed;
    } ir;

    Function f;

    LazyCodeMotionDriver(Function &f_) : f(f_) { preprocess(); }

    void preprocess();
    void run_lazy_code_motion();

    ID2EXPRS get_earliest_expressions();
    ID2EXPRS get_latest_expressions();
    std::pair<ID2EXPRS, ID2EXPRS> get_available_expressions_lazy_code_motion();
    std::pair<ID2EXPRS, ID2EXPRS> get_postponable_expressions();
    std::pair<ID2EXPRS, ID2EXPRS> get_used_expressions();
};

static void run_lazy_code_motion(Function &f) {
    LazyCodeMotionDriver lazy_code_motion_driver(f);
    lazy_code_motion_driver.run_lazy_code_motion();
    f = lazy_code_motion_driver.f;
}

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_LAZY_CODE_MOTION_HPP
