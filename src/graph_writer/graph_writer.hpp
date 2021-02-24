//
// Created by shoco on 10/8/2020.
//

#ifndef TAC_PARSER_GRAPH_WRITER_HPP
#define TAC_PARSER_GRAPH_WRITER_HPP

#include <fstream>
#include <gvc.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fmt/ranges.h>

class GraphWriter {
    std::unordered_map<std::string, std::string> node_names, additional_info_above,
        additional_info_below;
    std::vector<std::pair<std::string, std::vector<std::string>>> node_texts;
    std::vector<std::tuple<std::string, std::string, std::string>> edges;
    std::string graph_title;

  public:
    void set_node_text(const std::string &node_name, const std::vector<std::string> &text_lines);
    void set_node_name(const std::string &node, const std::string &name);
    void add_edge(std::string node1, std::string node2, std::string edge_label = "");
    void render_to_file(const std::string &filename);
    void add_info_above(const std::string &node, const std::string &info, bool above);
    void set_title(const std::string &title);

    using NodeAttributeMap = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;
    NodeAttributeMap node_attributes;
};

std::string escape_string(const std::string &s);

#endif // TAC_PARSER_GRAPH_WRITER_HPP
