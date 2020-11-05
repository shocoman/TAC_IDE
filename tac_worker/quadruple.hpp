#ifndef TAC_PARSER_QUADRUPLE_HPP
#define TAC_PARSER_QUADRUPLE_HPP

#include <string>
#include <ostream>
#include <utility>
#include <vector>
#include <optional>
#include <iostream>


struct Dest {
    enum class Type {
        None, Var, ArraySet, Deref, JumpLabel,
    };

    Type type{};
    std::string name;
    std::optional<std::string> index;

    Dest(std::string dest_name, std::optional<std::string> element_name, Type dest_type)
            : name(std::move(dest_name)), index(std::move(element_name)), type(dest_type) {}

    Dest() = default;

    friend std::ostream &operator<<(std::ostream &os, const Dest &destination) {
        const char *type_names[] = {"None", "Var", "ArraySet", "Deref", "JumpLabel"};
        os << "type: " << type_names[static_cast<int>(destination.type)]
           << "; name: " << destination.name;
        if (destination.index.has_value()) os << "; index: " << destination.index.value();
        return os;
    }

    std::optional<std::string> fmt() const {
        switch (type) {
            case Type::Var:
                return name;
            case Type::ArraySet:
                return name + "[" + index.value() + "]";
            case Type::Deref:
                return "*" + name;
            case Type::JumpLabel:
                return name;
            default:
                return {};
        }
    }
};


struct Operand {
    enum class Type {
        None, Var, LInt, LDouble, LBool, LChar
    };

    std::string value;
    int predecessor_id = -1;
    Type type;

    Operand() : type(Type::None) {}

    Operand(std::string s, Type t) : value(std::move(s)), type(t) {}

    explicit Operand(const std::string &s) {
        char *end = nullptr;
        if (strtol(s.c_str(), &end, 10); end != s.c_str() && *end == '\0') {
            type = Type::LInt;
        } else if (strtod(s.c_str(), &end); end != s.c_str() && *end == '\0') {
            type = Type::LDouble;
        } else if (s == "true" || s == "false") {
            type = Type::LBool;
        } else if (!s.empty()) {
            type = Type::Var;
        } else {
            type = Type::None;
        }
        value = s;
    }

    std::string get_string() const {
        return value;
    }

    int get_int() const {
        char *end = nullptr;
        return strtol(value.c_str(), &end, 10);
    }

    double get_double() const {
        char *end = nullptr;
        return strtod(value.c_str(), &end);
    }

    bool is_var() const { return type == Type::Var; }

    bool is_constant() const { return !is_var(); }

    bool is_int() const { return type == Type::LInt; }

    bool is_double() const { return type == Type::LDouble; }

    bool is_number() const { return type == Type::LInt || type == Type::LDouble; }

    void clear() {
        value.clear();
        type = Type::None;
    }

    bool operator==(const Operand &rhs) const {
        if (is_number() && rhs.is_number())
            return get_double() == rhs.get_double();
        else
            return value == rhs.value;
    }

};

struct Quad {
    enum class Type {
        Nop, Add, Sub, Mult, Div, UMinus,
        Lt, Gt, Eq, Neq,
        Assign, Deref, Ref, ArrayGet,
        IfTrue, IfFalse, Goto, Halt, Call, Param, Return,
        PhiNode,
        Print,
    };

    std::optional<Dest> dest{};

    std::vector<Operand> ops{};
//    Operand op1{};
//    Operand op2{};
    Type type{};


    Quad(std::string op1, std::string op2, Type op_type, const Dest &dest = {})
            : type(op_type), dest(dest) {
        if (!op1.empty()) ops.emplace_back(std::move(op1));
        if (!op2.empty()) ops.emplace_back(std::move(op2));
    }

    Quad() = default;

    static bool is_commutative(Type t) {
        return t == Type::Add || t == Type::Mult || t == Type::Assign
               || t == Type::Eq || t == Type::Neq;
    }

    static bool is_jump(Type t) {
        return t == Type::Goto || t == Type::IfTrue || t == Type::IfFalse;
    }

    static bool is_critical(Type t) {
        return t == Type::Print || t == Type::Return;
    }

    static bool is_foldable(Type t) {
        return !(t == Type::Goto || t == Type::Print || t == Type::Return || t == Type::Assign || t == Type::PhiNode);
    }


    std::vector<std::string> get_used_vars() const {
        auto used_vars = get_rhs();
        if (auto l = get_lhs(); l.has_value()) {
            used_vars.push_back(l.value());
        }
        return used_vars;
    }

    std::optional<std::string> get_lhs() const {
        if (dest && !is_jump()) {
            return (dest.value().name);
        } else {
            return {};
        }
    }

    void clear_op(int i) {
        if (i < ops.size())
            ops.erase(ops.begin() + i);
    }

    std::optional<Operand> get_op(int i) const {
        if (ops.size() > i)
            return ops.at(i);
        else return {};
    }

    std::vector<std::string> get_rhs(bool include_constants = true) const {
        std::vector<std::string> rhs_vars;
        if (dest) {
            if (dest.value().type == Dest::Type::ArraySet)
                rhs_vars.push_back(dest.value().index.value());
        }
        auto op1 = get_op(0);
        auto op2 = get_op(1);
        if (op1 && !op1->value.empty() && (include_constants || op1->is_var())) rhs_vars.push_back(op1->get_string());
        if (op2 && !op2->value.empty() && (include_constants || op2->is_var())) rhs_vars.push_back(op2->get_string());
        return rhs_vars;
    }

    bool is_jump() const {
        return type == Type::Goto
               || type == Type::IfTrue
               || type == Type::IfFalse;
    }

    bool is_unary() const {
        return type == Type::Deref
               || type == Type::Ref
               || type == Type::Assign
               || type == Type::UMinus;
    }

    friend std::ostream &operator<<(std::ostream &os, const Quad &quad) {
        const char *type_names[] = {"Nop", "Add", "Sub", "Mult", "Div", "UMinus", "Lt", "Gt", "Eq", "Neq", "Assign",
                                    "Deref", "Ref", "ArrayGet", "IfTrue", "IfFalse", "Goto", "Halt", "Call", "Param",
                                    "Return", "PhiNode", "Print"};
        if (quad.dest.has_value()) os << "dest: { " << quad.dest.value() << "}" << "; ";
        if (quad.get_op(0)->type != Operand::Type::None)
            os << "op1: " << quad.get_op(0)->get_string() << "; ";
        if (quad.get_op(1)->type != Operand::Type::None)
            os << "op2: " << quad.get_op(1)->get_string() << "; ";
        os << "type: " << type_names[static_cast<int>(quad.type)];
        return os;
    }


    std::string fmt() const {
        std::optional<std::string> destination = dest.has_value() ? dest.value().fmt() : std::nullopt;

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
                return destination.value() + " = " + get_op(0)->get_string() + "[" + get_op(1)->get_string() + "]";
            case Type::IfTrue:
                return "if " + get_op(0)->get_string() + " goto " + destination.value();
            case Type::IfFalse:
                return "ifFalse " + get_op(0)->get_string() + " goto " + destination.value();
            case Type::Goto:
                return "goto " + destination.value();
            case Type::Halt:
                return "halt";
            case Type::Call:
                return "call " + get_op(0)->get_string() + " " + get_op(0)->get_string();
            case Type::Param:
                return "param " + get_op(0)->get_string();
            case Type::Nop:
                return "nop";
            case Type::Return:
                return "return " + get_op(0)->get_string();
            case Type::Print:
                return "print " + get_op(0)->get_string();
            case Type::PhiNode:
                std::string output;
                for (auto &o : ops)
                    output += o.value + " ";
                return destination.value() + " = phi ( " + output + ")";
        }

        if (unary_op.has_value()) {
            return destination.value_or("%") + " = " + unary_op.value() + get_op(0)->get_string();
        } else {
            return destination.value_or("%") + " = " + get_op(0)->get_string() + op.value() + get_op(1)->get_string();
        }
    }


    bool operator==(const Quad &rhs) const {
        return ops == rhs.ops &&
               type == rhs.type &&
               (!dest.has_value() || dest->type == rhs.dest->type &&
                                     dest->name == rhs.dest->name &&
                                     dest->index == rhs.dest->index);
    }

    bool operator!=(const Quad &rhs) const {
        return !(rhs == *this);
    }
};


#endif //TAC_PARSER_QUADRUPLE_HPP


