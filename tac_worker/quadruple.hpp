#ifndef TAC_PARSER_QUADRUPLE_HPP
#define TAC_PARSER_QUADRUPLE_HPP

#include <string>
#include <ostream>
#include <utility>
#include <vector>
#include <variant>
#include <optional>

enum class OperationType {
    Nop, Add, Sub, Mult, Div, UMinus,
    Lt, Gt, Eq, Neq,
    Copy, Deref, ArrayGet,
    IfTrue, IfFalse, Goto, Halt, Call, Param,
};

enum class DestinationType {
    None, Var, ArraySet, Deref, JumpLabel,
};

struct Destination {
    DestinationType dest_type;
    std::string dest_name;
    std::optional<std::string> element_name;

    Destination(std::string dest_name, std::optional<std::string> element_name, DestinationType dest_type)
            : dest_name(std::move(dest_name)), element_name(std::move(element_name)), dest_type(dest_type) {}

    Destination() = default;

    friend std::ostream &operator<<(std::ostream &os, const Destination &destination) {
        const char* type_names[] = { "None", "Var", "ArraySet", "Deref", "JumpLabel" };
        os  << "dest_type: " << type_names[static_cast<int>(destination.dest_type)]
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

struct Quadruple {
    std::optional<Destination> dest;
    std::optional<std::string> operand_1;
    std::optional<std::string> operand_2;
    OperationType operation;

    Quadruple(std::optional<std::string> op1, std::optional<std::string> op2, OperationType op_type)
            : operand_1(std::move(op1)), operand_2(std::move((op2))), operation(op_type), dest({}) {}

    Quadruple() = default;

    friend std::ostream &operator<<(std::ostream &os, const Quadruple &quadruple) {
        const char* type_names[] = { "Nop", "Add", "Sub", "Mult", "Div", "UMinus", "Lt", "Gt", "Eq", "Neq", "Copy", "Deref", "ArrayGet", "IfTrue", "IfFalse", "Goto", "Halt", "Call", "Param", };
//        os  << "dest: { " << quadruple.dest.value() << "}"
//            << "; operand_1: " << quadruple.operand_1.value()
//            << "; operand_2: " << quadruple.operand_2.value()
//            << "; operation: " << type_names[static_cast<int>(quadruple.operation)];
        if (quadruple.dest.has_value())      os  << "dest: { " << quadruple.dest.value() << "}" << "; ";
        if (quadruple.operand_1.has_value()) os  << "operand_1: " << quadruple.operand_1.value() << "; ";
        if (quadruple.operand_2.has_value()) os  << "operand_2: " << quadruple.operand_2.value() << "; ";
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
            case OperationType::Add: op = "+";
                break;
            case OperationType::Sub: op = "-";
                break;
            case OperationType::Mult: op = "*";
                break;
            case OperationType::Div: op = "/";
                break;
            case OperationType::UMinus: unary_op = "-";
                break;
            case OperationType::Lt:     op = "<";
                break;
            case OperationType::Gt:     op = ">";
                break;
            case OperationType::Eq:     op = "==";
                break;
            case OperationType::Neq:    op = "!=";
                break;
            case OperationType::Copy:   unary_op = "";
                break;
            case OperationType::Deref:  unary_op = "*";
                break;
            case OperationType::ArrayGet: return destination.value() + " = " + operand_1.value() + "[" + operand_2.value() + "]";
            case OperationType::IfTrue: return "if " + operand_1.value() + " goto " + destination.value();
            case OperationType::IfFalse: return "ifFalse " + operand_1.value() + " goto " + destination.value();
            case OperationType::Goto: return "goto " + destination.value();
            case OperationType::Halt: return "halt";
            case OperationType::Call: return "call " + operand_1.value() + " " + operand_2.value();
            case OperationType::Param: return "param " + operand_1.value();
            case OperationType::Nop: return "nop";
        }

        if (unary_op.has_value()) {
            return destination.value() + " = " + unary_op.value() + operand_1.value();
        } else {
            return destination.value() + " = " + operand_1.value() + op.value() + operand_2.value();
        }
    }
};


#endif //TAC_PARSER_QUADRUPLE_HPP


