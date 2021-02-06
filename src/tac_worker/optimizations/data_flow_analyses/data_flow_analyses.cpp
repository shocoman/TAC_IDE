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
        for (auto &q : n->quads) {
            for (auto &u : q.get_used_vars()) {
                block_liveness_nametable.emplace(u, LivenessState{true, -1});
            }
        }

        for (int i = n->quads.size() - 1; i >= 0; --i) {
            std::map<std::string, LivenessState> current_nametable;
            auto &q = n->quads[i];
            auto lhs = q.get_lhs();
            auto rhs = q.get_rhs();

            // Step 1. attach info about x,y,z to i
            if (lhs.has_value()) {
                current_nametable[lhs.value()] = block_liveness_nametable.at(lhs.value());
            }
            for (auto &r : rhs) {
                current_nametable[r] = block_liveness_nametable.at(r);
            }
            // save in reversed order
            block_liveness_data.emplace(block_liveness_data.begin(), current_nametable);
            // Step 2. update name table about x (live=false)
            if (lhs.has_value()) {
                block_liveness_nametable[lhs.value()] = LivenessState{false, -69};
            }
            // Step 3. update name table about y,z (live=true, next_use=i)
            for (const auto &r : rhs) {
                block_liveness_nametable[r] = LivenessState{true, i};
            }
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

void liveness_analyses_engineering_compiler(Function &function) {
    BasicBlocks &blocks = function.basic_blocks;

    struct BlockLiveState {
        std::set<std::string> UEVar;   // upward exposed variables
        std::set<std::string> VarKill; // killed (re-assigned) variables
        std::set<std::string> LiveOut; // 'live' variables
    };
    std::map<int, BlockLiveState> block_live_states;

    // get UEVar and VarKill
    for (auto &b : blocks) {
        BlockLiveState b_state;

        for (const auto &q : b->quads) {
            for (auto &r : q.get_rhs(false))
                if (b_state.VarKill.find(r) == b_state.VarKill.end())
                    b_state.UEVar.emplace(r);

            if (auto lhs = q.get_lhs(); lhs.has_value())
                b_state.VarKill.insert(lhs.value());
        }

        block_live_states[b->id] = b_state;
    }

    auto live_out = [&block_live_states](BasicBlock *b) {
        std::set<std::string> &live_out_state = block_live_states.at(b->id).LiveOut;
        auto prev_live_out_state = live_out_state;

        // IN[B] = 'use' U (OUT[B] - 'def')
        for (const auto &s : b->successors) {
            auto &state = block_live_states.at(s->id);

            std::set_union(live_out_state.begin(), live_out_state.end(), state.UEVar.begin(),
                           state.UEVar.end(), std::inserter(live_out_state, live_out_state.end()));

            std::set<std::string> live_without_varkill;
            std::set_difference(state.LiveOut.begin(), state.LiveOut.end(), state.VarKill.begin(),
                                state.VarKill.end(),
                                std::inserter(live_without_varkill, live_without_varkill.end()));

            std::set_union(live_out_state.begin(), live_out_state.end(),
                           live_without_varkill.begin(), live_without_varkill.end(),
                           std::inserter(live_out_state, live_out_state.end()));
        }

        return prev_live_out_state != live_out_state;
    };

    int iter = 0;
    bool changed = true;
    while (changed) {
        changed = false;
        iter++;
        for (const auto &b : blocks) {
            if (live_out(b.get()))
                changed = true;
        }
    }

    // region Live Analyses Print
    std::cout << "Iterations: " << iter << std::endl;
    for (auto &[i, b] : block_live_states) {
        std::cout << "LiveOut for " << function.id_to_block.at(i)->get_name() << ": ";
        for (auto &a : b.LiveOut) {
            std::cout << a << "; ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    // endregion
}

void liveness_analyses_dragon_book(Function &function) {
    // backward data-flow analysis
    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_po = function.get_post_ordering();
    auto id_po_pairs = std::vector<std::pair<int, int>>(id_to_po.begin(), id_to_po.end());
    // sort in post order
    std::sort(id_po_pairs.begin(), id_po_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    // calculate definitions
    using DefinitionsSet = std::unordered_set<std::string>;
    std::unordered_map<int, DefinitionsSet> id_to_uses;
    std::unordered_map<int, DefinitionsSet> id_to_defs;
    for (const auto &b : blocks) {
        DefinitionsSet uses_in_block;
        DefinitionsSet defs_in_block;

        for (const auto &q : b->quads) {
            for (auto &r : q.get_rhs(false))
                uses_in_block.insert(r);

            if (q.is_assignment())
                defs_in_block.insert(q.dest->name);
        }

        id_to_uses[b->id] = uses_in_block;
        id_to_defs[b->id] = defs_in_block;
    }

    // region GenKillPrint
    std::cout << "DefUsePrint" << std::endl;
    for (auto &[id, uses] : id_to_uses) {
        std::cout << "Block: " << id_to_block.at(id)->get_name() << std::endl;
        std::cout << " Uses: ";
        for (auto &name : uses) {
            std::cout << name << ", ";
        }
        std::cout << std::endl;
        std::cout << " DefinitionsSet: ";
        auto &defs = id_to_defs.at(id);
        for (auto &name : defs) {
            std::cout << name << ", ";
        }
        std::cout << std::endl;
    }
    // endregion

    std::unordered_map<int, DefinitionsSet> in_sets;
    std::unordered_map<int, DefinitionsSet> out_sets;

    auto exit_node = function.find_exit_block();
    for (auto &b : blocks)
        in_sets[b->id] = {};

    bool changed = true;
    while (changed) {
        changed = false;

        // iterate in post order
        for (auto &[id, po] : id_po_pairs) {
            if (po == exit_node->id) // skip exit block
                continue;
            auto &block = id_to_block.at(id);

            // calculate OUT set
            // OUT[B] = Union of successors' INs
            DefinitionsSet OUT;
            for (auto &succ : block->successors) {
                auto &succs_in = in_sets.at(succ->id);
                for (auto &d : succs_in)
                    OUT.insert(d);
            }
            out_sets[id] = OUT;

            // calculate IN set
            // IN[B] = 'use' U (OUT[B] - 'def')
            // (OUT[B] - 'def')
            auto &defined = id_to_defs.at(id);
            for (auto &d : defined)
                OUT.erase(d);

            // 'use' U (OUT[B] - 'def')
            auto IN = id_to_uses.at(id);
            for (auto &not_redefined : OUT)
                IN.insert(not_redefined);

            if (in_sets.at(id) != IN)
                changed = true;
            // IN[B] = 'use' U (OUT[B] - 'def')
            in_sets[id] = IN;
        }
    }

    // region LiveVariable Print
    for (auto &[id, out] : out_sets) {
        auto block_name = id_to_block.at(id)->get_name();
        std::cout << block_name << "; OUT: ";
        for (auto &name : out)
            std::cout << name << ", ";
        std::cout << std::endl;
        auto in = in_sets.at(id);
        std::cout << block_name << "; IN: ";
        for (auto &name : in)
            std::cout << name << ", ";
        std::cout << std::endl << std::endl;
    }
    // endregion
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

void available_expressions(Function &function) {
    // forward data-flow analysis
    // Algorithm 9.17 : Available expressions.
    // INPUT: A flow graph with e_kill and e_gen computed for each block B. The initial block is
    // B_1. OUTPUT: IN[B] and OUT[B], the set of expressions available at the entry and exit of each
    // block B of the flow graph. Method: OUT[ENTRY] = 0 (empty set); for (each basic block B other
    // than ENTRY) OUT[B] = U (universal set); while (changes to any OUT occur)
    //      for (each basic block other than ENTRY) {
    //          IN[B] = && of predecessors' OUTs;
    //          OUT[B] = e_gen U (IN[B] - e_kill);
    //      }

    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_rpo = function.get_reverse_post_ordering();
    auto id_rpo_pairs = std::vector<std::pair<int, int>>(id_to_rpo.begin(), id_to_rpo.end());
    // sort by rpo
    std::sort(id_rpo_pairs.begin(), id_rpo_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    // Collect all expressions (universal set)
    using Expression = std::tuple<Operand, Quad::Type, Operand>;
    std::set<Expression> all_expressions;
    for (auto &b : blocks)
        for (auto &q : b->quads)
            if (q.is_binary())
                all_expressions.emplace(*q.get_op(0), q.type, *q.get_op(1));

    auto print_expression = [](Expression expr, bool print = false, bool with_endl = true) {
        auto &[lhs, type, rhs] = expr;
        auto fmt = Quad(lhs, rhs, type).fmt(true);
        if (print) {
            std::cout << fmt;
            if (with_endl)
                std::cout << std::endl;
            else
                std::cout << ", ";
        }
        return fmt;
    };

    // region Print expressions
    std::cout << "Expressions: " << std::endl;
    for (auto &expr : all_expressions)
        print_expression(expr);
    // endregion

    // Calculate e_gen and e_kill sets for each block
    std::map<int, std::set<Expression>> id_to_e_gen;
    std::map<int, std::set<Expression>> id_to_e_kill;
    for (auto &b : blocks) {
        // downward exposed expressions (non-killed or redefined in block)
        std::set<Expression> e_gen;
        std::set<Expression> e_kill;

        for (auto &q : b->quads) {
            // check generated expressions
            if (q.is_binary()) {
                Expression expr = {*q.get_op(0), q.type, *q.get_op(1)};
                e_gen.insert(expr);
                // e_kill.erase(expr);
            }

            // check killed expressions
            if (q.is_assignment()) {
                auto def = q.dest->name;
                //                auto e_gen_copy = e_gen;
                for (auto &expr : /*e_gen_copy*/ all_expressions) {
                    auto &[lhs, type, rhs] = expr;
                    if (def == lhs.value || def == rhs.value) {
                        e_kill.insert(expr);
                        e_gen.erase(expr);
                    }
                }
            }
        }

        id_to_e_gen[b->id] = e_gen;
        id_to_e_kill[b->id] = e_kill;
    }

    // region Print e_gen and e_kill sets
    for (auto &[id, e_gen] : id_to_e_gen) {
        std::cout << id_to_block.at(id)->get_name() << "; Downward exposed: ";
        for (auto &expr : e_gen)
            print_expression(expr, true);
        std::cout << std::endl;
        auto e_kill = id_to_e_kill.at(id);
        std::cout << "\tKilled: ";
        for (auto &expr : e_kill)
            print_expression(expr, true);
        std::cout << std::endl << std::endl;
    }
    // endregion
    // region Print CFG
    {
        std::unordered_map<int, std::string> above, below;
        for (auto &[id, out] : id_to_e_gen) {
            std::string above_text = "{", below_text = "{";
            for (auto &expr : out)
                above_text += print_expression(expr) + ", ";
            for (auto &expr : id_to_e_kill.at(id))
                below_text += print_expression(expr) + ", ";
            above_text += "}";
            below_text += "}";
            if (above_text != "{}")
                above.emplace(id, "DE: " + above_text);
            if (below_text != "{}")
                above[id] += "<BR/>KILLED: " + below_text;
        }
        function.print_cfg("lala2.png", above, below);
    }
    // endregion

    std::map<int, std::set<Expression>> out_sets;
    std::map<int, std::set<Expression>> in_sets;

    // OUT[ENTRY] = 0 (empty set);
    // for (each basic block B other than ENTRY) OUT[B] = U (universal set);
    auto entry_node = function.find_entry_block();
    out_sets[entry_node->id] = {};
    for (auto &b : blocks)
        if (b->id != entry_node->id)
            out_sets[b->id] = all_expressions;

    // while (changes to any OUT occur)
    bool changed = true;
    while (changed) {
        changed = false;

        // for (each basic block other than ENTRY)
        // iterate in reverse post order
        for (auto &[id, rpo] : id_rpo_pairs) {
            if (id == entry_node->id) // skip entry block
                continue;
            auto block = id_to_block.at(id);

            // IN[B] = && of predecessors' OUTs;
            std::vector<std::set<Expression>> pred_outs;
            for (auto &pred : block->predecessors)
                pred_outs.push_back(out_sets.at(pred->id));
            auto IN = intersection_of_sets(std::move(pred_outs));
            in_sets[id] = IN;

            // OUT[B] = e_gen U (IN[B] - e_kill);
            auto &e_kill = id_to_e_kill.at(id);
            for (auto &killed : e_kill)
                IN.erase(killed);

            auto &e_gen = id_to_e_gen.at(id);
            auto OUT = union_of_sets(std::vector{e_gen, IN});
            if (OUT != out_sets.at(id))
                changed = true;
            out_sets[id] = OUT;
        }
    }

    // region Available Expressions Print
    std::cout << "Available Expressions" << std::endl;
    for (auto &[id, out] : in_sets) {
        std::cout << id_to_block.at(id)->get_name() << "; IN: ";
        for (auto &expr : out)
            print_expression(expr, true);
        std::cout << std::endl;
        auto in = out_sets.at(id);
        std::cout << "\tOUT: ";
        for (auto &expr : in)
            print_expression(expr, true);
        std::cout << std::endl << std::endl;
    }
    // endregion
    // region Print CFG
    std::unordered_map<int, std::string> above, below;
    for (auto &[id, out] : in_sets) {
        std::string above_text = "{", below_text = "{";
        for (auto &expr : out)
            above_text += print_expression(expr) + ", ";
        for (auto &expr : out_sets.at(id))
            below_text += print_expression(expr) + ", ";
        above_text += "}";
        below_text += "}";
        if (above_text != "{}")
            above.emplace(id, above_text);
        if (below_text != "{}")
            below.emplace(id, below_text);
    }
    function.print_cfg("lala.png", above, below);
    // endregion
}

void anticipable_expressions(Function &function) {
    // backward data-flow analysis
    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_po = function.get_post_ordering();
    auto id_po_pairs = std::vector<std::pair<int, int>>(id_to_po.begin(), id_to_po.end());
    // sort by po
    std::sort(id_po_pairs.begin(), id_po_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });

    // Stage 1: Collect all expressions (universal set)
    using Expression = std::tuple<Operand, Quad::Type, Operand>;
    std::set<Expression> all_expressions;
    for (auto &b : blocks) {
        for (auto &q : b->quads) {
            if (q.is_binary()) {
                Expression expr = {*q.get_op(0), q.type, *q.get_op(1)};
                all_expressions.insert(expr);
            }
        }
    }

    auto print_expression = [](Expression expr, bool with_endl = true) {
        auto &[lhs, type, rhs] = expr;
        std::cout << (int)type << " [" << lhs.value << ", " << rhs.value << "]";
        if (with_endl)
            std::cout << std::endl;
        else
            std::cout << ", ";
    };

    // region Print expressions
    std::cout << "Expressions: " << std::endl;
    for (auto &expr : all_expressions)
        print_expression(expr);
    // endregion

    // Calculate e_gen and e_kill sets for each block
    std::map<int, std::set<Expression>> id_to_e_gen;
    std::map<int, std::set<Expression>> id_to_e_kill;
    for (auto &b : blocks) {
        std::set<Expression> e_gen; // upward exposed expressions (used before killed)
        std::set<Expression> e_kill;
        std::set<std::string> redefined;

        for (auto &q : b->quads) {
            // check generated expressions
            if (q.is_binary()) {
                auto lhs = *q.get_op(0);
                auto rhs = *q.get_op(1);
                Expression expr = {lhs, q.type, rhs};
                //                if (e_kill.find(expr) == e_kill.end()) {
                //                    e_gen.insert(expr);
                //                }
                if (redefined.find(lhs.value) == redefined.end() &&
                    redefined.find(rhs.value) == redefined.end()) {
                    e_gen.insert(expr);
                } else {
                    e_kill.insert(expr);
                }
            }

            // check killed expressions
            if (q.is_assignment()) {
                auto def = q.dest->name;
                redefined.insert(def);

                for (auto &expr : all_expressions /*e_gen*/) {
                    auto &[lhs, type, rhs] = expr;
                    if ((def == lhs.value ||
                         def == rhs.value) /*&& e_gen.find(expr) == e_gen.end()*/)
                        e_kill.insert(expr);
                }
            }
        }

        id_to_e_gen[b->id] = e_gen;
        id_to_e_kill[b->id] = e_kill;
    }

    // region Print e_gen and e_kill sets
    for (auto &[id, e_gen] : id_to_e_gen) {
        std::cout << id_to_block.at(id)->get_name() << "; Upward exposed: ";
        for (auto &expr : e_gen)
            print_expression(expr, false);
        std::cout << std::endl;
        auto e_kill = id_to_e_kill.at(id);
        std::cout << "\tKilled: ";
        for (auto &expr : e_kill)
            print_expression(expr, false);
        std::cout << std::endl << std::endl;
    }
    // endregion

    std::map<int, std::set<Expression>> in_sets;
    std::map<int, std::set<Expression>> out_sets;

    // OUT[ENTRY] = 0 (empty set);
    // for (each basic block B other than EXIT) OUT[B] = U (universal set);
    auto exit_node = function.find_exit_block();
    in_sets[exit_node->id] = {};
    for (auto &b : blocks)
        if (b->id != exit_node->id)
            in_sets[b->id] = all_expressions;

    // while (changes to any OUT occur)
    bool changed = true;
    while (changed) {
        changed = false;

        // for (each basic block other than EXIT)
        // iterate in post order
        for (auto &[id, rpo] : id_po_pairs) {
            if (id == exit_node->id) // skip exit block
                continue;
            auto block = id_to_block.at(id);

            // OUT[B] = && of successors' OUTs;
            std::vector<std::set<Expression>> pred_outs;
            for (auto &pred : block->successors)
                pred_outs.push_back(in_sets.at(pred->id));
            auto OUT = intersection_of_sets(std::move(pred_outs));
            out_sets[id] = OUT;

            // IN[B] = e_gen U (OUT[B] - e_kill);
            auto &e_kill = id_to_e_kill.at(id);
            for (auto &killed : e_kill)
                OUT.erase(killed);

            auto &e_gen = id_to_e_gen.at(id);
            auto IN = union_of_sets(std::vector{e_gen, OUT});
            if (IN != in_sets.at(id))
                changed = true;
            in_sets[id] = IN;
        }
    }

    // region Anticipable Expressions Print
    std::cout << "Anticipable Expressions" << std::endl;
    for (auto &[id, out] : out_sets) {
        std::cout << id_to_block.at(id)->get_name() << "; OUT: ";
        for (auto &expr : out)
            print_expression(expr, false);
        std::cout << std::endl;
        auto in = in_sets.at(id);
        std::cout << "\tIN: ";
        for (auto &expr : in)
            print_expression(expr, false);
        std::cout << std::endl << std::endl;
    }
    // endregion
}
