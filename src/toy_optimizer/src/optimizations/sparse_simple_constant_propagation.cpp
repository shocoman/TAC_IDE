//
// Created by shoco on 1/22/2021.
//

#include "sparse_simple_constant_propagation.hpp"

void sparse_simple_constant_propagation(Function &function) {
    auto &blocks = function.basic_blocks;

    using Location = std::pair<int, int>;

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
        Location defined_at = {-1, -1};
        std::vector<Location> used_at{};
    };

    std::map<std::string, UseDefInfo> use_def_graph;
    std::map<std::string, Value> values;

    auto EvaluateOverLattice = [&](Quad &q) -> Value {
        // it is either PHI or instruction with 2 (or 1?) operands

        std::vector<Value> quad_values;
        for (auto &op : q.ops) {
            if (op.is_var())
                quad_values.push_back(values.at(op.get_string()));
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
            return (constants_equal)
                       ? Value{.type = Value::Type::Constant, .constant = constants[0]}
                       : Value{.type = Value::Type::Bottom};
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

    // fill use def graph
    for (auto &b : blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];

            Location location{b->id, q_index};
            if (!q.is_jump() && q.dest.has_value())
                use_def_graph[q.dest->name].defined_at = location;

            for (auto &op_name : q.get_rhs(false))
                use_def_graph[op_name].used_at.push_back(location);
        }
    }

    // Initialization Phase
    std::vector<std::string> work_list;
    for (auto &[name, use_def_info] : use_def_graph) {
        auto [block_id, quad_id] = use_def_info.defined_at;
        if (block_id == -1)
            continue;

        auto &q = function.id_to_block.at(block_id)->quads.at(quad_id);

        if (Quad::is_foldable(q.type))
            constant_folding(q);

        Value value;
        if (q.type == Quad::Type::Assign && q.get_op(0)->is_constant())
            value = {.type = Value::Type::Constant, .constant = q.get_op(0).value()};
        else if (q.type == Quad::Type::Getparam || q.type == Quad::Type::Call) // cannot be known
            value.type = Value::Type::Bottom;
        else // also phi nodes
            value.type = Value::Type::Top;

        if (value.type == Value::Type::Constant)
            work_list.push_back(name);
        values[name] = value;
    }

    // Propagation Phase
    while (!work_list.empty()) {
        std::string name = work_list.back();
        work_list.pop_back();

        for (auto &[block_id, quad_num] : use_def_graph.at(name).used_at) {
            auto &q = function.id_to_block.at(block_id)->quads.at(quad_num);
            if (q.is_jump() || !q.dest.has_value())
                continue;

            std::string m = q.dest->name;
            if (values.at(m).type != Value::Type::Bottom) {
                auto tmp = values.at(m);
                values[m] = EvaluateOverLattice(q);
                if (values.at(m) != tmp)
                    work_list.push_back(m);
            }
        }
    }

    // region Print SSCP
//    for (auto &[name, use_def_info] : use_def_graph) {
//        fmt::print("Var: {} defined at: {}; used at: ", name, use_def_info.defined_at);
//        for (auto &u : use_def_info.used_at)
//            fmt::print("{}", u);
//
//        if (values.count(name)) {
//            auto v = values.at(name);
//            fmt::print(" ValType: {}; {}\n", (int)v.type, v.constant.value);
//        }
//    }
    // endregion

    // Rewrite Program
    for (auto &[name, use_def_info] : use_def_graph) {
        auto [b_id, quad] = use_def_info.defined_at;
        auto &def_q = function.id_to_block.at(b_id)->quads.at(quad);

        if (values.count(name) > 0 && values.at(name).type == Value::Type::Constant) {
            auto &constant = values.at(name).constant;

            def_q.type = Quad::Type::Assign;
            def_q.ops[0] = constant;
            def_q.clear_op(1);

            for (auto &[block_id, quad_num] : use_def_info.used_at) {
                auto &use_q = function.id_to_block.at(block_id)->quads.at(quad_num);
                for (auto &op : use_q.ops)
                    if (op.value == name)
                        op = Operand(constant.value, constant.type, op.phi_predecessor);
            }
        }
    }

    for (auto &b : blocks)
        b->update_phi_positions();
}