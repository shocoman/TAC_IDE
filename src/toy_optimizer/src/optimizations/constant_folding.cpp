//
// Created by victor on 29.04.2021.
//

#include "constant_folding.hpp"

void constant_folding(Quad &q) {
    if (not Quad::is_foldable(q.type))
        return;
    bool is_unary = q.ops.size() <= 1;

    // unary
    if (is_unary) {
        if (q.type == Quad::Type::UMinus && q.ops[0].is_number()) {
            auto &num = q.ops[0].value;
            if (num[0] == '-')
                num.erase(num.begin());
            else
                num.insert(0, "-");

            q.ops[0] = Operand(num, q.ops[0].type);
            q.type = Quad::Type::Assign;
        } else if (q.type == Quad::Type::Not && q.ops[0].is_number()) {
            bool res = not q.get_op(0)->is_true();
            q.ops[0] = Operand(res == false ? "false" : "true", Operand::Type::LBool);
            q.type = Quad::Type::Assign;
        }
        return;
    }

    // if both lhs and rhs are the same variable or literal
    if (q.get_op(0)->get_string() == q.get_op(1)->get_string()) {
        bool is_eligible = true;
        bool res = false;
        switch (q.type) {
        case Quad::Type::Lt:
        case Quad::Type::Gt:
        case Quad::Type::Neq:
        case Quad::Type::Xor:
            res = false;
            break;
        case Quad::Type::Eq:
        case Quad::Type::Lte:
        case Quad::Type::Gte:
            res = true;
            break;
        case Quad::Type::And:
        case Quad::Type::Or:
            res = q.get_op(0)->is_true();
            break;
        default:
            is_eligible = false;
        }
        if (is_eligible) {
            q.clear_op(1);
            q.type = Quad::Type::Assign;
            q.ops[0] = Operand(res == false ? "false" : "true", Operand::Type::LBool);
            return;
        }
    }

    bool is_lnum = q.get_op(0)->is_number();
    bool is_rnum = q.get_op(1)->is_number();

    double l = q.get_op(0)->as_double().value_or(0);
    double r = q.get_op(1)->as_double().value_or(0);
    if (is_lnum && is_rnum) {
        if (q.is_comparison()) {
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

            q.clear_op(1);
            q.ops[0] = Operand(res == false ? "false" : "true");
            q.type = Quad::Type::Assign;
            return;
        }

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
            res = (int)l && (int)r;
            break;
        case Quad::Type::Or:
            res = (int)l || (int)r;
            break;
        case Quad::Type::Xor:
            res = int(l) ^ (int)r;
            break;
        case Quad::Type::Modulus:
            res = (int)l % (int)r;
            break;
        default:
            return;
            break;
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
        q.ops[0] = Operand("0", Operand::Type::LInt);
        q.clear_op(1);
        q.type = Quad::Type::Assign;
    }
    // a / a = 1, a != 0
    if (q.type == Quad::Type::Div && q.get_op(0) == q.get_op(1) && q.get_op(1)->value != "0") {
        q.ops[0] = Operand("1", Operand::Type::LInt);
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