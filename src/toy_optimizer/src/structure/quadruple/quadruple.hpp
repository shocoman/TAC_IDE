#ifndef TAC_PARSER_QUADRUPLE_HPP
#define TAC_PARSER_QUADRUPLE_HPP

#include <algorithm>
#include <fmt/ranges.h>
#include <optional>
#include <ostream>
#include <utility>
#include <vector>

#include "destination.hpp"
#include "operand.hpp"

struct Quad {
    enum class Type {
        Nop,
        Add,
        Sub,
        Mult,
        Div,
        Modulus,
        And,
        Or,
        Xor,
        Not,
        UMinus,
        Lt,
        Lte,
        Gt,
        Gte,
        Eq,
        Neq,
        Assign,
        Deref,
        Ref,
        ArrayGet,
        ArraySet,
        IfTrue,
        IfFalse,
        Goto,
        Halt,
        Call,
        Putparam,
        Return,
        PhiNode,
        Print,
        Getparam,
        VarDeclaration,
        ArrayDeclaration,
    };

    std::optional<Dest> dest{};
    std::vector<Operand> ops{};
    Type type{};

    Quad() = default;

    Quad(Operand op1, Operand op2, Type op_type, std::optional<Dest> dest = {}) : type(op_type), dest(std::move(dest)) {
        if (!op1.value.empty())
            ops.emplace_back(std::move(op1));
        if (!op2.value.empty())
            ops.emplace_back(std::move(op2));
    }

    std::optional<Operand> get_op(int i) const { return i < ops.size() ? std::make_optional(ops.at(i)) : std::nullopt; }

    void clear_op(int i) {
        if (i < ops.size())
            ops.erase(ops.begin() + i);
    }

    std::vector<std::string> get_rhs_names(bool include_constants = true) const {
        std::vector<std::string> rhs_vars;
        for (auto &op : ops) {
            if (op.is_var() || (include_constants && op.is_constant()))
                rhs_vars.push_back(op.get_string());
        }

        return rhs_vars;
    }

    std::optional<std::string> get_lhs() const {
        return (dest && dest->type == Dest::Type::Var) ? std::make_optional(dest->name) : std::nullopt;
    }

    std::vector<std::string> get_used_vars() const {
        auto used_vars = get_rhs_names(false);
        if (auto l = get_lhs(); l.has_value())
            used_vars.push_back(l.value());
        return used_vars;
    }

    bool is_jump() const { return type == Type::Goto || type == Type::IfTrue || type == Type::IfFalse; }

    bool is_unary() const {
        return type == Type::Deref || type == Type::Ref || type == Type::Assign || type == Type::UMinus;
    }

    bool is_conditional_jump() const { return type == Type::IfTrue || type == Type::IfFalse; }

    bool is_assignment() const {
        return not(type == Type::Return || type == Type::Nop || type == Type::Halt || type == Type::Putparam ||
                   type == Type::Getparam || type == Type::ArraySet || is_jump() ||
                   (type == Type::Call && !dest.has_value()));
    }
    bool is_comparison() const {
        return (type == Type::Lt || type == Type::Lte || type == Type::Gt || type == Type::Gte || type == Type::Eq ||
                type == Type::Neq);
    }

    bool is_binary() const {
        auto binaries = {
            Type::Add, Type::Sub, Type::Mult, Type::Div, Type::And, Type::Or,
            Type::Lt,  Type::Lte, Type::Gt,   Type::Gte, Type::Eq,  Type::Neq,
        };
        return std::any_of(binaries.begin(), binaries.end(),
                           [this](auto expr_type) { return this->type == expr_type; });
    }

    static bool is_commutative(Type t) {
        return t == Type::Add || t == Type::Mult || t == Type::Assign || t == Type::Eq || t == Type::Neq ||
               t == Type::And || t == Type::Or;
    }

    static bool is_critical(Type t) {
        auto critical = {Type::Print,          Type::Return,           Type::Putparam, Type::Call,
                         Type::VarDeclaration, Type::ArrayDeclaration, Type::ArraySet};
        return std::any_of(critical.begin(), critical.end(), [t](auto expr_type) { return t == expr_type; });
    }

    static bool is_foldable(Type t) {
        auto foldables = {Type::Add, Type::Sub, Type::Mult,   Type::Div, Type::Lt, Type::Lte, Type::Gt,  Type::Gte,
                          Type::Eq,  Type::Neq, Type::UMinus, Type::And, Type::Or, Type::Not, Type::Xor, Type::Modulus};
        return std::any_of(foldables.begin(), foldables.end(), [t](auto expr_type) { return t == expr_type; });
    }

    std::string fmt(bool only_rhs = false) const {
        std::optional<std::string> destination = dest.has_value() ? dest.value().fmt() : std::nullopt;

        std::string command;
        if (destination.has_value() && !only_rhs)
            command += destination.value() + " = ";

        std::optional<std::string> op;
        std::optional<std::string> unary_op;
        switch (type) {
        case Type::Add:
            op = "+";
            break;
        case Type::Sub:
            op = "-";
            break;
        case Type::Mult:
            op = "*";
            break;
        case Type::Div:
            op = "/";
            break;
        case Type::Or:
            op = "||";
            break;
        case Type::And:
            op = "&&";
            break;
        case Type::Xor:
            op = "^";
            break;
        case Type::Modulus:
            op = "%";
            break;
        case Type::UMinus:
            unary_op = "-";
            break;
        case Type::Lt:
            op = "<";
            break;
        case Type::Lte:
            op = "<=";
            break;
        case Type::Gt:
            op = ">";
            break;
        case Type::Gte:
            op = ">=";
            break;
        case Type::Eq:
            op = "==";
            break;
        case Type::Neq:
            op = "!=";
            break;
        case Type::Assign:
            unary_op = "";
            break;
        case Type::Deref:
            unary_op = "*";
            break;
        case Type::Ref:
            unary_op = "&";
            break;
        case Type::Not:
            unary_op = "!";
            break;
        case Type::ArrayGet:
            return command + get_op(0)->get_string() + "[" + get_op(1)->get_string() + "]";
        case Type::ArraySet:
            return fmt::format("{}[{}] = {}", destination.value(), get_op(0)->value, get_op(1)->value);
        case Type::IfTrue:
            return "if " + get_op(0)->get_string() + " goto " + destination.value();
        case Type::IfFalse:
            return "iffalse " + get_op(0)->get_string() + " goto " + destination.value();
        case Type::Goto:
            return "goto " + destination.value();
        case Type::Halt:
            return "halt";
        case Type::Call:
            return command + "call " + get_op(0)->get_string() + ", " + get_op(1)->get_string();
        case Type::Putparam:
            return "putparam " + get_op(0)->get_string();
        case Type::Getparam:
            return "getparam " + get_op(0)->get_string();
        case Type::Nop:
            return "nop";
        case Type::Return:
            return "return " + get_op(0)->get_string();
        case Type::Print:
            return "print_to_console " + get_op(0)->get_string();
        case Type::VarDeclaration:
            return destination.value() + ": ." + get_op(0)->get_string() + " " + get_op(1)->get_string();
        case Type::ArrayDeclaration:
            return fmt::format("{}: .block {}, {}, {}", destination.value(), get_op(0)->get_string(),
                               get_op(2)->get_string(), get_op(1)->get_string());
        case Type::PhiNode:
            std::vector<std::string> operators;
            for (auto &o : ops)
                operators.push_back(fmt::format("{}", o.value));
            return command + fmt::format("phi {}", operators);
        }

        if (unary_op.has_value()) {
            return command + unary_op.value() + get_op(0)->get_string();
        } else {
            return command + get_op(0)->get_string() + op.value() + get_op(1)->get_string();
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const Quad &quad) { return os << quad.fmt(); }
    bool operator==(const Quad &rhs) const {
        return ops == rhs.ops && type == rhs.type &&
               (!dest.has_value() || dest->type == rhs.dest->type && dest->name == rhs.dest->name);
    }
    bool operator!=(const Quad &rhs) const { return !(rhs == *this); }
};

#endif // TAC_PARSER_QUADRUPLE_HPP
