#include <iostream>
#include <unordered_set>

#include "tac_worker/optimizations/lazy_code_motion.hpp"
#include "src/parser/driver/driver.hpp"
#include "tac_worker/optimization_runner.hpp"
#include "tac_worker/optimizations/data_flow_analyses/data_flow_analyses.hpp"
#include "tac_worker/optimizations/data_flow_analyses/data_flow_framework.hpp"
#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/anticipable_expressions.hpp"
#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/available_expressions.hpp"
#include "tac_worker/print_utility.hpp"

int main(int argc, char *argv[]) {
    setenv("DISPLAY", "192.168.55.241:0", true);

    ParseDriver drv;

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

    // drv.parse("../_TestCode/myfile");
    //     drv.parse("../_TestCode/myfile2");
    // drv.parse("../_TestCode/reach_def_test.txt");
    //    drv.parse("../_TestCode/lazy_code_motion.txt");
    //         drv.parse("../_TestCode/anticipable_expressions.txt");
    drv.parse("../_TestCode/anticipable_expressions2.txt");
    //    drv.parse("../_TestCode/available_expressions3.txt");
    //     drv.parse("../_TestCode/FactorialProgram.txt");

    auto functions = collect_quads_into_functions(drv.labels, drv.quadruples);
    //    optimize(functions[0]);

    //    functions[0].id_to_block.at(2)->successors.insert(functions[0].id_to_block.at(6));
    functions[0].print_cfg("before.png");
    //    anticipable_expressions(functions[0]);
    //    available_expressions(functions[0]);

    //    earliest_expressions(functions[0]);
    //    later_placement_expressions(functions[0]);

    //    anticipable_expressions(functions[0]);
    //    AnticipableExpressions(functions[0]);
    //    available_expressions(functions[0]);
    //    AvailableExpressions(functions[0]);
    //    AvailableExpressions2(functions[0]);
    //    PostponableExpressions(functions[0]);
    lazy_code_motion(functions[0]);

    functions[0].print_cfg("after.png");

    std::getchar();
    return 0;
}
