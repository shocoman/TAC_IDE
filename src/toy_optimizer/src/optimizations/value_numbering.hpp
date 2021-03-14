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

#include "../structure/function.hpp"
#include "../data_flow_analyses/dominators.hpp"

void constant_folding(Quad &n);

struct ValueNumberTableStack {
    using OpRecord = std::tuple<Quad::Type, std::vector<int>>;
    struct ValueNumberTable {
        std::map<std::string, int> value_numbers;
        std::map<int, std::string> value_number_to_name;
        std::map<OpRecord, int> operations;
    };

    std::vector<ValueNumberTable> tables;
    int current_number = 0;

    void set_value_number_for_name(std::string name, int value) {
        tables.back().value_numbers[name] = value;
    }

    void set_name_for_value(int value, std::string name) {
        tables.back().value_number_to_name[value] = name;
    }

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

struct DValueNumberTableStack {
    using DOpRecord = std::tuple<Quad::Type, std::vector<std::string>>;
    struct DValueNumberTable {
        std::unordered_map<std::string, std::string> name_to_value_number;
        std::unordered_map<std::string, std::string> value_number_to_name;
        std::map<DOpRecord, std::string> operations;
        std::map<std::set<Operand>, std::string> phi_nodes;
    };

    std::vector<DValueNumberTable> tables;
    std::map<std::set<Operand>, std::string> phi_nodes;

    void set_value_number_for_name(std::string name, std::string value) {
        tables.back().name_to_value_number[name] = value;
    }

    void set_name_for_value(std::string value, std::string name) {
        tables.back().value_number_to_name[value] = name;
    }

    void set_operation_for_value_number(DOpRecord op, std::string value) { tables.back().operations[op] = value; }

    std::optional<std::string> get_value_number_by_name(const std::string &name) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            auto v = it->name_to_value_number.find(name);
            if (v != it->name_to_value_number.end())
                return v->second;
        }
        return {};
    }

    std::optional<std::string> get_name_by_value_number(const std::string &value) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            auto n = it->value_number_to_name.find(value);
            if (n != it->value_number_to_name.end())
                return n->second;
        }
        return {};
    }

    std::optional<std::string> get_value_number_by_operation(const DOpRecord &op) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            auto v = it->operations.find(op);
            if (v != it->operations.end())
                return v->second;
        }
        return {};
    }

    void set_phi_node_for_value(std::set<Operand> &ops, const std::string &name) {
        phi_nodes[ops] = name;
    }

    std::optional<std::string> get_phi_node_by_operation(std::set<Operand> &ops) {
        auto v = phi_nodes.find(ops);
        return v != phi_nodes.end() ? std::make_optional(v->second) : std::nullopt;
    }

    void push_table() { tables.emplace_back(); }

    void pop_table() { tables.pop_back(); }
};

void dominator_based_value_numbering(Function &function);

#endif // TAC_PARSER_LOCAL_VALUE_NUMBERING_H
