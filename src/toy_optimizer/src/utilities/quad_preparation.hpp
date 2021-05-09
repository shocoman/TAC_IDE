//
// Created by shoco on 1/21/2021.
//

#ifndef TAC_PARSER_QUAD_PREPARATION_HPP
#define TAC_PARSER_QUAD_PREPARATION_HPP

#include <cassert>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fmt/ranges.h>

#include "../structure/basic_block.hpp"
#include "../structure/function.hpp"

bool is_builtin_function(const std::string &func_name);

std::unordered_map<int, std::optional<std::string>>
get_leading_quad_indices(const std::vector<Quad> &quads,
                         std::unordered_map<int, std::string> &id_to_label);

BasicBlocks construct_basic_blocks_from_indices(
    const std::vector<Quad> &quads, std::unordered_map<int, std::string> &id_to_label,
    std::unordered_map<int, std::optional<std::string>> &leader_indices);

std::vector<Function>
split_basic_blocks_into_functions(BasicBlocks blocks,
                                  std::unordered_set<std::string> function_names);

std::vector<Function> collect_quads_into_functions(std::unordered_map<std::string, int> &labels,
                                                   std::vector<Quad> &quads);


void NNNothing();

#endif // TAC_PARSER_QUAD_PREPARATION_HPP
