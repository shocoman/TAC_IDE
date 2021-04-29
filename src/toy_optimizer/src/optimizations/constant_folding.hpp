//
// Created by victor on 29.04.2021.
//

#ifndef TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_OPTIMIZATIONS_CONSTANT_FOLDING_HPP
#define TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_OPTIMIZATIONS_CONSTANT_FOLDING_HPP

#include <fmt/ranges.h>

#include "../structure/program.hpp"

void constant_folding(Quad &q);

static void run_constant_folding_on_every_quad(Function &f) {
    for (auto &b : f.basic_blocks)
        for (auto &q : b->quads)
            constant_folding(q);
}

#endif // TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_OPTIMIZATIONS_CONSTANT_FOLDING_HPP
