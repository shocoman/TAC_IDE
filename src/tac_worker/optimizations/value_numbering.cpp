//
// Created by shoco on 1/22/2021.
//

#include "value_numbering.hpp"

void constant_folding(Quad &n) {
    // unary
    if (n.ops.size() <= 1) {
        if (n.type == Quad::Type::UMinus) {
            n.ops[0] = Operand("-" + n.ops[0].value, n.ops[0].type);
            n.type = Quad::Type::Assign;
        }
        return;
    }

    bool is_lnum = n.get_op(0)->is_number();
    bool is_rnum = n.get_op(1)->is_number();

    double l = n.get_op(0)->as_double().value_or(0);
    double r = n.get_op(1)->as_double().value_or(0);
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
            // relational operations
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
        } break;
        }
        if (std::all_of(n.ops.begin(), n.ops.end(), [](Operand &a) { return a.is_int(); }))
            n.ops[0] = Operand(std::to_string((int)res));
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
            if (n.type == Quad::Type::Add && r == 0 || n.type == Quad::Type::Mult && r == 1 ||
                n.type == Quad::Type::Sub && r == 0 || n.type == Quad::Type::Div && r == 1) {
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

void local_value_numbering(std::vector<Quad> &quads, ValueNumberTableStack &t) {
    for (auto &q : quads) {
        if (q.is_jump())
            continue;

        if (Quad::is_foldable(q.type))
            constant_folding(q);

        // generate and/or save value number for every operand
        std::vector<int> operand_values;
        for (auto &op : q.get_rhs()) {
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
            std::string name = t.get_name_by_value_number(op_value).value();
            q.ops[0] = Operand(name);
            q.clear_op(1);
        } else {
            t.set_operation_value(op_hash_key, t.current_number);
            op_value = t.current_number;

            if (q.dest.has_value()) {
                t.set_name_for_value(t.current_number, q.dest->name);
                t.current_number++;
            }
        }

        if (q.dest.has_value())
            t.set_value_number_for_name(q.dest->name, op_value);
    }
}

void superlocal_value_numbering(Function &function) {
    std::vector<BasicBlock *> work_list = {function.find_entry_block()};
    std::unordered_set<int> visited_blocks;

    using SVNFuncType = std::function<void(BasicBlock *, ValueNumberTableStack &)>;
    SVNFuncType SVN = [&](BasicBlock *b, ValueNumberTableStack &t) {
        t.push_table();
        local_value_numbering(b->quads, t);

        for (auto &s : b->successors)
            if (s->predecessors.size() == 1)
                SVN(s, t);
            else if (visited_blocks.insert(b->id).second)
                work_list.push_back(s);

        t.pop_table();
    };

    ValueNumberTableStack t;
    while (!work_list.empty()) {
        BasicBlock *b = work_list.back();
        work_list.pop_back();
        SVN(b, t);
    }
}

void dominator_based_value_numbering(Function &function) {
    auto id_to_idom = find_immediate_dominators(function);

    using DVNTFuncType = std::function<void(BasicBlock *, DValueNumberTableStack &)>;
    DVNTFuncType dvnt = [&](BasicBlock *b, DValueNumberTableStack &t) {
        t.push_table();

        // process every phi function
        for (int i = 0; i < b->phi_functions; ++i) {
            auto &phi = b->quads[i];
            std::set<Operand> operands(phi.ops.begin(), phi.ops.end());
            // phi is meaningless (all operands are equal)
            if (std::equal(phi.ops.begin() + 1, phi.ops.end(), phi.ops.begin())) {
                t.set_value_number_for_name(phi.dest->name, phi.ops[0].value);
                b->quads.erase(b->quads.begin() + i);
                --b->phi_functions;
                --i;
            }
            // or phi is redundant (same as one of the previous phi functions)
            else if (auto v = t.get_phi_node_by_operation(operands); v.has_value()) {
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
        for (int i = b->phi_functions; i < b->quads.size(); ++i) {
            auto &q = b->quads[i];
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
                t.set_value_number_for_name(q.dest->name, v.value());
                b->quads.erase(b->quads.begin() + i);
                --i;
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
                    if (auto v = t.get_value_number_by_name(op.value); v.has_value()) {
                        auto pred = op.phi_predecessor;
                        op = Operand(v.value());
                        op.phi_predecessor = pred;
                    }
                }
            }
        }

        // call for each child in dominator tree
        for (auto &[child_id, parent_id] : id_to_idom)
            if (b->id == parent_id)
                dvnt(function.id_to_block.at(child_id), t);

        t.pop_table();
    };

    auto entry = function.find_entry_block();
    DValueNumberTableStack t;
    dvnt(entry, t);
}
