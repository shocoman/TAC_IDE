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
    std::map<int, std::string> value_number_to_name;
    std::map<OpRecord, int> operations;
};


struct ValueNumberTableStack {
    std::vector<ValueNumberTable> tables;
    int current_number = 0;

    void set_value_number_for_name(std::string name, int value) {
        tables.back().value_numbers[name] = value;
    }

    void set_name_for_value(int value, std::string name) {
        tables.back().value_number_to_name[value] = name;
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

    void push_table() {
        tables.emplace_back();
    }

    void pop_table() {
        tables.pop_back();
    }
};


static void constant_folding(Quad &n) {
    if (n.type == Quad::Type::UMinus) {
        n.ops[0] = Operand("-" + n.ops[0].value, n.ops[0].type);
        n.type = Quad::Type::Assign;
    }
    if (n.ops.size() <= 1) return;


    bool is_lnum = n.get_op(0)->is_number();
    bool is_rnum = n.get_op(1)->is_number();

    double l = n.get_op(0)->get_double();
    double r = n.get_op(1)->get_double();
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
            default: {
                // relation operations

                bool res = false;
                switch (n.type) {
                    case Quad::Type::Lt:
                        res = l < r;
                        break;
                    case Quad::Type::Gt:
                        res = l > r;
                        break;
                    case Quad::Type::Eq:
                        res = l == r;
                        break;
                    case Quad::Type::Neq:
                        res = l != r;
                        break;
                    default:
                        break;
                }

                n.ops[0] = Operand(res == false ? "false" : "true");
                n.type = Quad::Type::Assign;
                n.clear_op(1);
                return;
            }
                break;
        }
        if (std::all_of(n.ops.begin(), n.ops.end(), [](Operand &a) { return a.is_int(); }))
            n.ops[0] = Operand(std::to_string((int) res));
        else
            n.ops[0] = Operand(std::to_string(res));

        n.type = Quad::Type::Assign;
        n.clear_op(1);
        return;
    }

    // algebraic identities
    // a - a = 0
    if (n.type == Quad::Type::Sub && n.get_op(0) == n.get_op(1)) {
        n.ops[0] = Operand("0");
        n.clear_op(1);
        n.type = Quad::Type::Assign;
    }
    // a / a = 1, a != 0
    if (n.type == Quad::Type::Div && n.get_op(0) == n.get_op(1) && n.get_op(1)->value != "0") {
        n.ops[0] = Operand("1");
        n.clear_op(1);
        n.type = Quad::Type::Assign;
    }

    if (is_lnum || is_rnum) {
        // a * 0 = 0
        if (n.type == Quad::Type::Mult && (l == 0 && is_lnum || r == 0 && is_rnum)) {
            n.ops[0] = Operand("0", Operand::Type::LInt);
            n.clear_op(1);
            n.type = Quad::Type::Assign;
        }

        if (is_lnum) {
            // 0 + a = a OR 1 * a = a
            if (n.type == Quad::Type::Add && l == 0 || n.type == Quad::Type::Mult && l == 1) {
                n.ops[0] = n.get_op(1).value();
                n.clear_op(1);
                n.type = Quad::Type::Assign;
            }
                // 2 * a = a + a
            else if (n.type == Quad::Type::Mult && l == 2) {
                n.ops[0] = n.get_op(1).value();
                n.type = Quad::Type::Add;
            }
        }

        if (is_rnum) {
            // a + 0 = a OR a * 1 = a OR a - 0 = a OR a / 1 = a
            if (n.type == Quad::Type::Add && r == 0
                || n.type == Quad::Type::Mult && r == 1
                || n.type == Quad::Type::Sub && r == 0
                || n.type == Quad::Type::Div && r == 1) {
                n.clear_op(1);
                n.type = Quad::Type::Assign;
            }
                // a * 2 = a + a
            else if (n.type == Quad::Type::Mult && r == 2) {
                n.ops[1] = n.get_op(0).value();
                n.type = Quad::Type::Add;
            }
        }
    }

}


static void local_value_numbering(std::vector<Quad> &quads, ValueNumberTableStack &t) {
//    std::map<std::string, int> value_numbers;
//    std::map<int, std::string> value_number_to_name;
//    std::map<OpRecord, int> operations;
//    quads.clear();
//    quads.emplace_back(Quad("x", "y", Quad::Type::Add, Dest("a", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("x", "y", Quad::Type::Add, Dest("b", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("x", "0", Quad::Type::Mult, Dest("a", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("x", "y", Quad::Type::Add, Dest("c", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("t1", {}, Quad::Type::IfTrue, Dest("LABEL2", {}, Dest::Type::JumpLabel)));
//    quads.emplace_back(Quad("t1", {}, Quad::Type::IfTrue, Dest("LABEL2", {}, Dest::Type::JumpLabel)));
//    quads.emplace_back(Quad("0", {}, Quad::Type::Assign, Dest("a", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("0", {}, Quad::Type::Assign, Dest("b", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("0", {}, Quad::Type::Assign, Dest("c", {}, Dest::Type::Var)));
//    quads.emplace_back(Quad("0", {}, Quad::Type::Assign, Dest("d", {}, Dest::Type::Var)));


    for (auto &q : quads) {
        if (q.is_jump()) {
            std::cout << q.fmt() << std::endl;
            continue;
        }

        if (Quad::is_foldable(q.type)) {
            constant_folding(q);
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
            std::string val = t.get_name_by_value_number(op_value).value();
            q.ops[0] = Operand(val);
            q.clear_op(1);

        } else {
            t.set_operation_value(op_hash_key, t.current_number);
            op_value = t.current_number;

            t.set_name_for_value(t.current_number, q.dest.value().name);
            t.current_number++;
        }
        t.set_value_number_for_name(q.dest.value().name, op_value);

        std::cout << q.fmt() << std::endl;
    }
    std::cout << std::endl;
}


static void superlocal_value_numbering(std::vector<std::unique_ptr<BasicBlock>> &blocks) {
    std::vector<BasicBlock *> work_list{blocks.front().get()};
    std::set<int> visited_blocks;

    using SVNFuncType = std::function<void(BasicBlock *, ValueNumberTableStack &)>;
    SVNFuncType SVN = [&](BasicBlock *b, ValueNumberTableStack &t) {
        t.push_table();
        local_value_numbering(b->quads, t);

        for (auto &s : b->successors) {
            if (s->predecessors.size() == 1) {
                SVN(s, t);
            } else if (visited_blocks.find(b->id) == visited_blocks.end()) {
                visited_blocks.insert(b->id);
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











using DOpRecord = std::tuple<Quad::Type, std::vector<std::string>>;
struct DValueNumberTable {
    std::map<std::string, std::string> value_numbers;
    std::map<std::string, std::string> value_number_to_name;
    std::map<DOpRecord, std::string> operations;
    std::map<std::set<Operand>, std::string> phi_nodes;
};


struct DValueNumberTableStack {
    std::vector<DValueNumberTable> tables;

    void set_value_number_for_name(std::string name, std::string value) {
        tables.back().value_numbers[name] = value;
    }

    void set_name_for_value(std::string value, std::string name) {
        tables.back().value_number_to_name[value] = name;
    }

    void set_operation_value(DOpRecord op, std::string value) {
        tables.back().operations[op] = value;
    }

    std::optional<std::string> get_value_number_by_name(const std::string &name) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            if (auto v = it->value_numbers.find(name); v != it->value_numbers.end()) {
                return v->second;
            }
        }
        return {};
    }

    std::optional<std::string> get_name_by_value_number(std::string value) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            if (auto n = it->value_number_to_name.find(value); n != it->value_number_to_name.end()) {
                return n->second;
            }
        }
        return {};
    }

    std::optional<std::string> get_value_number_by_operation(const DOpRecord &op) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            if (auto v = it->operations.find(op); v != it->operations.end()) {
                return v->second;
            }
        }
        return {};
    }


    void set_phi_node_for_value(std::set<Operand> &ops, const std::string &name) {
        tables.back().phi_nodes[ops] = name;
    }

    std::optional<std::string> get_phi_node_by_operation(std::set<Operand> &ops) {
        for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
            if (auto v = it->phi_nodes.find(ops); v != it->phi_nodes.end()) {
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




static void dominator_based_value_numbering(BasicBlocks &blocks, ID2Block &id_to_block, ID2IDOM &id_to_idom) {

    std::set<int> visited_blocks;

    using DVNTFuncType = std::function<void(BasicBlock *, DValueNumberTableStack &)>;
    DVNTFuncType dvnt = [&](BasicBlock *b, DValueNumberTableStack &t) {
        t.push_table();

        // process phi functions
        for (int i = 0; i < b->phi_functions; ++i) {
            auto &phi = b->quads[i];

            std::set<Operand> ops_set(phi.ops.begin(), phi.ops.end());
            // is meaningless (all operands are equal)
            if (std::equal(phi.ops.begin() + 1, phi.ops.end(), phi.ops.begin())) {
                t.set_value_number_for_name(phi.dest->name, phi.ops[0].value);
                b->quads.erase(b->quads.begin() + i);
                --b->phi_functions;
                --i;
            }
            // is redundant (same as previous phi functions)
            else if (auto v = t.get_phi_node_by_operation(ops_set); v.has_value()) {
                t.set_value_number_for_name(phi.dest->name, v.value());
                b->quads.erase(b->quads.begin() + i);
                --b->phi_functions;
                --i;
            } else {
                t.set_value_number_for_name(phi.dest->name, phi.dest->name);
                t.set_phi_node_for_value(ops_set, phi.dest->name);
            }
        }

        // work through each assignment 'x = y op z'
        for (int i = b->phi_functions; i < b->quads.size(); ++i) {
            auto &q = b->quads[i];
            if (q.is_jump()) continue;

            if (Quad::is_foldable(q.type)) {
                constant_folding(q);
            }

            std::vector<std::string> expr;
            for (auto &op : q.ops) {
                if (auto v = t.get_value_number_by_name(op.value); v.has_value()) {
                    op = Operand(v.value());
                } else {
                    t.set_value_number_for_name(op.value, op.value);
                }
                expr.push_back(op.value);
            }

            if (Quad::is_commutative(q.type))
                std::sort(expr.begin(), expr.end());
            auto op_hash_key = std::tuple{q.type, expr};
            if (auto v = t.get_value_number_by_operation(op_hash_key); v.has_value()) {
                t.set_value_number_for_name(q.dest->name, v.value());
                b->quads.erase(b->quads.begin() + i);
                --i;
            } else {
                t.set_value_number_for_name(q.dest->name, q.dest->name);
                t.set_operation_value(op_hash_key, q.dest->name);
            }
        }





        // call for each child in dominator tree
        for (auto &[child_id, parent_id] : id_to_idom)
            if (b->id == parent_id)
                dvnt(id_to_block.at(child_id), t);

        t.pop_table();
    };

    // assume first block in blocks is entry
    auto b = blocks.front().get();
    DValueNumberTableStack t;
    dvnt(b, t);

}


#endif //TAC_PARSER_LOCAL_VALUE_NUMBERING_H