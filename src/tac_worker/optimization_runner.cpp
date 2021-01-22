//
// Created by shoco on 10/7/2020.
//

#include <numeric>

#include "optimization_runner.hpp"

void print_loops(BasicBlocks &nodes) {
    // generate ids for node names
    std::map<int, std::string> name_by_id;
    std::map<std::string, int> id_by_name;
    int counter = 0;
    for (const auto &n : nodes) {
        name_by_id[counter] = n->node_name;
        id_by_name[n->node_name] = counter;
        ++counter;
    }

    std::map<int, std::list<int>> adjacency_list;
    for (const auto &n : nodes) {
        adjacency_list[id_by_name[n->node_name]] = {};
        for (const auto &s : n->successors) {
            adjacency_list[id_by_name[n->node_name]].emplace_back(id_by_name[s->node_name]);
        }
    }

    LoopFinder l = LoopFinder(adjacency_list);
    l.find();

    std::cout << "LOOPS: " << std::endl;
    for (const auto &loop : l.loops) {
        for (const auto &i : loop) {
            std::cout << name_by_id.at(i) << " -> ";
        }
        std::cout << std::endl;
    }
}

void optimize(Function &function) {
    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    auto rpo = function.get_reverse_post_ordering();

    //    function.print_basic_block_info();
    //    function.print_to_console();
    //    function.remove_blocks_without_predecessors();

    //    constant_folding(blocks);
    //    liveness_analyses(blocks);
    //    print_loops(blocks);

//    function.print_cfg("before.png");

    //    for (auto &n : blocks) {
    //        ValueNumberTableStack t;
    //        t.push_table();
    //        local_value_numbering(n->quads, t);
    //    }

    //    live_analyses(blocks);
//    superlocal_value_numbering(blocks);

    convert_to_ssa(function);
    //    function.print_cfg("ssa");

    //    remove_phi_functions(function);

    //    sparse_simple_constant_propagation(blocks);

    //    reverse_graph(blocks);
    //    auto id_to_rev_idom = find_immediate_dominators(<#initializer #>);
    //    ID2DF revDF = find_dominance_frontier(blocks, id_to_rev_idom);
    //    reverse_graph(blocks);

    //    for (auto &[id, df] : revDF) {
    //        std::cout << "DF: " << id_to_block.at(id)->node_name << " || ";
    //        for (auto &d : df) {
    //            std::cout << id_to_block.at(d)->node_name << ", ";
    //        }
    //        std::cout << std::endl;
    //    }

    //    convert_to_ssa(blocks, id_to_block);
    //    sparse_simple_constant_propagation(blocks);

    //    useless_code_elimination(blocks, id_to_block, revDF, id_to_rev_idom);

    // find_immediate_dominators(blocks, id_to_block);
    //    print_dominator_tree(id_to_block, 0, id_to_idom);

    //    print_cfg(blocks, "before.png");
    //
    //    id_to_immediate_dominator id_to_idom = find_immediate_dominators(blocks, id_to_block);
    //
    //    dominator_based_value_numbering(blocks, id_to_block, id_to_idom);

    function.print_cfg("after.png");
}
