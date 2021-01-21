//
// Created by shoco on 1/21/2021.
//

#ifndef TAC_PARSER_FUNCTION_H
#define TAC_PARSER_FUNCTION_H

#include "basic_block.h"
struct Function {
    BasicBlocks basic_blocks;

    Function(BasicBlocks blocks): basic_blocks{std::move(blocks)} {
        std::cout << "This cs called" << std::endl;
//        basic_blocks = std::move(blocks);
//        connect_blocks(); 
    }
    //    explicit Function(BasicBlocks &&blocks) : basic_blocks{std::move(blocks)} {
    //    connect_blocks(); }

    void connect_blocks() {
        for (int i = 0; i < basic_blocks.size(); ++i) {
            if (i != basic_blocks.size() - 1 && basic_blocks[i]->allows_fallthrough()) {
                basic_blocks[i]->add_successor(basic_blocks[i + 1].get());
            }
            if (basic_blocks[i]->jumps_to.has_value()) {
                auto jump_to = basic_blocks[i]->jumps_to.value();
                auto block =
                    std::find_if(basic_blocks.begin(), basic_blocks.end(), [&jump_to](auto &e) {
                        return e->lbl_name.has_value() && e->lbl_name.value() == jump_to;
                    });

                basic_blocks[i]->add_successor(block->get());
            }
        }
    }

    void print() {
        for (auto &b : basic_blocks) {
            std::cout << "\tBasicBlock: " << b->node_name << "; " << b->lbl_name.value_or("NONE")
                      << "; Jumps to " << b->jumps_to.value_or("NONE")
                      << "; Successors: " << b->successors.size()
                      << "; Predecessors: " << b->predecessors.size() << " \n"
                      << b->fmt();
        }
    }
};

#endif // TAC_PARSER_FUNCTION_H
