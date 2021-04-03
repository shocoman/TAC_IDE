//
// Created by shoco on 3/14/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_STRUCTURE_PROGRAM_HPP
#define TAC_PARSER_SRC_TAC_WORKER_STRUCTURE_PROGRAM_HPP

#include <sstream>
#include "../utilities/parser/driver/driver.hpp"
#include "function.hpp"
#include "../utilities/quad_preparation.hpp"

struct Program {
    std::vector<Function> functions;

    static Program from_file(std::string file_path);
    static Program from_program_code(std::string program_code);
    std::vector<std::string> get_function_names();
    Function* get_function_by_name(const std::string &name);
    std::string get_as_code() const;
};

#endif // TAC_PARSER_SRC_TAC_WORKER_STRUCTURE_PROGRAM_HPP
