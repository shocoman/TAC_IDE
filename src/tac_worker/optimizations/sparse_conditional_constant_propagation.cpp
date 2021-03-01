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
    auto entry = f.find_entry_block();
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

    // Rewrite Program
    for (auto &[name, use_def_info] : use_def_graph) {
        auto [b_id, quad] = use_def_info.defined_at;
        auto &def_q = f.id_to_block.at(b_id)->quads.at(quad);

        auto def_key = std::pair{name, use_def_info.defined_at};
        if (values.count(def_key) > 0 && values.at(def_key).type == Value::Type::Constant) {

            def_q.type = Quad::Type::Assign;
            def_q.ops[0] = values.at(def_key).constant;
            def_q.clear_op(1);

            for (auto &[block_id, quad_num] : use_def_info.used_at) {
                auto &constant = values.at(def_key).constant;

                auto &use_q = f.id_to_block.at(block_id)->quads.at(quad_num);
                for (auto &op : use_q.ops)
                    if (op.value == name)
                        op = Operand(constant.value, constant.type, op.phi_predecessor);
            }
        }
    }

    for (auto &b : blocks)
        b->update_phi_positions();
}
