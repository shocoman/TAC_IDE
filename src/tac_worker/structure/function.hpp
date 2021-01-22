//
// Created by shoco on 1/21/2021.
//

#ifndef TAC_PARSER_FUNCTION_HPP
#define TAC_PARSER_FUNCTION_HPP

#include <algorithm>

#include "basic_block.hpp"
#include "graph_writer/graph_writer.hpp"


struct Function {
    BasicBlocks basic_blocks;
    ID2Block id_to_block;

    Function() = delete;
    Function(BasicBlocks blocks) : basic_blocks(std::move(blocks)) {
        connect_blocks();
        add_entry_and_exit_block();
        add_missing_jumps();

        for (auto &b : basic_blocks)
            id_to_block[b->id] = b.get();
    }

    std::string get_function_name() const { return basic_blocks[0]->lbl_name.value_or("None"); }
    void print_to_console() const;
    void print_basic_block_info() const;
    void print_cfg(const std::string &filename) const;

    void connect_blocks();
    void reverse_graph();
    void add_entry_and_exit_block();
    void add_missing_jumps();
    void remove_blocks_without_predecessors();

    std::unordered_map<int, int> get_post_ordering();
    std::unordered_map<int, int> get_reverse_post_ordering() const;

    BasicBlock *find_root_node() const;
};

#endif // TAC_PARSER_FUNCTION_HPP
