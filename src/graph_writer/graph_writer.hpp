//
// Created by shoco on 10/8/2020.
//

#ifndef TAC_PARSER_GRAPH_WRITER_HPP
#define TAC_PARSER_GRAPH_WRITER_HPP

#include <fmt/ranges.h>
#include <fstream>
#include <gvc.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>

class GraphWriter {
    using AttributeMap = std::unordered_map<std::string, std::string>;
    using NodeAttributeMap = std::unordered_map<std::string, AttributeMap>;
    NodeAttributeMap node_attributes;

    std::unordered_map<std::string, std::string> node_names, additional_info_above,
        additional_info_below;
    std::vector<std::pair<std::string, std::vector<std::string>>> node_texts;
    std::vector<std::tuple<std::string, std::string, AttributeMap>> edges;
    std::string graph_title;

    void add_legend_subgraph(Agraph_t *g);

  public:
    void set_node_text(const std::string &node_name, const std::vector<std::string> &text_lines);
    void set_node_name(const std::string &node, const std::string &name);
    void add_edge(std::string node1, std::string node2,
                  std::unordered_map<std::string, std::string> attributes = {});
    void render_to_file(const std::string &filename);
    void add_info_above(const std::string &node, const std::string &info, bool above);
    void set_title(const std::string &title);

    void set_attribute(const std::string &n, const std::string &attr, const std::string &val);
    std::optional<std::string> get_attribute(const std::string &n, const std::string &attr);

    std::vector<std::vector<std::string>> legend_marks;
};

std::string escape_string(const std::string &s);

#endif // TAC_PARSER_GRAPH_WRITER_HPP
