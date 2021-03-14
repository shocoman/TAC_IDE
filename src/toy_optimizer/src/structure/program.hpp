//
// Created by shoco on 3/14/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_STRUCTURE_PROGRAM_HPP
#define TAC_PARSER_SRC_TAC_WORKER_STRUCTURE_PROGRAM_HPP

#include "../utilities/parser/driver/driver.hpp"
#include "function.hpp"
#include "../utilities/quad_preparation.hpp"

struct Program {
    std::vector<Function> functions;

    static Program from_file(std::string file_path) {
        ParseDriver drv;
        drv.parse_from_file(file_path);
        return Program{collect_quads_into_functions(drv.labels, drv.quadruples)};
    }

    static Program from_program_code(std::string program_code) {
        ParseDriver drv;
        drv.parse_from_string(program_code);
        return Program{collect_quads_into_functions(drv.labels, drv.quadruples)};
    }

    std::vector<std::string> get_function_names() {
        std::vector<std::string> names;
        for (auto &f : functions)
            names.push_back(f.function_name);
        return names;
    }

    Function* get_function_by_name(const std::string &name) {
        for (auto &f : functions)
            if (f.function_name == name)
                return &f;
        return nullptr;
    }
};

#endif // TAC_PARSER_SRC_TAC_WORKER_STRUCTURE_PROGRAM_HPP
