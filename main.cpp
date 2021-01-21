#include "tac_worker/Function.h"
#include <iostream>
#include <numeric>
#include <unordered_set>

#include "parse_driver/driver.hpp"
#include "tac_worker/dataflow_graph.hpp"

bool is_builtin_function(const std::string &func_name) {
    auto ends_with = [&](const std::string &ending) {
        return func_name.size() >= ending.size() &&
               !func_name.compare(func_name.size() - ending.size(), ending.size(), ending);
    };
    return ends_with("read") || ends_with("write");
}

std::unordered_map<int, std::string>
get_leading_quad_indices(const std::vector<Quad> &quads,
                         std::unordered_map<int, std::string> &id_to_label) {
    std::unordered_map<int, std::string> leader_indices = {{0, ""}};
    for (int i = 0; i < quads.size(); i++) {
        // search through labels
        if (id_to_label.find(i) != id_to_label.end())
            leader_indices.insert({i, ""});
        // check if quad is some kind of jump instruction
        if (quads[i].is_jump())
            leader_indices.insert({i + 1, quads[i].dest->name});
    }
    return leader_indices;
}

BasicBlocks
construct_basic_blocks_from_indices(const std::vector<Quad> &quads,
                                    std::unordered_map<int, std::string> &id_to_label,
                                    std::unordered_map<int, std::string> &leader_indices) {
    BasicBlocks nodes;
    BasicBlock *curr_node = nullptr;
    for (int i = 0, node_number = 0; i <= quads.size(); i++) {
        // if current quad is a leader
        if (auto leader_index = leader_indices.find(i); leader_index != leader_indices.end()) {
            if (curr_node != nullptr) {
                curr_node->jumps_to = leader_index->second;
                nodes.emplace_back(curr_node);
            }
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

std::vector<Function>
split_basic_blocks_into_functions(BasicBlocks blocks,
                                  std::unordered_set<std::string> function_names) {
    std::vector<BasicBlocks> basic_block_groups;

    for (auto &b : blocks) {
        if (basic_block_groups.empty() ||
            function_names.find(b->lbl_name.value_or("")) != function_names.end()) {
            basic_block_groups.emplace_back();
        }
        basic_block_groups.back().emplace_back(std::move(b));
    }

    std::vector<Function> functions;
    for (auto &group : basic_block_groups) {
//        auto f = Function(std::move(group));
        functions.emplace_back(std::move(group));
    }
    return functions;
}

void find_functions(std::map<std::string, int> &labels, std::vector<Quad> &quads) {
    const auto main_function = "main";
    assert(labels.find(main_function) != labels.end() && "where is 'main' function?");

    std::unordered_set<std::string> function_names;
    for (const auto &q : quads) {
        if (q.type == Quad::Type::Call && !is_builtin_function(q.get_op(0)->value)) {
            function_names.insert(q.get_op(0)->value);
        }
    }
    function_names.insert(main_function);

    // add global label if need one
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

    for (auto &f : functions) {
        //        f.print();
    }
}

int main(int argc, char *argv[]) {
    setenv("DISPLAY", "192.168.211.241:0", true);

    ParseDriver drv;

    //    for (int i = 1; i < argc; ++i)
    //        if (argv[i] == std::string ("-p"))
    //            drv.trace_parsing = true;
    //        else if (argv[i] == std::string ("-s"))
    //            drv.trace_scanning = true;
    //        else if (!drv.parse (argv[i]))
    //            std::cout << "Parsing result: " << drv.result << '\n';
    //        else
    //            res = 1;

    //    drv.parse("../myfile");
    //    drv.parse("../myfile2");
    drv.parse("../FactorialProgram.txt");

    //    make_cfg(std::move(drv.labels), std::move(drv.quadruples));
    find_functions(drv.labels, drv.quadruples);

    return 0;
}
