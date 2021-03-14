//
// Created by shoco on 1/21/2021.
//

#ifndef TAC_PARSER_FUNCTION_HPP
#define TAC_PARSER_FUNCTION_HPP

#include <algorithm>

#include "basic_block.hpp"
#include "../utilities/graph_writer/graph_writer.hpp"

struct Function {
    std::string function_name = "Function_has_no_name";
    BasicBlocks basic_blocks;
    ID2Block id_to_block;

    Function() = default;

    Function(BasicBlocks blocks) : basic_blocks(std::move(blocks)) {
        if (!basic_blocks.empty())
            function_name = basic_blocks.at(0)->lbl_name.value_or(function_name);

        connect_blocks();
        add_missing_jumps();
        add_entry_and_exit_block();
        update_block_ids();
    }

    Function(const Function &f) {
        for (auto &b : f.basic_blocks) {
            if (b->type != BasicBlock::Type::Normal)
                continue;

            auto block_copy = std::make_unique<BasicBlock>(*b);
            block_copy->successors.clear();
            block_copy->predecessors.clear();
            basic_blocks.emplace_back(std::move(block_copy));
        }

        if (!basic_blocks.empty())
            function_name = basic_blocks.at(0)->lbl_name.value_or(function_name);
        connect_blocks();
        add_missing_jumps();
        add_entry_and_exit_block();
        update_block_ids();
    }

    void print_to_console() const;
    void print_as_code() const;
    void print_basic_block_info() const;
    std::vector<char> print_cfg(std::string filename = "cfg.png", std::unordered_map<int, std::string> additional_info_above = {},
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
    Quad &get_quad(int block_id, int quad_i) const;

    BasicBlock *get_entry_block() const;
    BasicBlock *get_exit_block() const;
};

#endif // TAC_PARSER_FUNCTION_HPP
