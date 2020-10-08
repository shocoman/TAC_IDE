#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <string>
#include <map>
#include <utility>
#include "parser.hpp"

#define YY_DECL \
    yy::parser::symbol_type yylex (driver& drv)
YY_DECL;

struct Quadruple;

struct driver {
    driver();

    std::map<std::string, int> variables;
    std::map<std::string, int> labels;
    std::vector<Quadruple> quadruples;

    int result;

    int parse(const std::string& f);
    std::string file;
    bool trace_parsing;

    void scan_begin();
    void scan_end();
    bool trace_scanning;
    yy::location location;
};




#endif // ! DRIVER_HPP