//
// Created by shoco on 2/12/2021.
//

#include "sparse_conditional_constant_propagation.hpp"

void sparse_conditional_constant_propagation(Function &f) {
    auto &blocks = f.basic_blocks;

    struct Place {
        int block_num;
        int quad_num;
    };

    auto print_place = [&](Place place) {
        return "(" + std::to_string(place.block_num) + "; " + std::to_string(place.quad_num) + ")";
    };

    struct Value {
        enum class Type { Bottom /*Not a constant*/, Constant, Top /*Undefined (yet)*/ };
        Type type = Type::Top;
        Operand constant;

        bool operator==(const Value &rhs) const { return type == rhs.type && constant == rhs.constant; }
        bool operator!=(const Value &rhs) const { return !(*this == rhs); }
    };

    struct Record {
        Place defined_at = {-1, -1};
        std::vector<Place> used_at{};
        Value value{};

        bool operator==(const Record &rhs) const { return value == rhs.value; }
        bool operator!=(const Record &rhs) const { return value != rhs.value; }
    };

    std::map<std::string, Record> usingInfo;
    // fill in info
    for (auto &b : blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];

            Place place{b->id, q_index};
            if (!q.is_jump() && q.dest.has_value())
                usingInfo[q.dest->name].defined_at = place;

            for (const auto &op_name : q.get_rhs(false)) {
                usingInfo[op_name].used_at.push_back(place);
            }
        }
    }

    //    for (auto &[name, value] : usingInfo) {
    //        std::cout << name << std::endl;
    //        std::cout << "Defined at: " << print_place(value.defined_at) << std::endl;
    //        std::cout << "Used at: " << print_into_string_with(value.used_at, print_place) <<
    //        std::endl; std::cout << std::endl;
    //    }

    using CFGEdge = std::pair<int, int>;
    using SSAEdge = std::pair<Place, Place>;
    std::set<CFGEdge> executed_edges;

    std::vector<CFGEdge> CFGWorkList;
    std::vector<SSAEdge> SSAWorkList;

    auto entry = f.find_entry_block();
    for (auto &s : entry->successors)
        CFGWorkList.emplace_back(entry->id, s->id);

    auto EvaluateOverLattice = [&](Quad &q) -> Value {
        std::vector<Value> values;
        for (auto &op : q.ops) {
            if (op.is_var()) {
                auto useInfo = usingInfo.at(op.get_string());
                values.push_back(useInfo.value);
            } else
                values.push_back(Value{.type = Value::Type::Constant, .constant = op});
        }

        if (std::any_of(values.begin(), values.end(),
                        [](auto &v) { return v.type == Value::Type::Bottom; }))
            return Value{.type = Value::Type::Bottom};
        else if (std::all_of(values.begin(), values.end(),
                             [](auto &v) { return v.type == Value::Type::Top; }))
            return Value{.type = Value::Type::Top};

        std::vector<Operand> constants;
        for (auto &v : values)
            if (v.type == Value::Type::Constant)
                constants.push_back(v.constant);

        if (constants.size() == 1 ||
            (constants.size() >= 2 &&
             std::equal(constants.begin() + 1, constants.end(), constants.begin()))) {
            return Value{.type = Value::Type::Constant, .constant = constants[0]};
        } else {
            return Value{.type = Value::Type::Bottom};
        }
    };

    auto EvaluateAssign = [&](Place place) {
        auto [block_num, quad_num] = place;
        auto &b = f.id_to_block.at(block_num);
        auto &q = b->quads[quad_num];

        auto &dest_use_info = usingInfo.at(q.dest->name);
        for (auto &used : q.get_rhs(false)) {
            usingInfo[used].value = dest_use_info.value;
        }

        if (dest_use_info.value.type != Value::Type::Bottom) {
            // Evaluate over lattice
            Value v = EvaluateOverLattice(q);
            if (v != dest_use_info.value) {
                dest_use_info.value = v;
                for (auto &used_at : dest_use_info.used_at)
                    SSAWorkList.emplace_back(dest_use_info.defined_at, used_at);
            }
        }
    };

    auto EvaluateConditional = [&](BasicBlock *b) {
        auto &cond = b->quads.back();
        auto cond_var = *cond.get_op(0);
        auto value = cond_var.is_var() ? usingInfo.at(cond_var.get_string()).value
                                       : Value{.type = Value::Type::Constant, .constant = cond_var};

        // ?????????? ??? if Value(d)  ̸= Value(s) then
        // if (value.type != Value::Type::Bottom) {}
        if (value.type == Value::Type::Constant) {
            if (value.constant.is_true())
                CFGWorkList.emplace_back(b->id, b->get_jumped_to_successor()->id);
            else
                CFGWorkList.emplace_back(b->id, b->get_fallthrough_successor()->id);
        } else {
            for (auto &s : b->successors) {
                CFGWorkList.emplace_back(b->id, s->id);
            }
        }
    };

    auto EvaluateOperands = [&](Quad &phi) {
        std::string x = phi.dest->name;
        if (usingInfo.at(x).value.type != Value::Type::Bottom) {
            for (auto &op : phi.get_rhs(false)) {
                // ????
            }
        }
    };

    auto EvaluateResult = [&](Quad &phi) {
        auto &x_using_info = usingInfo.at(phi.dest->name);
        if (x_using_info.value.type != Value::Type::Bottom) {
            Value v = EvaluateOverLattice(phi);
            if (x_using_info.value != v) {
                x_using_info.value = v;
                for (auto &used_at : x_using_info.used_at) {
                    SSAWorkList.emplace_back(x_using_info.defined_at, used_at);
                }
            }
        }
    };

    auto EvaluateAllPhisInBlock = [&](CFGEdge edge) {
        auto [m, n] = edge;
        auto &b = f.id_to_block.at(n);

        for (int i = 0; i < b->phi_functions; i++)
            EvaluateOperands(b->quads[i]);
        for (int i = 0; i < b->phi_functions; i++)
            EvaluateResult(b->quads[i]);
    };

    auto EvaluatePhi = [&](SSAEdge edge) {
        auto [s, d] = edge;
        auto &phi = f.id_to_block.at(d.block_num)->quads[d.quad_num];
        EvaluateOperands(phi);
        EvaluateResult(phi);
    };

    while (!CFGWorkList.empty() || !SSAWorkList.empty()) {
        if (!CFGWorkList.empty()) {
            auto edge = CFGWorkList.back();
            auto [m, n] = edge;
            CFGWorkList.pop_back();

            if (executed_edges.count(edge) == 0) {
                executed_edges.insert(edge);

                EvaluateAllPhisInBlock(edge);

                bool no_other_entering_edge_is_executed = true;
                for (auto &pred : f.id_to_block.at(n)->predecessors)
                    if (executed_edges.count({pred->id, n}) > 0 && pred->id != m)
                        no_other_entering_edge_is_executed = false;
                if (no_other_entering_edge_is_executed) {
                    // Evaluate assignments
                    auto &b = f.id_to_block.at(n);
                    for (int q_index = b->phi_functions; q_index < b->quads.size(); q_index++) {
                        if (!b->quads[q_index].is_assignment()) {
                            EvaluateAssign(Place{b->id, q_index});
                        }
                    }

                    if (!b->quads.empty() && b->quads.back().is_conditional_jump()) {
                        EvaluateConditional(b);
                    } else if (!b->successors.empty()) {
                        CFGWorkList.emplace_back(b->id, (*b->successors.begin())->id);
                    }
                }
            }
        }

        if (!SSAWorkList.empty()) {
            auto edge = SSAWorkList.back();
            auto [s, d] = edge;
            SSAWorkList.pop_back();

            auto &c = f.id_to_block.at(d.block_num);
            bool any_entering_edge_is_executed = false;
            for (auto &pred : c->predecessors)
                if (executed_edges.count({pred->id, c->id}) > 0)
                    any_entering_edge_is_executed = true;
            if (any_entering_edge_is_executed) {
                if (c->quads[d.quad_num].type == Quad::Type::PhiNode)
                    EvaluatePhi(edge);
                else if (c->quads[d.quad_num].is_assignment())
                    EvaluateAssign(Place{c->id, d.quad_num});
                else
                    EvaluateConditional(c);
            }
        }
    }
}
