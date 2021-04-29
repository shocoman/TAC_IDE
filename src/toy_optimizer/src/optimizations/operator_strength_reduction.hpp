//
// Created by shoco on 2/16/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_OPERATOR_STRENGTH_REDUCTION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_OPERATOR_STRENGTH_REDUCTION_HPP

#include <fmt/ranges.h>
#include <map>
#include <set>

#include "../data_flow_analyses/dominators.hpp"
#include "../structure/function.hpp"
#include "../utilities/new_name_generator.hpp"


struct VariableInfo {
    using Place = std::pair<int, int>;
    Place defined_at = {-1, -1};

    int num = 0, lowlink = 0;
    bool visited = false;
    std::string header;
};

// operator strength reduction
struct OperatorStrengthReductionDriver {
    using Operation = std::tuple<std::string, std::string, std::string>;
    struct IntermediateResults {
        std::map<std::string, VariableInfo> use_def_graph;
        ID2DOMS id_to_doms;
        std::map<Operation, std::string> operations_lookup_table;
        std::optional<NewNameGenerator> new_name_generator;
    } ir;

    Function &f;
    OperatorStrengthReductionDriver(Function &f_) : f(f_) {
        ir.new_name_generator.emplace(f);
        ir.id_to_doms = get_dominators(f);
        fill_in_use_def_graph();
    }

    void run();
    void fill_in_use_def_graph();
    bool IsCandidateOperation(const std::string &node_name, std::string header);
    bool IsSCCValidIV(const std::vector<std::string> &SCC, std::string header);
    bool IsRegionConst(std::string &name, std::string &header);
    void ClassifyIV(const std::vector<std::string> &SCC);
    void ProcessSCC(const std::vector<std::string> &SCC);
    void DFS(const std::string &name);
    std::vector<char> PrintSSAGraph();

    void Replace(const std::string &node_name);
    std::string Reduce(const std::string &node_name, Operand &induction_var, Operand &reg_const);
    std::string Apply(const std::string &node_name, Operand &op1, Operand &op2);
};

static void run_operator_strength_reduction(Function &f) {
    auto operator_reduction = OperatorStrengthReductionDriver(f);
    operator_reduction.run();
    f = operator_reduction.f;
}

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_OPERATOR_STRENGTH_REDUCTION_HPP
