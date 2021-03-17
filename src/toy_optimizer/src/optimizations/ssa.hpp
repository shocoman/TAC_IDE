//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_SSA_HPP
#define TAC_PARSER_SSA_HPP

#include <numeric>
#include <set>

#include "../data_flow_analyses/data_flow_analyses.hpp"
#include "../data_flow_analyses/dominators.hpp"
#include "../data_flow_analyses/print_graph.hpp"
#include "../data_flow_analyses/live_variable_analysis.hpp"
#include "../structure/function.hpp"
#include "../utilities/graph_writer/graph_writer.hpp"
#include "../utilities/new_name_generator.hpp"
#include "../utilities/quad_preparation.hpp"
#include "value_numbering.hpp"

struct ConvertToSSADriver {
    struct IntermediateResults {
        std::set<std::string> global_names, all_names;
        std::map<std::string, std::set<BasicBlock *>> var_to_block;
        Function f_before_convert, f_after_phi_placement, f_after_renaming;
    } ir;

    Function f;

    ConvertToSSADriver(const Function &f_) : f(f_) { run(); }

    Function &run();
    void find_global_names();
    void place_phi_functions();
    void rename_variables();
};

struct ConvertFromSSADriver {
    struct IntermediateResults {
        std::set<std::string> global_names, all_names;
        std::map<std::string, std::set<BasicBlock *>> var_to_block;
        Function f_before_convert, f_after_renaming, f_after_phi_removal;

        ID2IDOM id_to_idom;
        std::map<std::string, std::vector<std::string>> stacks;
        std::optional<LiveVariableAnalysisDriver> liveness;
        std::optional<NewNameGenerator> new_name_generator;
    } ir;

    Function f;

    ConvertFromSSADriver() = delete;
    ConvertFromSSADriver(Function &f_) : f(f_) {
        run();
    }

    Function &run();
    void schedule_copies(BasicBlock *b);
    void insert_copies(BasicBlock *b);
    void convert_from_ssa();
};

#endif // TAC_PARSER_SSA_HPP
