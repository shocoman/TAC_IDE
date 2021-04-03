//
// Created by shoco on 3/14/2021.
//

#include "program.hpp"

std::string Program::get_as_code() const {
    std::string code;
    for (auto &f : functions) {
        code += f.get_as_code();
        code += '\n';
    }
    return code;
}

Function *Program::get_function_by_name(const std::string &name) {
    for (auto &f : functions)
        if (f.function_name == name)
            return &f;
    return nullptr;
}

std::vector<std::string> Program::get_function_names() {
    std::vector<std::string> names;
    for (auto &f : functions)
        names.push_back(f.function_name);
    return names;
}

Program Program::from_program_code(std::string program_code) {
    ParseDriver drv;
    int res = drv.parse_from_string(program_code);
    if (res) {
        throw yy::Parser::syntax_error(drv.location, g_bison_error_msg);
    }

    return Program{collect_quads_into_functions(drv.labels, drv.quadruples)};
}

Program Program::from_file(std::string file_path) {
    ParseDriver drv;
    int res = drv.parse_from_file(file_path);
    if (res) {
        throw yy::Parser::syntax_error(drv.location, g_bison_error_msg);
    }
    return Program{collect_quads_into_functions(drv.labels, drv.quadruples)};
}
