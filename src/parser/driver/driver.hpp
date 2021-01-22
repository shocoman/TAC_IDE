#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <string>
#include <unordered_map>
#include <utility>
#include "../compiled_parser/parser.hpp"


#define YY_DECL \
    yy::parser::symbol_type yylex (ParseDriver & drv)
YY_DECL;

struct ParseDriver {
    ParseDriver(bool trace_parsing = false, bool trace_scanning = false);

    std::unordered_map<std::string, int> labels;
    std::vector<Quad> quadruples;

    int parse(const std::string& f);
    std::string file;
    bool trace_parsing;

    void scan_begin();
    void scan_end();
    bool trace_scanning;
    yy::location location;
};

#endif // ! DRIVER_HPP