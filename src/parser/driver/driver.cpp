
#include "driver.hpp"

ParseDriver::ParseDriver(bool _trace_parsing, bool _trace_scanning)
    : trace_parsing(_trace_parsing), trace_scanning(_trace_scanning) {}

int ParseDriver::parse_from_file(const std::string &f) {
    file_name = f;
    location.initialize(&file_name);
    scan_from_file_begin();
    yy::parser bison_parser(*this);
    bison_parser.set_debug_level(trace_parsing);
    int res = bison_parser();
    scan_end();
    return res;
}

int ParseDriver::parse_from_string(const std::string &str) {
    file_name = "SCAN_FROM_STRING";
    location.initialize(&file_name);
    scan_from_string_begin(str);
    yy::parser bison_parser(*this);
    bison_parser.set_debug_level(trace_parsing);
    int res = bison_parser();
    return res;
}
