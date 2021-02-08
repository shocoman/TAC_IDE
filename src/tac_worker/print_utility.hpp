//
// Created by shoco on 2/8/2021.
//

#ifndef TAC_PARSER_PRINT_UTILITY_HPP
#define TAC_PARSER_PRINT_UTILITY_HPP

#include <ostream>
#include <sstream>
#include <string>

namespace PrintAll {
template <template <typename... Args> class ContainerT, typename... Args>
std::ostream &operator<<(std::ostream &stream, const ContainerT<Args...> &container) {
    stream << "{";
    for (auto it = container.begin(); it != container.end(); it++) {
        if (it != container.begin())
            stream << ", ";
        stream << *it;
    }
    stream << "}";
    return stream;
}

template <template <typename T, size_t N, typename... Args> class ContainerT, typename T, size_t N,
          typename... Args>
std::ostream &operator<<(std::ostream &stream, const ContainerT<T, N, Args...> &container) {
    stream << "{ ";
    for (auto it = container.begin(); it != container.end(); it++) {
        if (it != container.begin())
            stream << ", ";
        stream << *it;
    }
    stream << "}";
    return stream;
}

template <typename T1, typename T2>
std::ostream &operator<<(std::ostream &stream, const std::pair<T1, T2> &val) {
    stream << "{" << val.first << ", " << val.second << "}";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const std::string &str);
} // namespace PrintAll


template <template <typename...> class ContainerT, typename... Args>
std::string print_into_string(const ContainerT<Args...> &container) {
    std::ostringstream stream;
    PrintAll::operator<<(stream, container);
    return stream.str();
}


template <template <typename... > class ContainerT, typename F, typename... Args>
std::string print_into_string_with(const ContainerT<Args...> &container, F func) {
    std::ostringstream stream;
    stream << "{";
    for (auto it = container.begin(); it != container.end(); it++) {
        if (it != container.begin())
            stream << ", ";
        PrintAll::operator<<(stream, func(*it));
    }
    stream << "}";
    return stream.str();
}

#endif // TAC_PARSER_PRINT_UTILITY_HPP
