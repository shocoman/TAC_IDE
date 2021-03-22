//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_USELESS_CODE_ELIMINATION_HPP
#define TAC_PARSER_USELESS_CODE_ELIMINATION_HPP

#include <map>
#include <set>

#include "../data_flow_analyses/dominators.hpp"
#include "../data_flow_analyses/use_def_graph.hpp"
#include "../structure/function.hpp"

struct UselessCodeEliminationDriver {
    struct IntermediateResults {
        ID2IDOM id_to_ipostdom;
        ID2DF reverse_df;
        std::optional<UseDefGraph> use_def_graph;
        std::set<UseDefGraph::Location> critical_operations;
        std::set<int> marked_blocks;
    } ir;

    Function f;

    UselessCodeEliminationDriver(Function &f_) : f(f_) {}
    Function &run();
    void compute_reverse_dominance_frontier();
    void remove_noncritical_operations();
    void remove_unreachable_blocks();
    void merge_basic_blocks();
};

static void run_useless_code_elimination(Function &f) {
    UselessCodeEliminationDriver useless_code_elimination_driver(f);
    f = useless_code_elimination_driver.run();
};

#endif // TAC_PARSER_USELESS_CODE_ELIMINATION_HPP
