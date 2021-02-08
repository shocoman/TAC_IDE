//
// Created by shoco on 2/8/2021.
//

#include "print_utility.hpp"

std::ostream &PrintAll::operator<<(std::ostream &stream, const std::string &str) {
    stream << str.c_str();
    return stream;
}
