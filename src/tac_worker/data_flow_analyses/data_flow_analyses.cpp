//
// Created by shoco on 1/22/2021.
//

#include "data_flow_analyses.hpp"

void liveness_analyses_on_block_level(const BasicBlocks &nodes) {
    // block level liveness analyses
    struct LivenessState {
        bool live;
        int next_use;
    };

    for (auto &n : nodes) {
        std::vector<std::map<std::string, LivenessState>> block_liveness_data;
        std::map<std::string, LivenessState> block_liveness_nametable;

        // init vars used in the block
        for (auto &q : n->quads)
            for (auto &u : q.get_used_vars())
                block_liveness_nametable.emplace(u, LivenessState{true, -1});

        for (int i = n->quads.size() - 1; i >= 0; --i) {
            std::map<std::string, LivenessState> current_nametable;
            auto &q = n->quads[i];
            auto lhs = q.get_lhs();
            auto rhs = q.get_rhs();

            // Step 1. attach info about x,y,z to i
            if (lhs.has_value())
                current_nametable[lhs.value()] = block_liveness_nametable.at(lhs.value());

            for (auto &r : rhs)
                current_nametable[r] = block_liveness_nametable.at(r);

            // save in reversed order
            block_liveness_data.emplace(block_liveness_data.begin(), current_nametable);
            // Step 2. update name table about x (live=false)
            if (lhs.has_value())
                block_liveness_nametable[lhs.value()] = LivenessState{false, -69};

            // Step 3. update name table about y,z (live=true, next_use=i)
            for (const auto &r : rhs)
                block_liveness_nametable[r] = LivenessState{true, i};
        }

        for (int i = 0; i < n->quads.size(); ++i) {
            std::cout << n->quads[i].fmt() << ";\t";
            auto &l = block_liveness_data[i];
            for (auto &[name, liveness] : l) {
                std::cout << "[ " << name << "; Live: " << liveness.live
                          << "; Next use: " << liveness.next_use << " ]; ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

std::pair<ID2Defs, ID2Defs> live_variable_analyses(Function &f) {
    // backwards data-flow analysis

    // calculate upward-exposed uses and var definitions
    using DefinitionsSet = std::unordered_set<std::string>;
    std::unordered_map<int, DefinitionsSet> id_to_ue_uses, id_to_defs;
    for (const auto &b : f.basic_blocks) {
        DefinitionsSet ue_uses_in_block, defs_in_block;

        for (const auto &q : b->quads) {
            for (auto &r : q.get_rhs(false))
                if (defs_in_block.count(r) == 0)
                    ue_uses_in_block.insert(r);

            if (q.is_assignment())
                defs_in_block.insert(q.dest->name);
        }

        id_to_ue_uses[b->id] = ue_uses_in_block;
        id_to_defs[b->id] = defs_in_block;
    }

    auto [IN, OUT] = data_flow_framework<std::string>(f, Flow::Backwards, Meet::Union, {},
                                                      [&](auto &IN, auto &OUT, int id) {
                                                          auto X = OUT.at(id);
                                                          for (auto &def : id_to_defs.at(id))
                                                              X.erase(def);
                                                          for (auto &use : id_to_ue_uses.at(id))
                                                              X.insert(use);
                                                          return X;
                                                      });

    //    print_analysis_result_on_graph(
    //        f, id_to_ue_uses, id_to_defs, "Variable definitions and upward-exposed uses",
    //        [](auto &v) { return v; }, "UE Uses", "Definitions");
    //    print_analysis_result_on_graph(f, IN, OUT, "Live variable analyses", [](auto &v) { return v;
    //    });

    auto &uninitialized_vars = OUT.at(f.get_entry_block()->id);
    if (!uninitialized_vars.empty())
        fmt::print("Possibly uninitialized variables: {}\n", uninitialized_vars);

    return std::pair{IN, OUT};
}


std::pair<ID2EXPRS, ID2EXPRS> available_expressions(Function &f) {
    auto all_expressions = get_all_expressions(f);

    auto [id_to_de_exprs, id_to_killed_exprs] = get_downward_exposed_and_killed_expressions(f);

    auto [IN, OUT] =
        data_flow_framework<Expression>(f, Flow::Forwards, Meet::Intersection, all_expressions,
                                        [&](ID2EXPRS &IN, ID2EXPRS &OUT, int id) {
                                            auto X = IN.at(id);
                                            for (auto &expr : id_to_killed_exprs.at(id))
                                                X.erase(expr);
                                            for (auto &expr : id_to_de_exprs.at(id))
                                                X.insert(expr);
                                            return X;
                                        });
    return {IN, OUT};
}

std::pair<ID2EXPRS, ID2EXPRS> anticipable_expressions(Function &f) {
    auto all_expressions = get_all_expressions(f);

    auto [id_to_ue_exprs, id_to_killed_exprs] = get_upward_exposed_and_killed_expressions(f);

    auto [IN, OUT] =
        data_flow_framework<Expression>(f, Flow::Backwards, Meet::Intersection, all_expressions,
                                        [&](ID2EXPRS &IN, ID2EXPRS &OUT, int id) {
                                            auto X = OUT.at(id);
                                            for (auto &expr : id_to_killed_exprs.at(id))
                                                X.erase(expr);
                                            for (auto &expr : id_to_ue_exprs.at(id))
                                                X.insert(expr);
                                            return X;
                                        });

    return {IN, OUT};
}
