#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <string>
#include <map>
#include "../parser.hpp"

#define YY_DECL \
    yy::parser::symbol_type yylex (driver& drv)
YY_DECL;


class driver {
public:
    driver();

    std::map<std::string, int> variables;

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