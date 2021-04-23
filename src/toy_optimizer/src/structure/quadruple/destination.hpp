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
        Array,
        Deref,
        JumpLabel,
    };

    Type type{};
    std::string name;

    Dest(std::string dest_name, Type dest_type) : name(std::move(dest_name)), type(dest_type) {}

    Dest() = default;

    std::optional<std::string> fmt() const {
        switch (type) {
        case Type::Var:
            return name;
        case Type::Array:
            return name;
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
