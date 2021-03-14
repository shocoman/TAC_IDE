//
// Created by shoco on 2/12/2021.
//

#include "sparse_conditional_constant_propagation.hpp"

void sparse_conditional_constant_propagation(Function &f) {
    auto &blocks = f.basic_blocks;

    using Place = std::pair<int, int>;
    using Position = std::pair<std::string, Place>;

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

    std::map<std::string, UseDefInfo> use_def_graph;
    std::map<Position, Value> values;

    // fill in info (def-use graph)
    for (auto &b : blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];

            Place place{b->id, q_index};
            if (!q.is_jump() && q.dest.has_value()) {
                use_def_graph[q.dest->name].defined_at = place;

                values[{q.dest->name, place}] =
                    (q.type == Quad::Type::Getparam || q.type == Quad::Type::Call)
                        ? Value{.type = Value::Type::Bottom}
                        : Value{.type = Value::Type::Top};
            }

            for (const auto &op_name : q.get_rhs(false)) {
                use_def_graph[op_name].used_at.push_back(place);
                values[{op_name, place}] = Value{.type = Value::Type::Top};
            }
        }
    }

    for (auto &[name, use_def_info] : use_def_graph) {
        fmt::print("{}: {}; {}\n", name, use_def_info.defined_at, use_def_info.used_at);
    }
    fmt::print("Done\n");

    using CFGEdge = std::pair<int, int>;
    using SSAEdge = std::pair<Place, Place>;
    std::set<CFGEdge> executed_edges;

    std::vector<CFGEdge> CFGWorkList;
    std::vector<SSAEdge> SSAWorkList;

    // init cfg worklist with edges leaving entry node
    auto entry = f.get_entry_block();
    for (auto &s : entry->successors)
        CFGWorkList.emplace_back(entry->id, s->id);

    auto EvaluateOverLattice = [&](Place place, Quad &q) -> Value {
        // it is either PHI or instruction with 2 (or 1?) operands

        std::vector<Value> quad_values;
        for (auto &op : q.ops) {
            if (op.is_var())
                quad_values.push_back(values.at({op.get_string(), place}));
            else
                quad_values.push_back(Value{.type = Value::Type::Constant, .constant = op});
        }

        auto is_bottom = [](auto &v) { return v.type == Value::Type::Bottom; };
        auto is_top = [](auto &v) { return v.type == Value::Type::Top; };
        if (std::any_of(quad_values.begin(), quad_values.end(), is_bottom))
            return Value{.type = Value::Type::Bottom};
        else if (std::any_of(quad_values.begin(), quad_values.end(), is_top) &&
                 q.type != Quad::Type::PhiNode)
            return Value{.type = Value::Type::Top};

        std::vector<Operand> constants;
        for (auto &v : quad_values)
            if (v.type == Value::Type::Constant)
                constants.push_back(v.constant);

        if (q.type == Quad::Type::PhiNode) {
            bool constants_equal = std::equal(constants.begin() + 1, constants.end(), constants.begin());
            if (constants_equal)
                return Value{.type = Value::Type::Constant, .constant = constants[0]};
            else
                return Value{.type = Value::Type::Bottom};
        } else if (constants.size() != quad_values.size()) {
            return Value{.type = Value::Type::Bottom};
        } else if (constants.size() == 1) {
            return Value{.type = Value::Type::Constant, .constant = constants[0]};
        } else {
            auto folded_quad = Quad(constants[0], constants[1], q.type);
            constant_folding(folded_quad);
            assert(folded_quad.type == Quad::Type::Assign);
            return Value{.type = Value::Type::Constant, .constant = folded_quad.ops[0]};
        }
    };

    auto EvaluateAssign = [&](Place place) {
        auto [block_num, quad_num] = place;
        auto &b = f.id_to_block.at(block_num);
        auto &q = b->quads[quad_num];

        // for each value y used by the expression in m
        //   let (x,y) be the SSA edge that supplies y (x - definition place, y - use place)
        //   Value(y) ← Value(x)
        for (auto &y : q.get_rhs(false)) {
            values[{y, place}] = values.at({y, use_def_graph.at(y).defined_at});
        }

        auto &dest_value = values.at({q.dest->name, place});
        if (dest_value.type != Value::Type::Bottom) {
            Value v = EvaluateOverLattice(place, q);
            if (dest_value != v) {
                dest_value = v;
                for (auto &used_at : use_def_graph.at(q.dest->name).used_at)
                    SSAWorkList.emplace_back(place, used_at);
            }
        }
    };

    auto EvaluateConditional = [&](BasicBlock *b) {
        Place place = {b->id, b->quads.size() - 1};
        auto &cond_quad = b->quads.back();
        auto cond_var = *cond_quad.get_op(0);
        auto cond_var_name = cond_var.get_string();
        auto constant = Value{.type = Value::Type::Constant, .constant = cond_var};
        auto &value_at_use = cond_var.is_var() ? values.at({cond_var_name, place}) : constant;

        if (value_at_use.type != Value::Type::Bottom) {
            auto value_at_def =
                cond_var.is_constant()
                    ? value_at_use
                    : values.at({cond_var_name, use_def_graph.at(cond_var_name).defined_at});
            if (cond_var.is_constant() || value_at_use != value_at_def) {
                value_at_use = value_at_def;

                if (value_at_use.type == Value::Type::Bottom) {
                    for (auto &s : b->successors)
                        CFGWorkList.emplace_back(b->id, s->id);
                } else {
                    bool make_jump =
                        value_at_use.constant.is_true() && cond_quad.type == Quad::Type::IfTrue ||
                        !value_at_use.constant.is_true() && cond_quad.type == Quad::Type::IfFalse;
                    if (make_jump)
                        CFGWorkList.emplace_back(b->id, b->get_jumped_to_successor()->id);
                    else
                        CFGWorkList.emplace_back(b->id, b->get_fallthrough_successor()->id);
                }
            }
        }
    };

    auto EvaluatePhiOperands = [&](Place place, Quad &phi) {
        std::string x = phi.dest->name;

        if (values.at({x, place}).type != Value::Type::Bottom) {
            for (auto &op : phi.ops) {
                auto op_name = op.get_string();
                CFGEdge cfg_edge = {op.phi_predecessor->id, place.first};
                auto &value_at_def = values.at({op_name, use_def_graph.at(op_name).defined_at});
                auto &value_at_use = values.at({op_name, place});
                if (executed_edges.count(cfg_edge) > 0)
                    value_at_use = value_at_def;
            }
        }
    };

    auto EvaluatePhiResult = [&](Place place, Quad &phi) {
        std::string x = phi.dest->name;
        auto &x_use_info = use_def_graph.at(x);
        auto &x_value = values.at({x, place});
        if (x_value.type != Value::Type::Bottom) {
            Value v = EvaluateOverLattice(place, phi);
            if (x_value != v) {
                x_value = v;
                for (auto &used_at : x_use_info.used_at)
                    SSAWorkList.emplace_back(place, used_at);
            }
        }
    };

    auto EvaluateAllPhisInBlock = [&](CFGEdge edge) {
        auto [m, n] = edge;
        auto &b = f.id_to_block.at(n);

        for (int i = 0; i < b->phi_functions; i++)
            EvaluatePhiOperands(Place{n, i}, b->quads[i]);
        for (int i = 0; i < b->phi_functions; i++)
            EvaluatePhiResult(Place{n, i}, b->quads[i]);
    };

    auto EvaluatePhi = [&](SSAEdge edge) {
        auto [s, d] = edge;
        auto &phi = f.id_to_block.at(d.first)->quads[d.second];
        EvaluatePhiOperands(d, phi);
        EvaluatePhiResult(d, phi);
    };

    while (!CFGWorkList.empty() || !SSAWorkList.empty()) {
        if (!CFGWorkList.empty()) {
            auto edge = CFGWorkList.back();
            auto [m, n] = edge;
            CFGWorkList.pop_back();

            // if edge wasn't executed before
            if (executed_edges.insert(edge).second) {
                EvaluateAllPhisInBlock(edge);

                bool no_other_entering_edge_is_executed = true;
                for (auto &pred : f.id_to_block.at(n)->predecessors)
                    if (executed_edges.count({pred->id, n}) > 0 && pred->id != m)
                        no_other_entering_edge_is_executed = false;

                if (no_other_entering_edge_is_executed) {
                    auto &b = f.id_to_block.at(n);

                    for (int q_index = b->phi_functions; q_index < b->quads.size(); q_index++) {
                        auto &q = b->quads[q_index];
                        if (q.is_assignment())
                            EvaluateAssign(Place{b->id, q_index});
                    }

                    if (!b->quads.empty() && b->quads.back().is_conditional_jump())
                        EvaluateConditional(b);
                    else if (!b->successors.empty())
                        CFGWorkList.emplace_back(b->id, (*b->successors.begin())->id);
                }
            }
        }

        if (!SSAWorkList.empty()) {
            auto edge = SSAWorkList.back();
            auto [s, d] = edge;
            SSAWorkList.pop_back();

            auto &c = f.id_to_block.at(d.first);
            bool any_entering_edge_is_executed = false;
            for (auto &pred : c->predecessors)
                if (executed_edges.count({pred->id, c->id}) > 0)
                    any_entering_edge_is_executed = true;
            if (any_entering_edge_is_executed) {
                auto place = Place{c->id, d.second};
                auto &q = c->quads[d.second];
                if (q.type == Quad::Type::PhiNode)
                    EvaluatePhi(edge);
                else if (q.is_assignment())
                    EvaluateAssign(place);
                else
                    EvaluateConditional(c);
            }
        }
    }

    // Print executed edges
    for (auto &b : blocks) {
        for (auto &s : b->successors) {
            std::pair edge = {b->id, s->id};
            bool is_executed = executed_edges.count(edge) > 0;

            fmt::print("Edge ({} -> {}): ", b->get_name(), s->get_name());
            fmt::print("{}\n", is_executed ? "executed" : "not executed");
        }
    }

    fmt::print("Values\n");
    for (auto &[key, value] : values) {
        auto &[name, place] = key;
        auto &[block_id, quad_num] = place;
        auto &block = f.id_to_block.at(block_id);

        fmt::print("{} at ({}, '{}'): ", name, block->get_name(), block->quads[quad_num].fmt());

        if (value.type == Value::Type::Bottom)
            fmt::print("Bottom\n");
        else if (value.type == Value::Type::Top)
            fmt::print("Top\n");
        else if (value.type == Value::Type::Constant)
            fmt::print("Constants: {}\n", value.constant.get_string());
    }

    std::set<Place> changed_places;
    // Rewrite Program
    for (auto &[name, use_def_info] : use_def_graph) {
        auto [b_id, quad] = use_def_info.defined_at;
        auto &def_q = f.id_to_block.at(b_id)->quads.at(quad);

        auto def_key = std::pair{name, use_def_info.defined_at};
        if (values.count(def_key) > 0 && values.at(def_key).type == Value::Type::Constant) {
            auto &constant = values.at(def_key).constant;
            if (def_q.ops[0].value != constant.value)
                changed_places.insert(use_def_info.defined_at);

            def_q.type = Quad::Type::Assign;
            def_q.ops[0] = constant;
            def_q.ops.erase(def_q.ops.begin() + 1, def_q.ops.end());

            for (auto &[block_id, quad_num] : use_def_info.used_at) {
                auto &use_q = f.id_to_block.at(block_id)->quads.at(quad_num);
                for (auto &op : use_q.ops)
                    // ignore phi's ops
                    if (op.value == name && use_q.type != Quad::Type::PhiNode) {
                        if (op.value != constant.value)
                            changed_places.emplace(block_id, quad_num);
                        op = Operand(constant.value, constant.type, op.phi_predecessor);
                    }
            }
        }
    }

    for (auto &b : blocks)
        b->update_phi_positions();

    std::unordered_set<int> useless_blocks;
    for (auto &b : f.basic_blocks)
        useless_blocks.insert(b->id);
    for (auto &n : f.basic_blocks)
        for (auto &s : n->successors)
            if (executed_edges.count({n->id, s->id})) {
                useless_blocks.erase(n->id);
                useless_blocks.erase(s->id);
            }

    print_sccp_result_graph(f, executed_edges, useless_blocks, changed_places);

    remove_useless_blocks(f, useless_blocks);
}

void print_sccp_result_graph(Function &f, std::set<std::pair<int, int>> &executed_edges,
                             std::unordered_set<int> &useless_blocks,
                             std::set<std::pair<int, int>> &changed_places) {
    GraphWriter dot_writer;
    dot_writer.legend_marks = {{"Executed edge", "red", "solid"}, {"Ignored edge", "black", "dashed"}};

    for (const auto &n : f.basic_blocks) {
        auto node_name = n->get_name();

        dot_writer.set_attribute(node_name, "subscript", fmt::format("id={}", n->id));
        if (useless_blocks.count(n->id))
            dot_writer.set_attribute(node_name, "style", "dashed");

        // print all quads as text
        std::vector<std::string> quad_lines;
        for (int i = 0; i < n->quads.size(); ++i) {
            std::string s = escape_string(n->quads[i].fmt());
            if (changed_places.count({n->id, i}))
                s = fmt::format("<B>{}</B>", s);

            quad_lines.emplace_back(s);
        }
        dot_writer.set_node_text(node_name, quad_lines);

        for (auto &s : n->successors) {
            std::unordered_map<std::string, std::string> attributes = {
                {"label", s->lbl_name.value_or("")},
            };
            if (executed_edges.count({n->id, s->id}))
                attributes["color"] = "red";
            else
                attributes["style"] = "dashed";

            dot_writer.add_edge(node_name, s->get_name(), attributes);
        }
    }

    std::string filename = "graphs/sccp_result.png";
    dot_writer.set_title("SCCP result");
    dot_writer.render_to_file(filename);
    system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
}

void remove_useless_blocks(Function &f, std::unordered_set<int> &useless_blocks) {
    for (auto b_id : useless_blocks) {
        auto &block = f.id_to_block.at(b_id);

        // rewrite if's of predecessors
        for (auto &pred : block->predecessors) {
            if (pred->quads.empty())
                continue;
            auto &q = pred->quads.back();
            if (q.is_conditional_jump()) {
                pred->successors.erase(block);

                BasicBlock *left_jump_target = *pred->successors.begin();
                if (auto &jump_target = left_jump_target->lbl_name; jump_target.has_value()) {
                    q = Quad({}, {}, Quad::Type::Goto);
                    q.dest = Dest(jump_target.value(), {}, Dest::Type::JumpLabel);
                } else
                    pred->quads.pop_back();
            }
        }

        // rewrite phi's of successors
        for (auto &succ : block->successors) {
            for (int i = 0; i < succ->phi_functions; ++i) {
                auto &phi = succ->quads[i];
                for (int j = phi.ops.size() - 1; j >= 0; --j)
                    if (phi.ops[j].phi_predecessor->id == b_id)
                        phi.clear_op(j);

                // if phi has only one OP or all ops are equal
                bool ops_equal = std::equal(phi.ops.begin() + 1, phi.ops.end(), phi.ops.begin());
                if (ops_equal) {
                    phi.ops.erase(phi.ops.begin() + 1, phi.ops.end());
                    phi.type = Quad::Type::Assign;
                }
            }

            succ->update_phi_positions();
        }

        // remove block
        block->remove_predecessors();
        block->remove_successors();
        f.basic_blocks.erase(std::find_if(f.basic_blocks.begin(), f.basic_blocks.end(),
                                          [b_id](auto &block) { return block->id == b_id; }));
    }
}