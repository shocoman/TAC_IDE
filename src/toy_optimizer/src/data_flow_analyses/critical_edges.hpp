//
// Created by shoco on 5/7/2021.
//

#ifndef TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_DATA_FLOW_ANALYSES_CRITICAL_EDGES_HPP
#define TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_DATA_FLOW_ANALYSES_CRITICAL_EDGES_HPP

#include "../structure/program.hpp"

struct CriticalEdgesDriver {
    struct IntermediateResults {
        std::set<std::pair<int, int>> critical_edges;
        Function f_before_split;
    } ir;

    Function f;
    CriticalEdgesDriver(Function &f_) : f(f_) {
        run();
        ir.f_before_split = f;
    }

    void run();
    std::vector<char> print_critical_edges();
    void split_critical_edges();
};

#endif // TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_DATA_FLOW_ANALYSES_CRITICAL_EDGES_HPP
