//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_SPARSE_SIMPLE_CONSTANT_PROPAGATION_HPP
#define TAC_PARSER_SPARSE_SIMPLE_CONSTANT_PROPAGATION_HPP

#include "../data_flow_analyses/data_flow_analyses.hpp"
#include "value_numbering.hpp"

struct SparseSimpleConstantPropagationDriver {
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

    struct IntermediateResults {
        std::map<std::string, UseDefInfo> use_def_graph;
        std::map<std::string, Value> values;
    } ir;

    Function f;
    SparseSimpleConstantPropagationDriver(Function &f_) : f(f_) {}

    void run() {
        fill_use_def_graph();
        std::vector<std::string> work_list = initialize();
        propagate(work_list);
        rewrite_program();
    }

    void print_values() {
        for (auto &[name, use_def_info] : ir.use_def_graph) {
            fmt::print("Var: {} defined at: {}; used at: ", name, use_def_info.defined_at);
            for (auto &u : use_def_info.used_at)
                fmt::print("{}", u);

            if (ir.values.count(name)) {
                auto v = ir.values.at(name);
                fmt::print(" ValType: {}; {}\n", (int)v.type, v.constant.value);
            }
        }
    }

    Value evaluate_over_lattice(Quad &q) {
        // it is either PHI or instruction with 2 (or 1?) operands
        std::vector<Value> quad_values;
        for (auto &op : q.ops) {
            if (op.is_var())
                quad_values.push_back(ir.values.at(op.get_string()));
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
            return (constants_equal) ? Value{.type = Value::Type::Constant, .constant = constants[0]}
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
    }

    void fill_use_def_graph() {
        for (auto &b : f.basic_blocks) {
            for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
                auto &q = b->quads[q_index];

                Location location{b->id, q_index};
                if (!q.is_jump() && q.dest.has_value())
                    ir.use_def_graph[q.dest->name].defined_at = location;

                for (auto &op_name : q.get_rhs(false))
                    ir.use_def_graph[op_name].used_at.push_back(location);
            }
        }
    }

    std::vector<std::string> initialize() {
        std::vector<std::string> work_list;
        for (auto &[name, use_def_info] : ir.use_def_graph) {
            auto [block_id, quad_id] = use_def_info.defined_at;
            if (block_id == -1)
                continue;

            auto &q = f.id_to_block.at(block_id)->quads.at(quad_id);

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
            ir.values[name] = value;
        }
        return work_list;
    }

    void propagate(std::vector<std::string> &work_list) {
        while (!work_list.empty()) {
            std::string name = work_list.back();
            work_list.pop_back();

            for (auto &[block_id, quad_num] : ir.use_def_graph.at(name).used_at) {
                auto &q = f.id_to_block.at(block_id)->quads.at(quad_num);
                if (q.is_jump() || !q.dest.has_value())
                    continue;

                std::string m = q.dest->name;
                if (ir.values.at(m).type != Value::Type::Bottom) {
                    auto tmp = ir.values.at(m);
                    ir.values[m] = evaluate_over_lattice(q);
                    if (ir.values.at(m) != tmp)
                        work_list.push_back(m);
                }
            }
        }
    }

    void rewrite_program() {
        for (auto &[name, use_def_info] : ir.use_def_graph) {
            auto [b_id, quad] = use_def_info.defined_at;
            auto &def_q = f.id_to_block.at(b_id)->quads.at(quad);

            if (ir.values.count(name) > 0 && ir.values.at(name).type == Value::Type::Constant) {
                auto &constant = ir.values.at(name).constant;

                def_q.type = Quad::Type::Assign;
                def_q.ops[0] = constant;
                def_q.clear_op(1);

                for (auto &[block_id, quad_num] : use_def_info.used_at) {
                    auto &use_q = f.id_to_block.at(block_id)->quads.at(quad_num);
                    for (auto &op : use_q.ops)
                        if (op.value == name)
                            op = Operand(constant.value, constant.type, op.phi_predecessor);
                }
            }
        }

        for (auto &b : f.basic_blocks)
            b->update_phi_positions();
    }
};

static void run_sparse_simple_constant_propagation(Function &f) {
    SparseSimpleConstantPropagationDriver simple_constant_propagation_driver(f);
    simple_constant_propagation_driver.run();
    f = simple_constant_propagation_driver.f;
}

#endif // TAC_PARSER_SPARSE_SIMPLE_CONSTANT_PROPAGATION_HPP
