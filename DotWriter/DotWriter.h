//
// Created by shoco on 10/8/2020.
//

#ifndef TAC_PARSER_DOTWRITER_H
#define TAC_PARSER_DOTWRITER_H


#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>

class DotWriter {
    std::map<std::string, std::string> node_names;
    std::map<std::string, std::vector<std::string>> node_texts;
    std::vector<std::tuple<std::string, std::string, std::string>> edges;

public:
    void set_node_text(const std::string &node_name, const std::vector<std::string> &lines);
    void set_node_name(const std::string &node, const std::string &name);
    void add_edge(std::string node1, std::string node2, std::string edge_label = "");
    void save_to_file(const std::string &filename);
};


#endif //TAC_PARSER_DOTWRITER_H
