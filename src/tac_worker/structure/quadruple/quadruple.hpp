#ifndef TAC_PARSER_QUADRUPLE_HPP
#define TAC_PARSER_QUADRUPLE_HPP

#include <algorithm>
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
        UMinus,
        Lt,
        Gt,
        Eq,
        Neq,
        Assign,
        Deref,
        Ref,
        ArrayGet,
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

    Quad(Operand op1, Operand op2, Type op_type, std::optional<Dest> dest = {})
        : type(op_type), dest(std::move(dest)) {
        if (!op1.value.empty())
            ops.emplace_back(std::move(op1));
        if (!op2.value.empty())
            ops.emplace_back(std::move(op2));
    }

    std::optional<Operand> get_op(int i) const {
        if (i < ops.size())
            return ops.at(i);
        else
            return {};
    }
    void clear_op(int i) {
        if (i < ops.size())
            ops.erase(ops.begin() + i);
    }

    std::vector<std::string> get_rhs(bool include_constants = true) const {
        if (type == Type::Call) // dont count functions
            return {};

        std::vector<std::string> rhs_vars;
        if (dest) {
            if (dest.value().type == Dest::Type::ArraySet)
                rhs_vars.push_back(dest.value().index.value().value);
        }
        auto op1 = get_op(0);
        auto op2 = get_op(1);
        if (op1 && !op1->value.empty() && (include_constants || op1->is_var()))
            rhs_vars.push_back(op1->get_string());
        if (op2 && !op2->value.empty() && (include_constants || op2->is_var()))
            rhs_vars.push_back(op2->get_string());
        return rhs_vars;
    }
    std::optional<std::string> get_lhs() const {
        if (dest && !is_jump()) {
            return (dest.value().name);
        } else {
            return {};
        }
    }
    std::vector<std::string> get_used_vars() const {
        auto used_vars = get_rhs();
        if (auto l = get_lhs(); l.has_value()) {
            used_vars.push_back(l.value());
        }
        return used_vars;
    }

    bool is_jump() const {
        return type == Type::Goto || type == Type::IfTrue || type == Type::IfFalse;
    }
    bool is_unary() const {
        return type == Type::Deref || type == Type::Ref || type == Type::Assign ||
               type == Type::UMinus;
    }
    bool is_conditional_jump() const { return type == Type::IfTrue || type == Type::IfFalse; }
    bool is_assignment() const {
        return !(is_jump() || type == Type::Return || type == Type::Nop || type == Type::Halt ||
                 type == Type::Putparam || (type == Type::Call && !dest.has_value()));
    }
    bool is_binary() const {
        auto binaries = {
            Type::Add, Type::Sub, Type::Mult, Type::Div, Type::Lt, Type::Gt, Type::Eq, Type::Neq,
        };
        return std::any_of(binaries.begin(), binaries.end(),
                           [this](auto expr_type) { return this->type == expr_type; });
    }

    static bool is_commutative(Type t) {
        return t == Type::Add || t == Type::Mult || t == Type::Assign || t == Type::Eq ||
               t == Type::Neq;
    }
    static bool is_critical(Type t) {
        return t == Type::Print || t == Type::Return || t == Type::Putparam || t == Type::Call;
    }
    static bool is_foldable(Type t) {
        auto foldables = {
            Type::Add, Type::Sub, Type::Mult, Type::Div,    Type::Lt,
            Type::Gt,  Type::Eq,  Type::Neq,  Type::UMinus,
        };
        return std::any_of(foldables.begin(), foldables.end(),
                           [t](auto expr_type) { return t == expr_type; });
    }

    std::string fmt() const {
        std::optional<std::string> destination =
            dest.has_value() ? dest.value().fmt() : std::nullopt;

        std::string command;
        if (destination.has_value()) {
            command += destination.value() + " = ";
        }

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
        case Type::UMinus:
            unary_op = "-";
            break;
        case Type::Lt:
            op = "<";
            break;
        case Type::Gt:
            op = ">";
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
        case Type::ArrayGet:
            return destination.value() + " = " + get_op(0)->get_string() + "[" +
                   get_op(1)->get_string() + "]";
        case Type::IfTrue:
            return "if " + get_op(0)->get_string() + " goto " + destination.value();
        case Type::IfFalse:
            return "ifFalse " + get_op(0)->get_string() + " goto " + destination.value();
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
            return destination.value() + ": ." + get_op(0)->get_string() + " " +
                   get_op(1)->get_string();
        case Type::ArrayDeclaration:
            return destination.value() + ": .block " + dest->index->value + ", " +
                   get_op(0)->get_string() + ", " + get_op(1)->get_string();
        case Type::PhiNode:
            std::string output;
            for (auto &o : ops)
                output += o.value + " ";
            return destination.value() + " = phi ( " + output + ")";
        }

        if (unary_op.has_value()) {
            return destination.value_or("%") + " = " + unary_op.value() + get_op(0)->get_string();
        } else {
            return destination.value_or("%") + " = " + get_op(0)->get_string() + op.value() +
                   get_op(1)->get_string();
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const Quad &quad) { return os << quad.fmt(); }
    bool operator==(const Quad &rhs) const {
        return ops == rhs.ops && type == rhs.type &&
               (!dest.has_value() || dest->type == rhs.dest->type && dest->name == rhs.dest->name &&
                                         dest->index == rhs.dest->index);
    }
    bool operator!=(const Quad &rhs) const { return !(rhs == *this); }
};

#endif // TAC_PARSER_QUADRUPLE_HPP
