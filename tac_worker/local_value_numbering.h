//
// Created by shoco on 10/15/2020.
//

#ifndef TAC_PARSER_LOCAL_VALUE_NUMBERING_H
#define TAC_PARSER_LOCAL_VALUE_NUMBERING_H

#include <string>
#include <map>
#include <vector>

#include "quadruple.hpp"
#include "../DotWriter/DotWriter.h"



//std::string get_lhs(const std::map<std::string, int> &value_numbers, Quad &n, bool with_value = true) {
//    if (n.dest) {
//        std::string &name = n.dest.value().dest_name;
//        return with_value ? name + "(" + std::to_string(value_numbers.at(name)) + ")" : name;
//    } else
//        return "";
//}
//
//std::string get_rhs(const std::map<std::string, int> value_numbers, Quad &n, bool with_value = true) {
//    std::string l = n.op1.value;
//    std::string r = n.op2.value;
//    if (!n.op1.value.empty() && with_value) l += "(" + std::to_string(value_numbers.at(n.op1.value)) + ")";
//    if (!n.op2.value.empty() && with_value) r += "(" + std::to_string(value_numbers.at(n.op2.value)) + ")";
//
//    std::string rhs;
//    switch (n.type) {
//        case Quad::Type::Assign:
//            rhs = l;
//            break;
//        case Quad::Type::Add:
//            rhs = l + " + " + r;
//            break;
//        case Quad::Type::Sub:
//            rhs = l + " - " + r;
//            break;
//        case Quad::Type::Mult:
//            rhs = l + " * " + r;
//            break;
//        case Quad::Type::Div:
//            rhs = l + " / " + r;
//            break;
//        default:
//            break;
//    }
//    return rhs;
//}




static void constant_folding(Quad &n) {
    double l = n.op1.get_double();
    double r = n.op2.get_double();
    bool is_lnum = n.op1.is_number();
    bool is_rnum = n.op2.is_number();

    if (is_lnum && is_rnum) {
        double res = 0;
        switch (n.type) {
            case Quad::Type::Add:
                res = l + r;
                break;
            case Quad::Type::Sub:
                res = l - r;
                break;
            case Quad::Type::Mult:
                res = l * r;
                break;
            case Quad::Type::Div:
                res = l / r;
                break;
            default:
                break;
        }
        n.op1 = std::to_string(res);
        n.type = Quad::Type::Assign;
        n.op2.clear();
        return;
    }

    // algebraic identities
    // a - a = 0
    if (n.type == Quad::Type::Sub && n.op1 == n.op2) {
        n.op1 = Operand("0");
        n.op2.clear();
        n.type = Quad::Type::Assign;
    }
    // a / a = 1, a != 0
    if (n.type == Quad::Type::Div && n.op1 == n.op2 && n.op2.value != "0") {
        n.op1 = Operand("1");
        n.op2.clear();
        n.type = Quad::Type::Assign;
    }

    if (is_lnum || is_rnum) {
        // a * 0 = 0
        if (n.type == Quad::Type::Mult && (l == 0 && is_lnum || r == 0 && is_rnum)) {
            n.op1 = Operand("0");
            n.op2.clear();
            n.type = Quad::Type::Assign;
        }

        if (is_lnum) {
            // 0 + a = a OR 1 * a = a
            if (n.type == Quad::Type::Add && l == 0 || n.type == Quad::Type::Mult && l == 1) {
                n.op1 = n.op2;
                n.op2.clear();
                n.type = Quad::Type::Assign;
            }
                // 2 * a = a + a
            else if (n.type == Quad::Type::Mult && l == 2) {
                n.op1 = n.op2;
                n.type = Quad::Type::Add;
            }
        }

        if (is_rnum) {
            // a + 0 = a OR a * 1 = a OR a - 0 = a OR a / 1 = a
            if (n.type == Quad::Type::Add && r == 0
                || n.type == Quad::Type::Mult && r == 1
                || n.type == Quad::Type::Sub && r == 0
                || n.type == Quad::Type::Div && r == 1) {
                n.op2.clear();
                n.type = Quad::Type::Assign;
            }
                // a * 2 = a + a
            else if (n.type == Quad::Type::Mult && r == 2) {
                n.op2 = n.op1;
                n.type = Quad::Type::Add;
            }
        }
    }

}


static void local_value_numbering(std::vector<Quad>& quads) {
    std::map<std::string, int> value_numbers;
    std::map<int, std::string> value_numbers_to_names;
    std::map<std::tuple<Quad::Type, std::vector<int>>, int> operations;

//    quads.emplace_back(Quad("x", "y", Quad::Type::Add, Dest("a", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("x", "y", Quad::Type::Add, Dest("b", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("17", "4", Quad::Type::Div, Dest("Z", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("x", "y", Quad::Type::Add, Dest("c", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("t1", {}, Quad::Type::IfTrue, Dest("LABEL2", {}, Dest::Type::JumpLabel)));

    std::vector<Quad> new_quads;

    int current_number = 0;
    for (auto &n : quads) {

        if (n.type != Quad::Type::Assign) {
            constant_folding(n);
        }

        auto operand_values = std::vector<int>{};
        // get value number for every operand
        for (auto &op : n.get_rhs()) {
            if (op.empty()) continue;
            if (value_numbers.find(op) == value_numbers.end()) {
                value_numbers[op] = current_number;
                value_numbers_to_names[current_number] = op;
                if (n.type != Quad::Type::Assign)
                    current_number++;
            }
            operand_values.push_back(value_numbers.at(op));
        }
//        auto rhs = get_rhs(value_numbers, n);
//        auto q = Quad(n.op1, n.op2, n.type);

        if (Quad::is_commutative(n.type))
            std::sort(operand_values.begin(), operand_values.end());
        auto op_hash_key = std::tuple{n.type, operand_values};

        // if hash key is already in the table replace current type with a copy
        // otherwise insert new value number with hash key
        int op_value;
        auto op = operations.find(op_hash_key);
        if (op != operations.end() && op->second == value_numbers.at(value_numbers_to_names.at(op->second))) {
            op_value = op->second;
//            rhs += "; Same as " + value_numbers_to_names.at(op_value) + "(" + std::to_string(op_value) + ")";

            n.type = Quad::Type::Assign;
            n.op1 = Operand(value_numbers_to_names.at(op_value));
            n.op2.clear();
        } else {
            operations[op_hash_key] = current_number;
            op_value = current_number;
            value_numbers_to_names[current_number] = n.dest.value().dest_name;
            current_number++;
        }
        value_numbers[n.dest.value().dest_name] = op_value;

//        auto lhs = get_lhs(value_numbers, n);
//        std::cout << lhs << " = " << rhs << std::endl;

//        std::cout << n.fmt() << std::endl;

    }
}


#endif //TAC_PARSER_LOCAL_VALUE_NUMBERING_H