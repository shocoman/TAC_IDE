//
// Created by shoco on 10/7/2020.
//

#include <numeric>

#include "optimization_runner.hpp"
#include "tac_worker/data_flow_analyses/dominators.hpp"

void optimize(Function &function) {
    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    auto rpo = function.get_reverse_post_ordering();
    auto id_to_idom = get_immediate_dominators(function);

//    convert_to_ssa(function);
//    dominator_based_value_numbering(function, id_to_idom);
//    useless_code_elimination(function);
//    sparse_simple_constant_propagation(function.basic_blocks);
//    remove_phi_functions(function);
//    reaching_definitions(function);
//    liveness_analyses_dragon_book(function);
//    liveness_analyses_engineering_compiler(function);
//    available_expressions(function);
//    anticipable_expressions(function);

    function.print_cfg("after.png");
}
