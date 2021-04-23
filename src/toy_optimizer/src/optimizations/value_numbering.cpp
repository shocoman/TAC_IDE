//
// Created by shoco on 1/22/2021.
//

#include "value_numbering.hpp"

void constant_folding(Quad &q) {
    // unary
    if (q.ops.size() <= 1) {
        if (q.type == Quad::Type::UMinus && q.ops[0].is_number()) {
            auto &num = q.ops[0].value;
            if (num[0] == '-')
                num.erase(num.begin());
            else
                num.insert(0, "-");

            q.ops[0] = Operand(num, q.ops[0].type);
            q.type = Quad::Type::Assign;
        }
        return;
    }

    bool is_lnum = q.get_op(0)->is_number();
    bool is_rnum = q.get_op(1)->is_number();

    double l = q.get_op(0)->as_double().value_or(0);
    double r = q.get_op(1)->as_double().value_or(0);
    if (is_lnum && is_rnum) {
        double res = 0;
        switch (q.type) {
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
        case Quad::Type::And:
            res = l && r;
            break;
        case Quad::Type::Or:
            res = l || r;
            break;
        default: {
            // relational operations
            bool res = false;
            switch (q.type) {
            case Quad::Type::Lt:
                res = l < r;
                break;
            case Quad::Type::Lte:
                res = l <= r;
                break;
            case Quad::Type::Gt:
                res = l > r;
                break;
            case Quad::Type::Gte:
                res = l >= r;
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

            q.ops[0] = Operand(res == false ? "false" : "true");
            q.type = Quad::Type::Assign;
            q.clear_op(1);
            return;
        } break;
        }
        if (std::all_of(q.ops.begin(), q.ops.end(), [](Operand &a) { return a.is_int(); }))
            q.ops[0] = Operand(std::to_string((int)res));
        else
            q.ops[0] = Operand(std::to_string(res));

        q.type = Quad::Type::Assign;
        q.clear_op(1);
        return;
    }

    // algebraic identities
    // a - a = 0
    if (q.type == Quad::Type::Sub && q.get_op(0) == q.get_op(1)) {
        q.ops[0] = Operand("0");
        q.clear_op(1);
        q.type = Quad::Type::Assign;
    }
    // a / a = 1, a != 0
    if (q.type == Quad::Type::Div && q.get_op(0) == q.get_op(1) && q.get_op(1)->value != "0") {
        q.ops[0] = Operand("1");
        q.clear_op(1);
        q.type = Quad::Type::Assign;
    }

    if (is_lnum || is_rnum) {
        // a * 0 = 0
        if (q.type == Quad::Type::Mult && (l == 0 && is_lnum || r == 0 && is_rnum)) {
            q.ops[0] = Operand("0", Operand::Type::LInt);
            q.clear_op(1);
            q.type = Quad::Type::Assign;
        }

        if (is_lnum) {
            // 0 + a = a OR 1 * a = a
            if (q.type == Quad::Type::Add && l == 0 || q.type == Quad::Type::Mult && l == 1) {
                q.ops[0] = q.get_op(1).value();
                q.clear_op(1);
                q.type = Quad::Type::Assign;
            }
            // 2 * a = a + a
            else if (q.type == Quad::Type::Mult && l == 2) {
                q.ops[0] = q.get_op(1).value();
                q.type = Quad::Type::Add;
            }
        }

        if (is_rnum) {
            // a + 0 = a OR a * 1 = a OR a - 0 = a OR a / 1 = a
            if (q.type == Quad::Type::Add && r == 0 || q.type == Quad::Type::Mult && r == 1 ||
                q.type == Quad::Type::Sub && r == 0 || q.type == Quad::Type::Div && r == 1) {
                q.clear_op(1);
                q.type = Quad::Type::Assign;
            }
            // a * 2 = a + a
            else if (q.type == Quad::Type::Mult && r == 2) {
                q.ops[1] = q.get_op(0).value();
                q.type = Quad::Type::Add;
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
        for (auto &op : q.get_rhs_names()) {
            if (!t.get_value_number_by_name(op).has_value()) {
                t.set_value_number_for_name(op, t.current_number);
                t.set_name_for_value(t.current_number, op);
                if (q.type != Quad::Type::Assign)
                    t.current_number++;
            }
            operand_values.push_back(*t.get_value_number_by_name(op));
        }

        if (Quad::is_commutative(q.type))
            std::sort(operand_values.begin(), operand_values.end());
        auto op_hash_key = std::tuple{q.type, operand_values};

        // if hash key is already in the table replace current type with a copy operation
        int op_value;
        auto op = t.get_value_number_by_operation(op_hash_key);
        if (op && op == t.get_value_number_by_name(*t.get_name_by_value_number(*op))) {
            op_value = op.value();

            std::string name = *t.get_name_by_value_number(op_value);
            q = Quad(Operand(name), {}, Quad::Type::Assign, q.dest);
        }
        // otherwise insert new value number with hash key
        else {
            t.set_operation_value(op_hash_key, t.current_number);
            op_value = t.current_number;

            if (q.dest.has_value()) {
                t.set_name_for_value(t.current_number, q.dest->name);
                ++t.current_number;
            }
        }

        if (q.dest.has_value())
            t.set_value_number_for_name(q.dest->name, op_value);
    }
}

void superlocal_value_numbering(Function &function) {
    std::vector<BasicBlock *> work_list = {function.get_entry_block()};
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
