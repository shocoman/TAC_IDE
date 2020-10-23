//
// Created by shoco on 10/15/2020.
//

#ifndef TAC_PARSER_LOCAL_VALUE_NUMBERING_H
#define TAC_PARSER_LOCAL_VALUE_NUMBERING_H

#include <string>
#include <map>
#include <vector>
#include <functional>

#include "quadruple.hpp"
#include "basic_block.h"


using OpRecord = std::tuple<Quad::Type, std::vector<int>>;
struct ValueNumberTable {
    std::map<std::string, int> value_numbers;
    std::map<int, std::string> value_numbers_to_names;
    std::map<OpRecord, int> operations;
};


struct ValueNumberTableStack {
    std::vector<ValueNumberTable> tables;
    int current_number = 0;

    void set_value_number_for_name(std::string name, int value) {
        tables.back().value_numbers[name] = value;
    }

    void set_name_for_value(int value, std::string name) {
        tables.back().value_numbers_to_names[value] = name;
    }

    void set_operation_value(OpRecord op, int value) {
        tables.back().operations[op] = value;
    }

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
            if (auto n = it->value_numbers_to_names.find(value); n != it->value_numbers_to_names.end()) {
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

    void push_table() {
        tables.emplace_back();
    }

    void pop_table() {
        tables.pop_back();
    }
};


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


static void local_value_numbering(std::vector<Quad> &quads, ValueNumberTableStack &t) {
//    std::map<std::string, int> value_numbers;
//    std::map<int, std::string> value_numbers_to_names;
//    std::map<OpRecord, int> operations;

//    quads.emplace_back(Quad("x", "y", Quad::Type::Add, Dest("a", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("x", "y", Quad::Type::Add, Dest("b", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("17", "4", Quad::Type::Div, Dest("Z", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("x", "y", Quad::Type::Add, Dest("c", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("t1", {}, Quad::Type::IfTrue, Dest("LABEL2", {}, Dest::Type::JumpLabel)));
//    quads.emplace_back(Quad("t1", {}, Quad::Type::IfTrue, Dest("LABEL2", {}, Dest::Type::JumpLabel)));


    for (auto &q : quads) {
        if (q.is_jump()) {
            std::cout << q.fmt() << std::endl;
            continue;
        }

        if (q.type != Quad::Type::Assign) {
            //constant_folding(q);
        }

        // generate and/or save value number for every operand
        auto operand_values = std::vector<int>{};
        for (auto &op : q.get_rhs()) {
            if (op.empty()) continue;

            if (!t.get_value_number_by_name(op).has_value()) {
                t.set_value_number_for_name(op, t.current_number);
                t.set_name_for_value(t.current_number, op);
                if (q.type != Quad::Type::Assign)
                    t.current_number++;
            }
            operand_values.push_back(t.get_value_number_by_name(op).value());
//            if (value_numbers.find(op) == value_numbers.end()) {
//                value_numbers[op] = current_number;
//                value_numbers_to_names[current_number] = op;
//                if (q.type != Quad::Type::Assign)
//                    current_number++;
//            }
//            operand_values.push_back(value_numbers.at(op));
        }

        if (Quad::is_commutative(q.type))
            std::sort(operand_values.begin(), operand_values.end());
        auto op_hash_key = std::tuple{q.type, operand_values};

        // if hash key is already in the table replace current type with a copy
        // otherwise insert new value number with hash key
        int op_value;
        auto op = t.get_value_number_by_operation(op_hash_key);
        if (op.has_value() && op == t.get_value_number_by_name(*t.get_name_by_value_number(*op))) {
            op_value = op.value();

            q.type = Quad::Type::Assign;
            q.op1 = Operand(t.get_name_by_value_number(op_value).value());
            q.op2.clear();
        } else {
            t.set_operation_value(op_hash_key, t.current_number);
            op_value = t.current_number;

            t.set_name_for_value(t.current_number, q.dest.value().dest_name);
            t.current_number++;
        }
        t.set_value_number_for_name(q.dest.value().dest_name, op_value);

        std::cout << q.fmt() << std::endl;
//        auto op = operations.find(op_hash_key);
//        if (op != operations.end() && op->second == value_numbers.at(value_numbers_to_names.at(op->second))) {
//            op_value = op->second;
//
//            q.type = Quad::Type::Assign;
//            q.op1 = Operand(value_numbers_to_names.at(op_value));
//            q.op2.clear();
//        } else {
//            operations[op_hash_key] = current_number;
//            op_value = current_number;
//            value_numbers_to_names[current_number] = q.dest.value().dest_name;
//            current_number++;
//        }
//        value_numbers[q.dest.value().dest_name] = op_value;
    }
    std::cout << std::endl;
}


static void superlocal_value_numbering(std::vector<std::unique_ptr<BasicBlock>> &blocks) {
    std::vector<BasicBlock *> work_list{blocks.front().get()};
    std::set<int> visited_blocks;

    using SVNFuncType = std::function<void(BasicBlock *, ValueNumberTableStack &)>;
    SVNFuncType SVN = [&](BasicBlock *b, ValueNumberTableStack &t) {
        visited_blocks.insert(b->id);
        t.push_table();
        local_value_numbering(b->quads, t);

        for (auto &s : b->successors) {
            if (s->predecessors.size() == 1) {
                SVN(s, t);
            } else if (visited_blocks.find(b->id) == visited_blocks.end()) {
                work_list.push_back(s);
            }
        }

        t.pop_table();
    };

    ValueNumberTableStack t;
    while (!work_list.empty()) {
        BasicBlock *b = work_list.back();
        work_list.pop_back();
        SVN(b, t);
    }

}


#endif //TAC_PARSER_LOCAL_VALUE_NUMBERING_H