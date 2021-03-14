//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_OPERAND_HPP
#define TAC_PARSER_OPERAND_HPP

struct BasicBlock;
struct Operand {
    enum class Type { None, Var, LInt, LDouble, LBool, LChar, LString };

    std::string value;
    Type type;
    BasicBlock *phi_predecessor = nullptr; // for SSA (phi node)

    Operand() : type(Type::None) {}

    Operand(std::string s, Type t, BasicBlock *phi_pred = nullptr)
        : value(std::move(s)), type(t), phi_predecessor(phi_pred) {}

    Operand(const std::string &s) {
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
        if (type == Type::LChar)
            return "'" + value + "'";
        else
            return value;
    }

    std::optional<double> as_double() const {
        switch (type) {
        case Type::LChar:
            return (double)value[0];
        case Type::LBool:
            if (value == "true")
                return 1.0;
            else if (value == "false")
                return 0.0;
        case Type::LInt:
        case Type::LDouble: {
            char *end_ptr = nullptr;
            double n = strtod(value.c_str(), &end_ptr);
            return (*end_ptr == '\0') ? std::optional(n) : std::nullopt;
        }
        case Type::None:
        case Type::LString:
        case Type::Var:
        default:
            return std::nullopt;
        }
    }

    int get_int() const { return (int)as_double().value(); }

    double get_double() const { return as_double().value(); }

    bool is_var() const { return type == Type::Var; }

    bool is_constant() const { return !(is_var() || type == Type::None); }

    bool is_int() const { return type == Type::LInt; }

    bool is_double() const { return type == Type::LDouble; }

    bool is_number() const { return type == Type::LInt || type == Type::LDouble; }

    bool is_true() const {
        auto d = as_double();
        return d.has_value() && *d > 0.0 ? true : false;
    }

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

    bool operator<(const Operand &rhs) const {
        if (is_number() && rhs.is_number())
            return get_double() < rhs.get_double();
        else
            return value < rhs.value;
    }
};

#endif // TAC_PARSER_OPERAND_HPP
