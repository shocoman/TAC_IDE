#include <iostream>
#include <unordered_set>

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
    //    functions[0].print_cfg("before.png");
    //    anticipable_expressions(functions[0]);
    //    available_expressions(functions[0]);

    //    earliest_expressions(functions[0]);
    //    later_placement_expressions(functions[0]);

    {
        auto all_expressions = get_all_expressions_set(functions[0]);

        auto [id_to_de_exprs, id_to_killed_exprs] =
            get_downward_exposed_and_killed_expressions(functions[0]);

        auto [IN, OUT] = data_flow_framework<Expression>(
            functions[0], Flow::Forwards, Meet::Intersection, all_expressions,
            [&id_to_killed_exprs, &id_to_de_exprs](ID2EXPRS &IN, ID2EXPRS &OUT, int id) {
                auto X = IN.at(id);
                for (auto &expr : id_to_killed_exprs.at(id))
                    X.erase(expr);
                for (auto &expr : id_to_de_exprs.at(id))
                    X.insert(expr);
                return X;
            });

        // region Available Expressions Print
        std::cout << "Available Expressions" << std::endl;
        for (auto &[id, in] : IN) {
            std::cout << functions[0].basic_blocks.at(id)->get_name() << std::endl;
            std::cout << "\tIN: " + print_into_string_with(in, print_expression) << std::endl;
            std::cout << "\tOUT: " + print_into_string_with(OUT.at(id), print_expression) << std::endl;
        }
        // endregion
        // region Print CFG
        std::unordered_map<int, std::string> above, below;
        for (auto &[id, in] : IN) {
            above.emplace(id, "IN: " + print_into_string_with(in, print_expression));
            below.emplace(id, "OUT: " + print_into_string_with(OUT.at(id), print_expression));
        }
        std::string title = "Available Expressions V2<BR/>";
        title += "All Expressions: " + print_into_string_with(all_expressions, print_expression);
        functions[0].print_cfg("lala.png", above, below, title);
        // endregion
    }

    available_expressions(functions[0]);

    std::getchar();
    return 0;
}
