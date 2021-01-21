
#include "driver.hpp"

ParseDriver::ParseDriver(bool trace_parsing, bool trace_scanning) {
    this->trace_parsing = trace_parsing;
    this->trace_scanning = trace_scanning;
}

int ParseDriver::parse(const std::string& f) {
    file = f;
    location.initialize(&file);
    scan_begin();
    yy::parser parse(*this);
    parse.set_debug_level(trace_parsing);
    int res = parse();
    scan_end();
    return res;
}

