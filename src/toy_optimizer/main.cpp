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

    if (getenv("DISPLAY") == nullptr)
        setenv("DISPLAY", "192.168.194.241:0", true);

    ParseDriver drv;

    int res =
        //    drv.parse_from_file("../../../_Examples/Toy/myfile")
        //    drv.parse_from_file("../../../_Examples/Toy/reach_def_test.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/lazy_code_motion.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/anticipable_expressions.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/anticipable_expressions2.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/available_expressions3.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/FactorialProgram.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/ssa_test.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/sccp_test.txt")
//              drv.parse_from_file("../../../_Examples/Toy/new_sccp_test.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/sccp2.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/strength_reduction.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/copy_propagation.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/bob_maxcol.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/live_test.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/ref_test.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/copy_propagation2.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/ssa_swap_problem.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/ssa_lost_copy_problem.txt")
        //    drv.parse_from_file("../../../_Examples/Toy/value_numbering.txt")
//            drv.parse_from_file("../../../_Examples/Toy/Examples/TempIdents.txt")
            drv.parse_from_file("../../../_Examples/edu_test.txt")
        ;

    auto functions = collect_quads_into_functions(drv.labels, drv.quadruples);
    auto &f = functions[0];
    //    run_convert_to_ssa(f);
    //    run_convert_from_ssa(f);

    f.print_cfg("before.png");
//    run_convert_to_ssa(f);
//    run_convert_from_ssa(f);
//    f.print_cfg("after.png");

    //    superlocal_value_numbering(f);
    //    dominator_based_value_numbering(f);
    //    run_global_value_numbering(f);
    //    f.print_cfg("after.png");

    //    run_useless_code_elimination(f);
    //    ConvertToSSADriver convert_to_ssa_driver(f);

    //    run_convert_to_ssa(f);
    //    f.print_cfg("before.png");
    //
    //    run_convert_from_ssa(f);
    //    run_copy_propagation(f);
    //    run_useless_code_elimination(f);
    //
    //    f.print_cfg("after.png");

    std::getchar();
    return 0;
}
