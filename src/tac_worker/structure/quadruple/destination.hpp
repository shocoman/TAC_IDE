//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_DESTINATION_HPP
#define TAC_PARSER_DESTINATION_HPP

#include "operand.hpp"

struct Dest {
    enum class Type {
        None,
        Var,
        ArraySet,
        Deref,
        JumpLabel,
    };

    Type type{};
    std::string name;
    std::optional<Operand> index;

    Dest(std::string dest_name, Operand element_name, Type dest_type)
        : name(std::move(dest_name)), index(std::move(element_name)), type(dest_type) {}

    Dest() = default;

    friend std::ostream &operator<<(std::ostream &os, const Dest &destination) {
        const char *type_names[] = {"None", "Var", "ArraySet", "Deref", "JumpLabel"};
        os << "type: " << type_names[static_cast<int>(destination.type)]
           << "; name: " << destination.name;
        if (destination.index.has_value())
            os << "; index: " << destination.index.value().value;
        return os;
    }

    std::optional<std::string> fmt() const {
        switch (type) {
        case Type::Var:
            return name;
        case Type::ArraySet:
            return name + "[" + index.value().value + "]";
        case Type::Deref:
            return "*" + name;
        case Type::JumpLabel:
            return name;
        default:
            return {};
        }
    }
};

#endif // TAC_PARSER_DESTINATION_HPP
