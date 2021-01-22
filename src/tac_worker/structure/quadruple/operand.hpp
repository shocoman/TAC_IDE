//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_OPERAND_HPP
#define TAC_PARSER_OPERAND_HPP

struct Operand {
    enum class Type { None, Var, LInt, LDouble, LBool, LChar, LString };

    std::string value;
    Type type;
    int predecessor_id = -1; // for SSA (phi node)

    Operand() : type(Type::None) {}

    Operand(std::string s, Type t) : value(std::move(s)), type(t) {}

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

    bool operator<(const Operand &rhs) const {
        if (is_number() && rhs.is_number())
            return get_double() < rhs.get_double();
        else
            return value < rhs.value;
    }
};

#endif // TAC_PARSER_OPERAND_HPP
