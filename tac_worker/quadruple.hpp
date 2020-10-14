#ifndef TAC_PARSER_QUADRUPLE_HPP
#define TAC_PARSER_QUADRUPLE_HPP

#include <string>
#include <ostream>
#include <utility>
#include <vector>
#include <optional>
#include <iostream>

enum class OperationType {
    Nop, Add, Sub, Mult, Div, UMinus,
    Lt, Gt, Eq, Neq,
    Copy, Deref, Ref, ArrayGet,
    IfTrue, IfFalse, Goto, Halt, Call, Param, Return,
};


enum class DestinationType {
    None, Var, ArraySet, Deref, JumpLabel,
};

struct Destination {
    DestinationType dest_type{};
    std::string dest_name;
    std::optional<std::string> element_name;

    Destination(std::string dest_name, std::optional<std::string> element_name, DestinationType dest_type)
            : dest_name(std::move(dest_name)), element_name(std::move(element_name)), dest_type(dest_type) {}

    Destination() = default;

    friend std::ostream &operator<<(std::ostream &os, const Destination &destination) {
        const char *type_names[] = {"None", "Var", "ArraySet", "Deref", "JumpLabel"};
        os << "dest_type: " << type_names[static_cast<int>(destination.dest_type)]
           << "; dest_name: " << destination.dest_name;
        if (destination.element_name.has_value()) os << "; element_name: " << destination.element_name.value();
        return os;
    }

    std::optional<std::string> fmt() const {
        switch (dest_type) {
            case DestinationType::Var:
                return dest_name;
            case DestinationType::ArraySet:
                return dest_name + "[" + element_name.value() + "]";
            case DestinationType::Deref:
                return "*" + dest_name;
            case DestinationType::JumpLabel:
                return dest_name;
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
    Type type;

    Operand() : type(Type::None) {}

    Operand(std::string s, Type t) : value(std::move(s)), type(t) {}

    Operand(std::string &&s) {
        char *end = nullptr;
        if (long i = strtol(s.c_str(), &end, 10); end != s.c_str() && *end == '\0') {
            type = Type::LInt;
        } else if (double d = strtod(s.c_str(), &end); end != s.c_str() && *end == '\0') {
            type = Type::LDouble;
        } else if (!s.empty()) {
            type = Type::Var;
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

    bool is_var() { return type == Type::Var; }

    bool is_int() { return type == Type::LInt; }

    bool is_double() { return type == Type::LDouble; }

    bool is_number() {
        return type == Type::LInt || type == Type::LDouble;
    }
};

struct Quadruple {

    std::optional<Destination> dest{};
    Operand operand_1{};
    Operand operand_2{};
    OperationType operation{};

    Quadruple(std::string op1, std::string op2, OperationType op_type)
            : operand_1(std::move(op1)), operand_2(std::move((op2))), operation(op_type), dest({}) {}

    Quadruple() = default;


    std::vector<std::string> get_used_vars() {
        auto used_vars = get_rhs();
        if (auto l = get_lhs(); l.has_value()) {
            used_vars.push_back(l.value());
        }
        return used_vars;
    }

    std::optional<std::string> get_lhs() {
        if (dest && !is_jump()) {
            return (dest.value().dest_name);
        } else {
            return {};
        }
    }

    std::vector<std::string> get_rhs() {
        std::vector<std::string> rhs_vars;
        if (dest) {
            if (dest.value().dest_type == DestinationType::ArraySet)
                rhs_vars.push_back(dest.value().element_name.value());
        }
        if (operand_1.is_var()) rhs_vars.push_back(operand_1.get_string());
        if (operand_2.is_var()) rhs_vars.push_back(operand_2.get_string());
        return rhs_vars;
    }

    bool is_jump() {
        return operation == OperationType::Goto
               || operation == OperationType::IfTrue
               || operation == OperationType::IfFalse;
    }


    friend std::ostream &operator<<(std::ostream &os, const Quadruple &quadruple) {
        const char *type_names[] = {"Nop", "Add", "Sub", "Mult", "Div", "UMinus", "Lt", "Gt", "Eq", "Neq", "Copy",
                                    "Deref", "Ref", "ArrayGet", "IfTrue", "IfFalse", "Goto", "Halt", "Call", "Param",
                                    "Return"};
        if (quadruple.dest.has_value()) os << "dest: { " << quadruple.dest.value() << "}" << "; ";
        if (quadruple.operand_1.type != Operand::Type::None)
            os << "operand_1: " << quadruple.operand_1.get_string() << "; ";
        if (quadruple.operand_2.type != Operand::Type::None)
            os << "operand_2: " << quadruple.operand_2.get_string() << "; ";
        os << "operation: " << type_names[static_cast<int>(quadruple.operation)];
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
        switch (operation) {
            case OperationType::Add:
                op = "+";
                break;
            case OperationType::Sub:
                op = "-";
                break;
            case OperationType::Mult:
                op = "*";
                break;
            case OperationType::Div:
                op = "/";
                break;
            case OperationType::UMinus:
                unary_op = "-";
                break;
            case OperationType::Lt:
                op = "<";
                break;
            case OperationType::Gt:
                op = ">";
                break;
            case OperationType::Eq:
                op = "==";
                break;
            case OperationType::Neq:
                op = "!=";
                break;
            case OperationType::Copy:
                unary_op = "";
                break;
            case OperationType::Deref:
                unary_op = "*";
                break;
            case OperationType::Ref:
                unary_op = "&";
                break;
            case OperationType::ArrayGet:
                return destination.value() + " = " + operand_1.get_string() + "[" + operand_2.get_string() + "]";
            case OperationType::IfTrue:
                return "if " + operand_1.get_string() + " goto " + destination.value();
            case OperationType::IfFalse:
                return "ifFalse " + operand_1.get_string() + " goto " + destination.value();
            case OperationType::Goto:
                return "goto " + destination.value();
            case OperationType::Halt:
                return "halt";
            case OperationType::Call:
                return "call " + operand_1.get_string() + " " + operand_2.get_string();
            case OperationType::Param:
                return "param " + operand_1.get_string();
            case OperationType::Nop:
                return "nop";
            case OperationType::Return:
                return "return";

        }

        if (unary_op.has_value()) {
            return destination.value() + " = " + unary_op.value() + operand_1.get_string();
        } else {
            return destination.value() + " = " + operand_1.get_string() + op.value() + operand_2.get_string();
        }
    }
};


#endif //TAC_PARSER_QUADRUPLE_HPP


