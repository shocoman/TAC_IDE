#include <iostream>
#include <numeric>
#include <unordered_set>

#include "src/parser/driver/driver.hpp"
#include "tac_worker/optimization_runner.hpp"
#include "tac_worker/quad_preparation.hpp"
#include "tac_worker/structure/function.hpp"
#include <queue>

int main(int argc, char *argv[]) {
    setenv("DISPLAY", "172.17.53.113:0", true);

    ParseDriver drv;

    //    for (int i = 1; i < argc; ++i)
    //        if (argv[i] == std::string("-p"))
    //            drv.trace_parsing = true;
    //        else if (argv[i] == std::string("-s"))
    //            drv.trace_scanning = true;
    //        else if (!drv.parse(argv[i]))
    //            std::cout << "Parsing result: " << drv.result << '\n';
    //        else
    //            res = 1;

    // drv.parse("../_TestCode/myfile");
//     drv.parse("../_TestCode/myfile2");
    // drv.parse("../_TestCode/reach_def_test.txt");
    //       drv.parse("../_TestCode/lazy_code_motion.txt");
    drv.parse("../_TestCode/available_expressions.txt");
    // drv.parse("../_TestCode/anticipate_expression.txt");
    // drv.parse("../_TestCode/FactorialProgram.txt");

    auto functions = collect_quads_into_functions(drv.labels, drv.quadruples);
//    optimize(functions[0]);
    functions[0].print_cfg("before.png");
    available_expressions(functions[0]);

    std::getchar();
    return 0;
}
