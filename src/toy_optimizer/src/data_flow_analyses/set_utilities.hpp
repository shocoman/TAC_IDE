//
// Created by shoco on 2/5/2021.
//

#ifndef TAC_PARSER_UTILITIES_HPP
#define TAC_PARSER_UTILITIES_HPP

#include <algorithm>
#include <fmt/ranges.h>
#include <iterator>
#include <set>
#include <sstream>
#include <unordered_set>
#include <vector>

template <typename T> std::set<T> intersection_of_sets(std::vector<std::set<T>> sets) {
    if (sets.empty()) {
        return {};
    } else {
        auto base = sets[0];
        for (int i = 1; i < sets.size(); i++) {
            std::set<T> buffer;
            std::set_intersection(base.begin(), base.end(), sets[i].begin(), sets[i].end(),
                                  std::inserter(buffer, buffer.end()));
            base = std::move(buffer);
        }
        return base;
    }
}
template <typename T> std::set<T> union_of_sets(std::vector<std::set<T>> sets) {
    if (sets.empty()) {
        return {};
    } else {
        auto base = sets[0];
        for (int i = 1; i < sets.size(); i++) {
            std::set<T> buffer;
            std::set_union(base.begin(), base.end(), sets[i].begin(), sets[i].end(),
                           std::inserter(buffer, buffer.end()));
            base = std::move(buffer);
        }
        return base;
    }
}
template <typename T>
std::unordered_set<T> intersection_of_sets(std::vector<std::unordered_set<T>> sets) {
    if (sets.empty()) {
        return {};
    } else {
        auto result = sets[0];
        for (int i = 1; i < sets.size(); i++) {
            auto copy = result;
            for (auto &el : copy)
                if (sets[i].count(el) == 0)
                    result.erase(el);
        }
        return result;
    }
}
template <typename T> std::unordered_set<T> union_of_sets(std::vector<std::unordered_set<T>> sets) {
    if (sets.empty()) {
        return {};
    } else {
        auto result = sets[0];
        for (int i = 1; i < sets.size(); i++)
            for (auto &el : sets[i])
                result.insert(el);
        return result;
    }
}

template <template <typename...> class ContainerT, typename F, typename... Args>
std::string print_into_string_with(const ContainerT<Args...> &container, F func) {
    std::ostringstream stream;
    stream << "{";
    for (auto it = container.begin(); it != container.end(); it++) {
        if (it != container.begin())
            stream << ", ";
        stream << fmt::format("{}", func(*it));
    }
    stream << "}";

    return stream.str();
}

#endif // TAC_PARSER_UTILITIES_HPP
