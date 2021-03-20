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

        // Print result
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

std::pair<ID2EXPRS, ID2EXPRS> get_anticipable_expressions(Function &f) {
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
