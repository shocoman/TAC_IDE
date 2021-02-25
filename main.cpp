#include <iostream>
#include <unordered_set>

#include "fmt/ranges.h"

#include "src/parser/driver/driver.hpp"
#include "tac_worker/optimization_runner.hpp"
#include "tac_worker/optimizations/data_flow_analyses/data_flow_analyses.hpp"
#include "tac_worker/optimizations/data_flow_analyses/data_flow_framework.hpp"
#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/anticipable_expressions.hpp"
#include "tac_worker/optimizations/data_flow_analyses/expressions_analyses/available_expressions.hpp"
#include "tac_worker/optimizations/lazy_code_motion.hpp"
#include "tac_worker/optimizations/operator_strength_reduction.hpp"
#include "tac_worker/optimizations/sparse_conditional_constant_propagation.hpp"
#include "tac_worker/print_utility.hpp"

void copy_propagation_on_ssa(Function &f) {
    auto id_to_rpo = f.get_reverse_post_ordering();
    std::map<int, int> rpo_to_id;
    for (auto &[id, rpo] : id_to_rpo)
        rpo_to_id[rpo] = id;

    std::unordered_map<std::string, std::string> copy_map;
    for (int i = 0; i <= 1; i++)
        for (auto &[rpo, id] : rpo_to_id) {
            auto &block = f.id_to_block.at(id);

            for (auto &q : block->quads) {
                for (auto &op : q.ops) {
                    if (op.is_var())
                        if (copy_map.count(op.value) > 0)
                            op.value = copy_map.at(op.value);
                }

                if (q.is_assignment()) {
                    copy_map.erase(q.dest->name);
                    if (q.type == Quad::Type::Assign && q.get_op(0)->is_var())
                        copy_map[q.dest->name] = q.get_op(0)->value;
                }
            }
        }
}

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
    //    drv.parse("../_TestCode/copy_propagation.txt");

    auto functions = collect_quads_into_functions(drv.labels, drv.quadruples);
    auto &f = functions[0];
    //    optimize(functions[0]);

    //    f.print_cfg("before.png");
    convert_to_ssa(f);

    auto operator_reduction = OSRDriver(f);
    dominator_based_value_numbering(f);

    f.print_cfg("before.png");

    copy_propagation_on_ssa(f);

    useless_code_elimination(f);


    f.print_cfg("after.png");

    std::getchar();
    return 0;
}
