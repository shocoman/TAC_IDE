//
// Created by shoco on 2/12/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_SPARSE_CONDITIONAL_CONSTANT_PROPAGATION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_SPARSE_CONDITIONAL_CONSTANT_PROPAGATION_HPP

#include <fmt/ranges.h>
#include <map>
#include <set>

#include "../structure/program.hpp"
#include "constant_folding.hpp"

struct SparseConditionalConstantPropagation {
    using Place = std::pair<int, int>;
    using Position = std::pair<std::string, Place>;
    using CFGEdge = std::pair<int, int>;
    using SSAEdge = std::pair<Place, Place>;

    struct Value {
        enum class Type { Bottom /*Not a constant*/, Constant, Top /*Undefined (yet)*/ };
        Type type = Type::Top;
        Operand constant;

        bool operator==(const Value &rhs) const {
            if (type == rhs.type) {
                if (type == Value::Type::Constant)
                    return constant == rhs.constant;
                return true;
            }
            return false;
        }
        bool operator!=(const Value &rhs) const { return !(*this == rhs); }
    };

    struct UseDefInfo {
        Place defined_at = {-1, -1};
        std::vector<Place> used_at;
    };

    struct IntermediateResults {
        std::map<std::string, UseDefInfo> use_def_graph;
        std::map<Position, Value> values;

        std::unordered_set<int> useless_blocks;
        std::set<Place> changed_places;
        std::set<CFGEdge> executed_edges;
        Function f_before_block_removal;

        std::vector<CFGEdge> CFGWorkList;
        std::vector<SSAEdge> SSAWorkList;
    } ir;

    Function f;
    SparseConditionalConstantPropagation(Function &f_) : f(f_) {}

    void run();
    void fill_in_use_def_graph();
    void init_worklist();
    void propagate();
    Value evaluate_over_lattice(Place place, Quad &q);
    void evaluate_assign(Place place);
    void evaluate_conditional(BasicBlock *b);
    void evaluate_phi_operands(Place place, Quad &phi);
    void evaluate_phi_result(Place place, Quad &phi);
    void evaluate_all_phis_in_block(CFGEdge edge);
    void evaluate_phi(SSAEdge edge);
    void print_result_info();
    void rewrite_program();
    void collect_useless_blocks();
    void remove_useless_blocks();
    std::vector<char> print_sccp_result_graph();
};

static void run_sparse_conditional_constant_propagation(Function &f) {
    SparseConditionalConstantPropagation sccp(f);
    sccp.run();
    f = sccp.f;
}

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_SPARSE_CONDITIONAL_CONSTANT_PROPAGATION_HPP
