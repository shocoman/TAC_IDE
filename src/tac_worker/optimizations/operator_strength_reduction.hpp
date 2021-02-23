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

// operator strength reduction
struct OSRDriver {
    Function &func;
    std::map<std::string, VariableInfo> useInfo;
    ID2DOMS id_to_doms;
    std::map<std::tuple<std::string, std::string, std::string>, std::string> operations_lookup_table;

    explicit OSRDriver(Function &f);

    void OSR();
    void FillInUseDefGraph();
    Quad &GetQuad(VariableInfo::Place p);
    bool IsCorrectIVarAndRConstPair(Operand &mb_iv, Operand &mb_rc);
    void Replace(const std::string &node_name);
    bool IsCandidateOperation(const std::string &node_name, std::string header);
    bool IsSCCValidIV(const std::vector<std::string> &SCC, std::string header);
    bool IsRegionConst(std::string &name, std::string &header);
    void ClassifyIV(const std::vector<std::string> &SCC);
    void ProcessSCC(const std::vector<std::string> &SCC);
    void DFS(const std::string &name);
    void PrintSSAGraph();
    std::string Reduce(const std::string &node_name, Operand &induction_var, Operand &reg_const);
    std::string Apply(const std::string &node_name, Operand &op1, Operand &op2);
    std::string MakeNewName();
};

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_OPERATOR_STRENGTH_REDUCTION_HPP
