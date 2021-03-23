//
// Created by shoco on 3/17/2021.
//

#ifndef TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_DATA_FLOW_ANALYSES_LIVE_VARIABLE_ANALYSIS_HPP
#define TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_DATA_FLOW_ANALYSES_LIVE_VARIABLE_ANALYSIS_HPP

struct LiveVariableAnalysisDriver {
    using DefinitionsSet = std::unordered_set<std::string>;
    using MapDefinitionsSets = std::unordered_map<int, DefinitionsSet>;

    struct IntermediateResults {
        std::unordered_map<int, DefinitionsSet> id_to_ue_uses, id_to_defs;
    } ir;

    Function &f;
    std::pair<ID2Defs, ID2Defs> live_variable_analysis;

    LiveVariableAnalysisDriver(Function &f_) : f(f_) { run(); }

    std::pair<ID2Defs, ID2Defs> &run() {
        ir.id_to_defs.clear();
        ir.id_to_ue_uses.clear();

        calculate_upwardexposed_uses_and_definitions();
        run_live_variable_analysis();
        return live_variable_analysis;
    }

    void calculate_upwardexposed_uses_and_definitions() {
        for (const auto &b : f.basic_blocks) {
            DefinitionsSet ue_uses_in_block, defs_in_block;

            for (const auto &q : b->quads) {
                for (auto &r : q.get_rhs_names(false))
                    if (defs_in_block.count(r) == 0)
                        ue_uses_in_block.insert(r);

                if (q.is_assignment())
                    defs_in_block.insert(q.dest->name);
            }

            ir.id_to_ue_uses[b->id] = ue_uses_in_block;
            ir.id_to_defs[b->id] = defs_in_block;
        }
    }

    void run_live_variable_analysis() {
        live_variable_analysis = data_flow_framework<std::string>(
            f, Flow::Backwards, Meet::Union, {}, [&](auto &IN, auto &OUT, int id) {
                auto X = OUT.at(id);
                for (auto &def : ir.id_to_defs.at(id))
                    X.erase(def);
                for (auto &use : ir.id_to_ue_uses.at(id))
                    X.insert(use);
                return X;
            });
    }

    std::set<std::string> get_uninitialized_variables(bool print = true) {
        auto &OUT = live_variable_analysis.second;
        auto &uninitialized_vars = OUT.at(f.get_entry_block()->id);
        if (!uninitialized_vars.empty() && print)
            fmt::print("Possibly uninitialized variables: {}\n", uninitialized_vars);
        return uninitialized_vars;
    }

    std::vector<char> print_upwardexposed_and_definitions() {
        return print_analysis_result_on_graph(
            f, ir.id_to_ue_uses, ir.id_to_defs, "Variable definitions and upward-exposed uses",
            [](auto &v) { return v; }, "UE Uses", "Definitions");
    }

    std::vector<char> print_live_variable_analysis() {
        auto &[IN, OUT] = live_variable_analysis;
        return print_analysis_result_on_graph(f, IN, OUT, "Live variable analyses",
                                              [](auto &v) { return v; });
    }

    bool all_variables_are_initialized() { return get_uninitialized_variables(false).empty(); }

    bool live_at_entry(int block_id, const std::string &name) {
        return live_variable_analysis.first.at(block_id).count(name) != 0;
    }

    bool live_at_exit(int block_id, const std::string &name) {
        return live_variable_analysis.second.at(block_id).count(name) != 0;
    }
};

#endif // TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_DATA_FLOW_ANALYSES_LIVE_VARIABLE_ANALYSIS_HPP
