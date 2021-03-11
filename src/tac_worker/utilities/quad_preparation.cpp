//
// Created by shoco on 1/21/2021.
//

#include "quad_preparation.hpp"

std::vector<Function> collect_quads_into_functions(std::unordered_map<std::string, int> &labels,
                                                   std::vector<Quad> &quads) {
    const auto main_function = "main";
    auto main_function_exists = labels.find(main_function) != labels.end();
    //    assert(main_function_exists && "where is 'main' function?");

    std::unordered_set<std::string> function_names;
    for (const auto &q : quads) {
        if (q.type == Quad::Type::Call && !is_builtin_function(q.get_op(0)->value)) {
            function_names.insert(q.get_op(0)->value);
        }
    }

    if (main_function_exists)
        function_names.insert(main_function);

    // add global label (function) if need one
    std::string start_label = "_start";
    auto starts_with_label = false;
    for (auto &[name, pos] : labels) {
        if (pos == 0) {
            starts_with_label = true;
            start_label = name;
            break;
        }
    }

    if (!starts_with_label) {
        labels.emplace(start_label, 0);
    }

    auto contains_global_block = false;
    if (function_names.find(start_label) == function_names.end()) {
        contains_global_block = true;
        function_names.emplace(start_label);
    }

    // collect quads into blocks
    // collect blocks into functions
    std::unordered_map<int, std::string> id_to_label;
    for (auto &[name, pos] : labels)
        id_to_label.emplace(pos, name);

    auto leader_indices = get_leading_quad_indices(quads, id_to_label);
    auto blocks = construct_basic_blocks_from_indices(quads, id_to_label, leader_indices);
    auto functions = split_basic_blocks_into_functions(std::move(blocks), function_names);

    return functions;
}

std::vector<Function> split_basic_blocks_into_functions(BasicBlocks blocks,
                                                        std::unordered_set<std::string> function_names) {

    std::vector<BasicBlocks> basic_block_groups;

    for (auto &b : blocks) {
        if (basic_block_groups.empty() || function_names.count(b->lbl_name.value_or("")) > 0) {
            basic_block_groups.emplace_back();
        }
        basic_block_groups.back().emplace_back(std::move(b));
    }

    std::vector<Function> functions;
    for (auto &group : basic_block_groups) {
        functions.emplace_back(std::move(group));
    }
    return functions;
}

BasicBlocks construct_basic_blocks_from_indices(
    const std::vector<Quad> &quads, std::unordered_map<int, std::string> &id_to_label,
    std::unordered_map<int, std::optional<std::string>> &leader_indices) {
    BasicBlocks nodes;
    BasicBlock *curr_node = nullptr;
    for (int i = 0, node_number = 0; i <= quads.size(); i++) {
        // if current quad is a leader
        if (auto leader_index = leader_indices.find(i); leader_index != leader_indices.end()) {
            if (curr_node != nullptr)
                nodes.emplace_back(curr_node);
            curr_node = new BasicBlock();
            curr_node->id = node_number++;
            curr_node->node_name = curr_node->get_name();

            if (auto lbl = id_to_label.find(i); lbl != id_to_label.end())
                curr_node->lbl_name = lbl->second;
        }
        if (i < quads.size())
            curr_node->quads.push_back(quads[i]);
    }
    if (curr_node && !curr_node->quads.empty())
        nodes.emplace_back(curr_node);
    return nodes;
}

std::unordered_map<int, std::optional<std::string>>
get_leading_quad_indices(const std::vector<Quad> &quads,
                         std::unordered_map<int, std::string> &id_to_label) {
    std::unordered_map<int, std::optional<std::string>> leader_indices = {{0, std::nullopt}};
    for (int i = 0; i < quads.size(); i++) {
        // search through labels
        if (id_to_label.find(i) != id_to_label.end())
            leader_indices.insert({i, std::nullopt});
        // check if quad is some kind of jump instruction
        if (quads[i].is_jump())
            leader_indices.insert({i + 1, quads[i].dest->name});
    }
    return leader_indices;
}

bool is_builtin_function(const std::string &func_name) {
    auto built_in_functions = {
        "iwrite", "fwrite",  "cwrite", "swrite", "iread",  "fread",    "cread",
        "sread",  "toascii", "tobyte", "toword", "tolong", "todouble",
    };

    return std::any_of(built_in_functions.begin(), built_in_functions.end(),
                       [&](auto &f) { return func_name == f; });
}
