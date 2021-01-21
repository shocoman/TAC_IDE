#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <string>
#include <map>
#include <utility>
#include "parser.hpp"

#define YY_DECL \
    yy::parser::symbol_type yylex (ParseDriver & drv)
YY_DECL;

struct Quad;

struct ParseDriver {
    ParseDriver(bool trace_parsing = false, bool trace_scanning = false);

    std::map<std::string, int> variables;
    std::map<std::string, int> labels;
    std::vector<Quad> quadruples;

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