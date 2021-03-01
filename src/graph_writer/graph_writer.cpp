//
// Created by shoco on 10/8/2020.
//

#include "graph_writer.hpp"

std::string escape_string(const std::string &s) {
    std::string buffer;
    buffer.reserve(s.length() * 1.1);
    for (const auto &ch : s) {
        switch (ch) {
        case '<':
            buffer.append("&lt;");
            break;
        case '>':
            buffer.append("&gt;");
            break;
        default:
            buffer.push_back(ch);
            break;
        }
    }
    return buffer;
}

void GraphWriter::add_edge(std::string node1, std::string node2, std::string edge_label) {
    edges.emplace_back(std::move(node1), std::move(node2), std::move(edge_label));
}

void GraphWriter::set_node_name(const std::string &node, const std::string &name) {
    node_names.emplace(node, name);
}

void GraphWriter::set_node_text(const std::string &node_name,
                                const std::vector<std::string> &text_lines) {
    node_texts.emplace_back(node_name, text_lines);
}

std::optional<std::string> GraphWriter::get_attribute(const std::string &n, const std::string &attr) {
    return (node_attributes.count(n) && node_attributes.at(n).count(attr))
               ? std::make_optional(node_attributes.at(n).at(attr))
               : std::nullopt;
}

void GraphWriter::render_to_file(const std::string &filename) {
    std::unordered_map<std::string, Agnode_t *> ag_nodes;

    GVC_t *gvc = gvContext();
    Agraph_t *g = agopen((char *)"g", Agdirected, nullptr);

    // add nodes
    for (auto &[node, label_lines] : node_texts) {
        std::string final_label;
        auto node_name = node_names.count(node) ? node_names.at(node) : node;

        final_label += R"(<TABLE BORDER="0" CELLBORDER="1">)";
        if (additional_info_above.count(node))
            final_label +=
                fmt::format("<TR><TD BORDER='0'>{}</TD></TR>", additional_info_above.at(node));

        std::string subscript;
        if (auto v = get_attribute(node_name, "subscript"); v.has_value())
            subscript = fmt::format("<FONT COLOR='gray35' POINT-SIZE='13'>"
                                    "<SUB><I>{}</I></SUB></FONT>",
                                    v.value());

        // node title
        final_label += fmt::format("<TR><TD COLSPAN='2' PORT='{0}_enter'>"
                                   "{0}{1}</TD></TR>",
                                   node_name, subscript);

        if (!label_lines.empty()) {
            final_label += "<TR><TD COLSPAN='2'><FONT>";
            for (const auto &l : label_lines) {
                // align line to the left side
                final_label += l + "<BR ALIGN='LEFT'/>";
            }
            final_label += "</FONT></TD></TR>";
        }
        if (additional_info_below.count(node))
            final_label +=
                fmt::format("<TR><TD BORDER='0'>{}</TD></TR>", additional_info_below.at(node));

        //        if (node_attributes.count(node_name) &&
        //        node_attributes.at(node_name).count("true_branch"))
        //            final_label += fmt::format("<TR><TD PORT='{0}_T'>T</TD><TD
        //            PORT='{0}_F'>F</TD></TR>", node_name);

        final_label += "</TABLE>";

        Agnode_t *ag_node = agnode(g, (char *)node.c_str(), 1);
        agset(ag_node, (char *)"label", agstrdup_html(g, (char *)final_label.c_str()));

        ag_nodes.emplace(node, ag_node);
    }

    // add edges
    for (auto &[node1, node2, edge_name] : edges) {
        auto &n1 = ag_nodes.at(node1);
        auto &n2 = ag_nodes.at(node2);

        Agedge_t *e = agedge(g, n1, n2, (char *)edge_name.c_str(), 1);

        //        if (node_attributes.count(node1) && node_attributes.at(node1).count("true_branch")) {
        //            std::string &true_branch_name = node_attributes[node1]["true_branch"];
        //            auto tail_port = fmt::format(true_branch_name == node2 ? "{}_T" : "{}_F", node1);
        //            agsafeset(e, (char *)"tailport", (char *)tail_port.c_str(), (char *)"");
        //        }

        //        auto head_port = fmt::format("{}_enter", node2);
        //        agsafeset(e, (char *)"headport", (char *)head_port.c_str(), (char *)"");

        agsafeset(e, (char *)"label", (char *)edge_name.c_str(), (char *)"");
    }

    // set shape and font for all nodes and edges
    auto font_name = "DejaVu Sans Mono";
    agattr(g, AGRAPH, (char *)"fontname", (char *)font_name);
    agattr(g, AGRAPH, (char *)"fontsize", (char *)"25");
    agattr(g, AGRAPH, (char *)"labelloc", (char *)"t"); // graph title position (top)

    agattr(g, AGRAPH, (char *)"label", agstrdup_html(g, (char *)graph_title.c_str()));
    agattr(g, AGNODE, (char *)"fontname", (char *)font_name);
    agattr(g, AGNODE, (char *)"shape", (char *)"none");
    agattr(g, AGEDGE, (char *)"fontname", (char *)font_name);

    // print graph as png image to a file
    gvLayout(gvc, g, "dot");
    int res = gvRenderFilename(gvc, g, "png", (char *)filename.c_str());
    gvRenderFilename(gvc, g, "dot", (char *)"graph_in_text.dot");
    if (res)
        printf("Graphviz error. Something wrong with graph rendering: %i", res);

    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);
}

void GraphWriter::add_info_above(const std::string &node, const std::string &info, bool above) {
    if (above)
        additional_info_above[node] += info;
    else
        additional_info_below[node] += info;
}

void GraphWriter::set_title(const std::string &title) { graph_title = title; }

void GraphWriter::set_attribute(const std::string &n, const std::string &attr, const std::string &val) {
    node_attributes[n][attr] = val;
}
