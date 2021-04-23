//
// Created by shoco on 10/15/2020.
//

#ifndef TAC_PARSER_LOCAL_VALUE_NUMBERING_H
#define TAC_PARSER_LOCAL_VALUE_NUMBERING_H

#include <fmt/ranges.h>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../data_flow_analyses/dominators.hpp"
#include "../structure/function.hpp"

void constant_folding(Quad &q);

struct ValueNumberTableStack {
    using OpRecord = std::tuple<Quad::Type, std::vector<int>>;
    struct ValueNumberTable {
        std::map<std::string, int> value_numbers;
        std::map<int, std::string> value_number_to_name;
        std::map<OpRecord, int> operations;
    };

    std::vector<ValueNumberTable> tables;
    int current_number = 0;

    void set_value_number_for_name(std::string name, int value) { tables.back().value_numbers[name] = value; }

    void set_name_for_value(int value, std::string name) { tables.back().value_number_to_name[value] = name; }

    void set_operation_value(OpRecord op, int value) { tables.back().operations[op] = value; }

    std::optional<int> get_value_number_by_name(const std::string &name) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            if (auto v = it->value_numbers.find(name); v != it->value_numbers.end()) {
                return v->second;
            }
        }
        return {};
    }

    std::optional<std::string> get_name_by_value_number(int value) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            if (auto n = it->value_number_to_name.find(value); n != it->value_number_to_name.end()) {
                return n->second;
            }
        }
        return {};
    }

    std::optional<int> get_value_number_by_operation(const OpRecord &op) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            if (auto v = it->operations.find(op); v != it->operations.end()) {
                return v->second;
            }
        }
        return {};
    }

    void push_table() { tables.emplace_back(); }

    void pop_table() { tables.pop_back(); }
};

void local_value_numbering(std::vector<Quad> &quads, ValueNumberTableStack &t);
void superlocal_value_numbering(Function &function);

////////////////////////////////////////////////////////////////////////
/////////////// DOMINATOR BASED VALUE NUMBERING ////////////////////////
////////////////////////////////////////////////////////////////////////

struct GlobalValueNumberingDriver {
    struct ValueNumberTableStack {
        using OpRecord = std::tuple<Quad::Type, std::vector<std::string>>;
        struct ValueNumberTable {
            std::unordered_map<std::string, std::string> name_to_value_number;
            std::unordered_map<std::string, std::string> value_number_to_name;
            std::map<OpRecord, std::string> operations;
            std::map<std::set<Operand>, std::string> phi_nodes;
        };

        std::vector<ValueNumberTable> tables;
        std::map<std::set<Operand>, std::string> phi_nodes;

        void set_value_number_for_name(std::string name, std::string value) {
            tables.back().name_to_value_number[name] = value;
        }

        void set_operation_for_value_number(OpRecord op, std::string value) { tables.back().operations[op] = value; }

        std::optional<std::string> get_value_number_by_name(const std::string &name) {
            for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
                auto v = it->name_to_value_number.find(name);
                if (v != it->name_to_value_number.end())
                    return v->second;
            }
            return {};
        }

        std::optional<std::string> get_value_number_by_operation(const OpRecord &op) {
            for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
                auto v = it->operations.find(op);
                if (v != it->operations.end())
                    return v->second;
            }
            return {};
        }

        void set_phi_node_for_value(std::set<Operand> &ops, const std::string &name) { phi_nodes[ops] = name; }

        std::optional<std::string> get_phi_node_by_operation(std::set<Operand> &ops) {
            auto v = phi_nodes.find(ops);
            return v != phi_nodes.end() ? std::make_optional(v->second) : std::nullopt;
        }

        void push_table() { tables.emplace_back(); }

        void pop_table() { tables.pop_back(); }
    };

    struct IntermediateResults {
        std::vector<std::tuple<std::string, std::string>> removed_quads;
        ID2IDOM id_to_idom;
    } ir;

    Function f;
    GlobalValueNumberingDriver(Function &f_) : f(f_) {}

    void run() {
        ir.id_to_idom = get_immediate_dominators(f);

        auto entry = f.get_entry_block();
        ValueNumberTableStack t;
        dvnt(entry, t);
    }

    void dvnt(BasicBlock *b, ValueNumberTableStack &t) {
        t.push_table();

        // process every phi function
        for (int i = 0; i < b->phi_functions; ++i) {
            auto &phi = b->quads.at(i);
            std::set<Operand> operands(phi.ops.begin(), phi.ops.end());
            // phi is meaningless (all operands are equal)
            if (std::equal(phi.ops.begin() + 1, phi.ops.end(), phi.ops.begin())) {
                ir.removed_quads.emplace_back(b->get_name(), phi.fmt());

                t.set_value_number_for_name(phi.dest->name, phi.ops[0].value);
                b->quads.erase(b->quads.begin() + i);
                --b->phi_functions;
                --i;
            }
            // or phi is redundant (same as one of the previous phi functions)
            else if (auto v = t.get_phi_node_by_operation(operands); v.has_value()) {
                ir.removed_quads.emplace_back(b->get_name(), phi.fmt());

                t.set_value_number_for_name(phi.dest->name, v.value());
                b->quads.erase(b->quads.begin() + i);
                --b->phi_functions;
                --i;
            } else {
                t.set_value_number_for_name(phi.dest->name, phi.dest->name);
                t.set_phi_node_for_value(operands, phi.dest->name);
            }
        }

        // work through each assignment of the form 'x = y op z'
        for (int quad_i = b->phi_functions; quad_i < b->quads.size(); ++quad_i) {
            auto &q = b->quads.at(quad_i);
            if (!q.is_assignment())
                continue;

            // overwrite 'x' and 'y' with saved value number
            for (auto &op : q.ops) {
                if (auto v = t.get_value_number_by_name(op.value); v.has_value())
                    op = Operand(v.value());
            }

            if (Quad::is_foldable(q.type))
                constant_folding(q);

            std::vector<std::string> expr;
            for (auto &op : q.ops)
                expr.push_back(op.value);
            if (Quad::is_commutative(q.type))
                std::sort(expr.begin(), expr.end());
            auto op_hash_key = std::tuple{q.type, expr};

            auto v = t.get_value_number_by_operation(op_hash_key);
            if (v.has_value()) {
                ir.removed_quads.emplace_back(b->get_name(), q.fmt());

                t.set_value_number_for_name(q.dest->name, v.value());
                b->quads.erase(b->quads.begin() + quad_i);
                --quad_i;
            } else {
                t.set_value_number_for_name(q.dest->name, q.dest->name);
                t.set_operation_for_value_number(op_hash_key, q.dest->name);
            }
        }

        // update phi functions for each successor
        for (auto succ : b->successors) {
            for (int i = 0; i < succ->phi_functions; ++i) {
                auto &phi = succ->quads[i];
                for (auto &op : phi.ops) {
                    auto v = t.get_value_number_by_name(op.value);
                    if (v.has_value())
                        op = Operand(v.value(), op.type, op.phi_predecessor);
                }
            }
        }

        // call for each child in dominator tree
        for (auto &[child_id, parent_id] : ir.id_to_idom)
            if (b->id == parent_id)
                dvnt(f.id_to_block.at(child_id), t);

        t.pop_table();
    }
};

static void run_global_value_numbering(Function &f) {
    GlobalValueNumberingDriver global_value_numbering(f);
    global_value_numbering.run();
    fmt::print("{}\n", global_value_numbering.ir.removed_quads);

    f = global_value_numbering.f;
}

#endif // TAC_PARSER_LOCAL_VALUE_NUMBERING_H
