//
// Created by shoco on 3/11/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_UTILITIES_NEW_NAME_GENERATOR_HPP
#define TAC_PARSER_SRC_TAC_WORKER_UTILITIES_NEW_NAME_GENERATOR_HPP

#include "../structure/function.hpp"

class NewNameGenerator {
    Function &f;
    std::unordered_set<std::string> used_names;
    int name_counter;

  public:
    NewNameGenerator() = delete;
    NewNameGenerator(Function &_f) : f(_f), name_counter(0) {
        update_used_names();
    }

    void update_used_names() {
        for (auto &b : f.basic_blocks) {
            for (auto &q : b->quads) {
                auto rhs = q.get_rhs(false);
                used_names.insert( rhs.begin(), rhs.end() );
                if (q.dest.has_value())
                    used_names.insert(q.dest->name);
            }
        }
    }

    std::string make_new_name() {
        std::string new_name;
        do {
            new_name = fmt::format("$t{}", name_counter);
            ++name_counter;
        } while (used_names.count(new_name) > 0);
        used_names.insert(new_name);
        return new_name;
    }

};

#endif // TAC_PARSER_SRC_TAC_WORKER_UTILITIES_NEW_NAME_GENERATOR_HPP
