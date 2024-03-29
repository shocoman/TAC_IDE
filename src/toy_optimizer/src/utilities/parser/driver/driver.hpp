#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "../compiled_parser/parser.hpp"
#include <string>
#include <unordered_map>
#include <utility>

#define YY_DECL yy::Parser::symbol_type yylex(ParseDriver &drv)
YY_DECL;

extern std::string g_bison_error_msg;

struct ParseDriver {

    std::string file_name;
    bool trace_parsing, trace_scanning;
    yy::location location;

    std::unordered_map<std::string, int> labels;
    std::vector<Quad> quadruples;

    explicit ParseDriver(bool trace_parsing = false, bool trace_scanning = false);

    int parse_from_file(const std::string &f);
    int parse_from_string(const std::string &str);

    void scan_from_file_begin();
    void scan_from_string_begin(const std::string &str);
    void scan_end();
};

#endif // ! DRIVER_HPP