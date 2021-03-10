//
// Created by shoco on 1/21/2021.
//

#ifndef TAC_PARSER_FUNCTION_HPP
#define TAC_PARSER_FUNCTION_HPP

#include <algorithm>

#include "basic_block.hpp"
#include "graph_writer/graph_writer.hpp"

struct Function {
    std::string name = "None";
    BasicBlocks basic_blocks;
    ID2Block id_to_block;

    Function() = default;
    Function(BasicBlocks blocks) : basic_blocks(std::move(blocks)) {
        name = basic_blocks[0]->lbl_name.value_or("None");

        connect_blocks();
        add_entry_and_exit_block();
        add_missing_jumps();
        update_block_ids();
    }

    void print_to_console() const;
    void print_basic_block_info() const;
    void print_cfg(std::string filename, std::unordered_map<int, std::string> additional_info_above = {},
                   std::unordered_map<int, std::string> additional_info_below = {},
                   std::string title = {});

    void update_block_ids();
    void connect_blocks();
    void reverse_graph();
    void add_entry_and_exit_block();
    void add_missing_jumps();
    void remove_blocks_without_predecessors();

    std::unordered_map<int, int> get_post_ordering();
    std::unordered_map<int, int> get_reverse_post_ordering() const;
    Quad& get_quad(int block_id, int quad_i) const;

    BasicBlock *get_entry_block() const;
    BasicBlock *get_exit_block() const;
};

#endif // TAC_PARSER_FUNCTION_HPP
