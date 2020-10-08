//
// Created by shoco on 10/8/2020.
//

#include "DotWriter.h"

void DotWriter::save_to_file(const std::string &filename) {
    std::ofstream f(filename);
    if (f.is_open()) {
        // write here
        // write header
        f << "digraph D {" << std::endl << std::endl;

        // write node and edge default settings
        f << "node [shape=record fontname=\"Comic Sans MS\"]" << std::endl;
        f << "edge [fontname=\"Comic Sans MS\"]" << std::endl << std::endl;

        // write labels
        for (auto &[node, label_lines] : node_texts) {
            std::string final_label;
            std::string xlabel;
            if (auto n = node_names.find(node); n != node_names.end()) {
//                    xlabel += "" + n->second + "";
                final_label += "Label: " + n->second + ";\\l";
            }
            final_label += "" + node + "|";


            for (auto l : label_lines) {
                //escape utility symbols (<, >, ...)
                for (int i = l.size() - 1; i >= 0; --i) {
                    auto c = l.at(i);
                    if (c == '<' || c == '>') {
                        l.insert(i, 1, c);
                        l[i] = '\\';
                    }
                }
                // align line to the left side with \l
                final_label += l + "\\l";
            }
            f << "\"" << node << "\"" << " [label=\"{" << final_label << "}\"]" << std::endl;
        }
        f << std::endl;

        // write edges
        for (auto &[node1, node2, edge_name] : edges) {
            f << "\"" << node1 << "\"" << " -> " << "\"" << node2 << "\"";
            if (!edge_name.empty())
                f << " [label=\"" + edge_name + "\"]";
            f << std::endl;
        }
        f << std::endl;

        // write footer
        f << "}" << std::endl;

        f.close();
    } else {
        std::cout << "File error: '" << filename << "'" << std::endl;
    }

}

void DotWriter::add_edge(std::string node1, std::string node2, std::string edge_label) {
    edges.emplace_back(std::move(node1), std::move(node2), std::move(edge_label));
}

void DotWriter::set_node_name(const std::string &node, const std::string &name) {
    node_names.emplace(node, name);
}

void DotWriter::set_node_text(const std::string &node_name, const std::vector<std::string> &lines) {
    node_texts.emplace(node_name, lines);
}
