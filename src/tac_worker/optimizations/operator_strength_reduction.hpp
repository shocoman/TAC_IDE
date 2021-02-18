//
// Created by shoco on 2/16/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_OPERATOR_STRENGTH_REDUCTION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_OPERATOR_STRENGTH_REDUCTION_HPP

#include <fmt/ranges.h>
#include <map>
#include <set>

#include "tac_worker/optimizations/data_flow_analyses/dominators.hpp"
#include "tac_worker/optimizations/value_numbering.hpp"
#include "tac_worker/structure/function.hpp"

struct VariableInfo {
    using Place = std::pair<int, int>;
    Place defined_at = {-1, -1};
    std::vector<Place> used_at;

    int num = 0, low = 0;
    bool visited = false;
    std::string header;
};

struct OperatorStrengthReductionDriver {

    Function &func;
    std::map<std::string, VariableInfo> useInfo;
    ID2DOMS id_to_doms;
    std::map<std::tuple<std::string, std::string, std::string>, std::string> operations_lookup_table;
    std::set<std::string> induction_variables;

    explicit OperatorStrengthReductionDriver(Function &f);

    void OSR();
    void FillInUseDefGraph();
    auto GetQuad(VariableInfo::Place p) -> Quad &;
    bool IsRegionConstant(const std::string &node_name, Operand &o);
    void Replace(const std::string &node_name);
    bool IsCandidateOperation(const std::string &node_name);
    bool IsValidUpdate(const std::string &node_name);
    void ClassifyIV(const std::vector<std::string> &SCC);
    void Process(const std::vector<std::string> &SCC);
    void DFS(const std::string &name);
    void PrintSSAGraph();
    std::string Reduce(const std::string &node_name, Operand &induction_var, Operand &region_constant);
    std::string Apply(const std::string node_name, Operand &op1, Operand &op2);
    std::string NewName();
};

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_OPERATOR_STRENGTH_REDUCTION_HPP
