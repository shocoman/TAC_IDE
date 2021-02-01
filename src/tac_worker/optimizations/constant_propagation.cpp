//
// Created by shoco on 1/30/2021.
//

#include "constant_propagation.hpp"

void constant_propagation(Function &function) {
    // forward data-flow problem

    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;

    auto id_to_rpo = function.get_reverse_post_ordering();
    auto id_rpo_pairs = std::vector<std::pair<int, int>>(id_to_rpo.begin(), id_to_rpo.end());
    // sort by rpo
    std::sort(id_rpo_pairs.begin(), id_rpo_pairs.end(),
              [](auto &a, auto &b) { return a.second < b.second; });




}

