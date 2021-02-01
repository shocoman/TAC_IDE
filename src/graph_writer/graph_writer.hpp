//
// Created by shoco on 10/8/2020.
//

#ifndef TAC_PARSER_GRAPH_WRITER_HPP
#define TAC_PARSER_GRAPH_WRITER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <iostream>
#include <gvc.h>

class GraphWriter {
    std::unordered_map<std::string, std::string> node_names;
    std::vector<std::pair<std::string, std::vector<std::string>>> node_texts;
    std::vector<std::tuple<std::string, std::string, std::string>> edges;

public:
    void set_node_text(const std::string &node_name, const std::vector<std::string> &text_lines);
    void set_node_name(const std::string &node, const std::string &name);
    void add_edge(std::string node1, std::string node2, std::string edge_label = "");
    void render_to_file(const std::string &filename);
};


#endif // TAC_PARSER_GRAPH_WRITER_HPP
