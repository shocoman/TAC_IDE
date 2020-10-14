//
// Created by shoco on 10/8/2020.
//

#include "DotWriter.h"


void DotWriter::add_edge(std::string node1, std::string node2, std::string edge_label) {
    edges.emplace_back(std::move(node1), std::move(node2), std::move(edge_label));
}

void DotWriter::set_node_name(const std::string &node, const std::string &name) {
    node_names.emplace(node, name);
}

void DotWriter::set_node_text(const std::string &node_name, const std::vector<std::string> &lines) {
    node_texts.emplace_back(node_name, lines);
}

void DotWriter::write_dot_to_file(const std::string &filename) {
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
                // xlabel += "" + n->second + "";
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


void DotWriter::render_to_file(const std::string &filename) {
    std::map<std::string, Agnode_t *> ag_nodes;
    std::map<std::string, Agedge_t *> ag_edges;

    GVC_t *gvc = gvContext();
    Agraph_t *g = agopen((char *) "g", Agdirected, nullptr);


    for (auto &[node, label_lines] : node_texts) {
        std::string final_label;
        if (auto n = node_names.find(node); n != node_names.end()) {
            final_label += "Label: " + n->second + ";\\l";
        }
        final_label += node + "|";

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
        // for correct vertical label formatting
        final_label.insert(0, "{");
        final_label.append("}");

        Agnode_t *ag_node = agnode(g, (char *) node.c_str(), 1);
        agsafeset(ag_node, (char *) "label", (char *) final_label.c_str(), (char *) "");

        ag_nodes.emplace(node, ag_node);
    }

    for (auto &[node1, node2, edge_name] : edges) {
        auto n1 = ag_nodes.find(node1)->second;
        auto n2 = ag_nodes.find(node2)->second;

        Agedge_t *e = agedge(g, n1, n2, (char *) edge_name.c_str(), 1);
        agsafeset(e, (char *) "label", (char *) edge_name.c_str(), (char *) "");
//        agsafeset(e, (char *) "headport", (char *) "n", (char *) "");
//        agsafeset(e, (char *) "tailport", (char *) "s", (char *) "");
//        agsafeset(e, (char *) "constraint", (char *) "false", (char *) "");
        ag_edges.emplace(edge_name, e);
    }

    // set shape and font for nodes and edges
    agsafeset(ag_nodes.begin()->second, (char *) "fontname", (char *) "Comic Sans MS", (char *) "Comic Sans MS");
    agsafeset(ag_nodes.begin()->second, (char *) "shape", (char *) "record", (char *) "record");
    agsafeset(ag_edges.begin()->second, (char *) "fontname", (char *) "Comic Sans MS", (char *) "Comic Sans MS");

    gvLayout(gvc, g, "dot");
    int res = gvRenderFilename(gvc, g, "png", (char *) filename.c_str());
    if (res) {
        printf("Graphviz error. Something wrong with graph rendering: %i", res);
    }

    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);
}
