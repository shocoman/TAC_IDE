#include <iostream>
#include <unordered_set>

#include "fmt/ranges.h"

#include "tac_worker/optimizations/operator_strength_reduction.hpp"
#include "src/parser/driver/driver.hpp"
#include "tac_worker/optimization_runner.hpp"
#include "tac_worker/optimizations/data_flow_analyses/data_flow_analyses.hpp"
#include "tac_worker/optimizations/data_flow_analyses/data_flow_framework.hpp"
#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/anticipable_expressions.hpp"
#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/available_expressions.hpp"
#include "tac_worker/optimizations/lazy_code_motion.hpp"
#include "tac_worker/optimizations/sparse_conditional_constant_propagation.hpp"
#include "tac_worker/print_utility.hpp"

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

    setenv("DISPLAY", "192.168.72.177:0", true);
    ParseDriver drv;


    //    drv.parse("../_TestCode/myfile");
    //    drv.parse("../_TestCode/reach_def_test.txt");
    //    drv.parse("../_TestCode/lazy_code_motion.txt");
    //    drv.parse("../_TestCode/anticipable_expressions.txt");
    //    drv.parse("../_TestCode/anticipable_expressions2.txt");
    //    drv.parse("../_TestCode/available_expressions3.txt");
    //    drv.parse("../_TestCode/FactorialProgram.txt");
    //    drv.parse("../_TestCode/ssa_test.txt");
    //    drv.parse("../_TestCode/sccp_test.txt");
    drv.parse("../_TestCode/strength_reduction.txt");

    auto functions = collect_quads_into_functions(drv.labels, drv.quadruples);
    auto &f = functions[0];
    //    optimize(functions[0]);

    convert_to_ssa(f);
//    f.print_cfg("before.png");

    auto operator_reduction = OSRDriver(f);

    //    sparse_conditional_constant_propagation(f);
    f.print_cfg("after.png");

    std::getchar();
    return 0;
}
