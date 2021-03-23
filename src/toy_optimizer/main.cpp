#include <any>
#include <iostream>
#include <optimizations/ssa.hpp>
#include <queue>
#include <unordered_set>

#include "fmt/ranges.h"

#include "all_headers.hpp"

#include "structure/program.hpp"
#include "utilities/parser/driver/driver.hpp"

int main(int argc, char *argv[]) {

    // region CmdArg parse
    //    for (int i = 1; i < argc; ++i)
    //        if (argv[i] == std::string("-p"))
    //            drv.trace_parsing = true;
    //        else if (argv[i] == std::string("-s"))
    //            drv.trace_scanning = true;
    //        else if (!drv.parse(argv[i]))
    //            std::cout << "Parsing result: " << drv.result << '\n';
    //        else
    //            res = 1;
    // endregion

    setenv("DISPLAY", "192.168.205.225:0", true);
    ParseDriver drv;

    //    drv.parse_from_file("../_TestCode/myfile");
    //    drv.parse_from_file("../_TestCode/reach_def_test.txt");
    //    drv.parse_from_file("../_TestCode/lazy_code_motion.txt");
    //    drv.parse_from_file("../_TestCode/anticipable_expressions.txt");
    //    drv.parse_from_file("../_TestCode/anticipable_expressions2.txt");
    //    drv.parse_from_file("../_TestCode/available_expressions3.txt");
    drv.parse_from_file("../_TestCode/FactorialProgram.txt");
    //    drv.parse_from_file("../_TestCode/ssa_test.txt");
    //    drv.parse_from_file("../_TestCode/sccp_test.txt");
    //    drv.parse_from_file("../_TestCode/sccp2.txt");
    //    drv.parse_from_file("../_TestCode/strength_reduction.txt");
    //    drv.parse_from_file("../_TestCode/copy_propagation.txt");
    //    drv.parse_from_file("../_TestCode/bob_maxcol.txt");
    //    drv.parse_from_file("../_TestCode/live_test.txt");
    //    drv.parse_from_file("../_TestCode/ref_test.txt");
    //    drv.parse_from_file("../_TestCode/copy_propagation2.txt");
    //    drv.parse_from_file("../_TestCode/ssa_swap_problem.txt");
    //    drv.parse_from_file("../_TestCode/ssa_lost_copy_problem.txt");

    auto functions = collect_quads_into_functions(drv.labels, drv.quadruples);
    auto &f = functions[1];

    f.print_cfg("before.png");

    std::getchar();
    return 0;
}
