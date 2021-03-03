//
// Created by shoco on 1/22/2021.
//

#include "data_flow_analyses.hpp"

void liveness_analyses_on_block(const BasicBlocks &nodes) {
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
    // backward data-flow analysis

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

//    print_analyses_result_on_graph(
//        f, id_to_ue_uses, id_to_defs, "Variable definitions and upward-exposed uses",
//        [](auto &v) { return v; }, "UE Uses", "Definitions");
//    print_analyses_result_on_graph(f, IN, OUT, "Live variable analyses", [](auto &v) { return v; });

    auto &uninitialized_vars = OUT.at(f.find_entry_block()->id);
    if (!uninitialized_vars.empty())
        fmt::print("Possibly uninitialized variables: {}\n", uninitialized_vars);

    return std::pair{IN, OUT};
}

void reaching_definitions(Function &function) {
    // forward data-flow analysis
    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_rpo = function.get_reverse_post_ordering();
    auto id_rpo_pairs = std::vector<std::pair<int, int>>(id_to_rpo.begin(), id_to_rpo.end());
    // sort by rpo
    std::sort(id_rpo_pairs.begin(), id_rpo_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    // calculate definitions
    struct Definition {
        int block;
        int quad;
        int count;
        std::string name;

        bool operator<(const Definition &r) const {
            return std::tie(block, quad, count, name) < std::tie(r.block, r.quad, r.count, r.name);
        }

        bool operator==(const Definition &r) const {
            return std::tie(block, quad, count, name) == std::tie(r.block, r.quad, r.count, r.name);
        }
    };

    int counter = 0;
    std::vector<Definition> definitions;
    for (int block_id = 0; block_id < blocks.size(); ++block_id) {
        auto &b = blocks[block_id];
        for (int quad_id = 0; quad_id < b->quads.size(); ++quad_id) {
            auto &q = b->quads[quad_id];
            if (q.is_assignment()) {
                definitions.push_back(Definition{block_id, quad_id, counter++, q.dest->name});
            }
        }
    }

    // region DefsPrint
    //    std::cout << "DefsPrint" << std::endl;
    //    for (auto &[b, q, c, name] : definitions) {
    //        std::cout << b << "; " << q << "; " << c << "; " << name << std::endl;
    //    }
    // endregion

    // compute 'gen' and 'kill' sets
    using DefinitionsSet = std::set<Definition>;
    std::unordered_map<int, DefinitionsSet> id_to_gen;
    std::unordered_map<int, DefinitionsSet> id_to_kill;

    for (int block_id = 0; block_id < blocks.size(); ++block_id) {
        auto &b = blocks[block_id];

        DefinitionsSet gen;
        DefinitionsSet kill;
        for (const auto &def : definitions) {
            if (def.block == block_id) {
                gen.insert(def);
                for (const auto &d : definitions)
                    if (def.count != d.count && def.name == d.name)
                        kill.insert(d);
            }
        }
        id_to_gen[b->id] = gen;
        id_to_kill[b->id] = kill;
    }

    // region GenKillPrint
    //    std::cout << "GenKillPrint" << std::endl;
    //    for (auto &[id, def] : id_to_gen) {
    //        std::cout << "ID: " << id << std::endl;
    //        std::cout << " Gen: ";
    //        for (auto &[b, q, count, name] : def) {
    //            std::cout << "( " << count + 1 << ": " << name << "), ";
    //        }
    //        std::cout << std::endl;
    //        std::cout << " Kill: ";
    //        auto &d = id_to_kill.at(id);
    //        for (auto &[b, q, count, name] : d) {
    //            std::cout << "( " << count + 1 << ": " << name << "), ";
    //        }
    //        std::cout << std::endl;
    //    }
    // endregion

    std::unordered_map<int, DefinitionsSet> in_set;
    std::unordered_map<int, DefinitionsSet> out_set;
    out_set[0] = {};
    in_set[0] = {};
    for (auto &b : blocks)
        out_set[b->id] = {};

    bool changed = true;
    while (changed) {
        changed = false;

        // iterate in reverse post order
        for (auto &[id, rpo] : id_rpo_pairs) {
            if (rpo == 0) // skip entry block
                continue;
            auto &block = id_to_block.at(id);

            // calculate IN set
            // IN[B] = Union of preds' OUTs
            DefinitionsSet IN;
            for (auto &pred : block->predecessors) {
                auto &preds_in = out_set.at(pred->id);
                for (auto &def : preds_in)
                    IN.insert(def);
            }
            in_set[id] = IN;

            // calculate OUT set
            // OUT[B] = 'gen' U (IN[B] - 'kill')
            // (IN[B] - 'kill')
            auto &killed = id_to_kill.at(id);
            for (auto &k : killed)
                IN.erase(k);

            // 'gen' U (IN[B] - 'kill')
            auto OUT = id_to_gen.at(id);
            for (auto &not_killed : IN)
                OUT.insert(not_killed);

            if (out_set.at(id) != OUT)
                changed = true;
            // OUT[B] = 'gen' U (IN[B] - 'kill')
            out_set[id] = OUT;
        }
    }

    // region Reaching Definitions Print
    //    for (auto &[id, out] : out_set) {
    //        std::cout << id << "; OUT: ";
    //        for (auto &[b, q, count, def] : out)
    //            std::cout << def << "." << count << ", ";
    //        std::cout << std::endl;
    //        auto in = in_set.at(id);
    //        std::cout << id << "; IN: ";
    //        for (auto &[b, q, count, def] : in)
    //            std::cout << def << "." << count << ", ";
    //        std::cout << std::endl;
    //        std::cout << std::endl;
    //    }
    // endregion
}

std::pair<ID2EXPRS, ID2EXPRS> available_expressions(Function &f) {
    auto all_expressions = get_all_expressions_set(f);

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
    auto all_expressions = get_all_expressions_set(f);

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
