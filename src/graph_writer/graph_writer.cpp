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

void GraphWriter::add_edge(std::string node1, std::string node2,
                           std::unordered_map<std::string, std::string> attributes) {
    edges.emplace_back(std::move(node1), std::move(node2), std::move(attributes));
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

        std::string cell_style;
        if (auto v = get_attribute(node_name, "style"); v.has_value())
            cell_style = v.value();

        std::string subscript;
        if (auto v = get_attribute(node_name, "subscript"); v.has_value())
            subscript = fmt::format("<FONT COLOR='gray35' POINT-SIZE='13'>"
                                    "<SUB><I>{}</I></SUB></FONT>",
                                    v.value());

        // node title
        final_label += fmt::format("<TR><TD STYLE='{2}' COLSPAN='2' PORT='{0}_enter'>"
                                   "{0}{1}</TD></TR>",
                                   node_name, subscript, cell_style);

        if (!label_lines.empty()) {
            final_label += fmt::format("<TR><TD STYLE='{}' COLSPAN='2'><FONT>", cell_style);
            for (const auto &l : label_lines) {
                // align line to the left side
                final_label += l + "<BR ALIGN='LEFT'/>";
            }
            final_label += "</FONT></TD></TR>";
        }
        if (additional_info_below.count(node))
            final_label +=
                fmt::format("<TR><TD BORDER='0'>{}</TD></TR>", additional_info_below.at(node));

        // if (auto a = get_attribute(node_name, "true_branch"); a.has_value())
        //     final_label += fmt::format("<TR><TD PORT='{0}_T'>T</TD><TD
        //     PORT='{0}_F'>F</TD></TR>", node_name);

        final_label += "</TABLE>";

        Agnode_t *ag_node = agnode(g, (char *)node.c_str(), 1);
        agset(ag_node, (char *)"label", agstrdup_html(g, (char *)final_label.c_str()));
        if (auto v = get_attribute(node_name, "pencolor"); v.has_value())
            agset(ag_node, (char *)"pencolor", (char *)v.value().c_str());

        ag_nodes.emplace(node, ag_node);
    }

    // add edges
    for (auto &[node1, node2, attributes] : edges) {
        auto &n1 = ag_nodes.at(node1);
        auto &n2 = ag_nodes.at(node2);
        auto edge_name = attributes.count("label") > 0 ? attributes.at("label") : "edge";
        Agedge_t *e = agedge(g, n1, n2, (char *)edge_name.c_str(), 1);

        //        if (auto a = get_attribute(node1, "true_branch"); a.has_value()) {
        //            std::string &true_branch_name = a.value();
        //            auto tail_port = fmt::format(true_branch_name == node2 ? "{}_T" : "{}_F", node1);
        //            agsafeset(e, (char *)"tailport", (char *)tail_port.c_str(), (char *)"");
        //        }
        //        auto head_port = fmt::format("{}_enter", node2);
        //        agsafeset(e, (char *)"headport", (char *)head_port.c_str(), (char *)"");

        for (auto &[attr, value] : attributes)
            agsafeset(e, (char *)attr.c_str(), (char *)value.c_str(), (char *)"");
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

    if (!legend_marks.empty()) {
        add_legend_subgraph(g);
    }

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

void GraphWriter::add_legend_subgraph(Agraph_t *g) {
    Agraph_s *legend_subgraph = agsubg(g, (char *)"acluster_name", 1);
    agattr(legend_subgraph, AGRAPH, (char *)"rank", (char *)"same");
    Agnode_t *ag_node_key1 = agnode(legend_subgraph, (char *)"Key1", 1);
    Agnode_t *ag_node_key2 = agnode(legend_subgraph, (char *)"Key2", 1);

    std::string node_key1_text = "<table border='0' cellpadding='2' cellspacing='0' cellborder='0'>",
                node_key2_text = node_key1_text;

    for (int i = 0; i < legend_marks.size(); ++i) {
        auto &name = legend_marks[i][0], &color = legend_marks[i][1], &style = legend_marks[i][2];

        // count occurrences of new line symbol
        int occurrences = 0, start = 0;
        std::string to_find_occurrences_of = "<BR/>";
        while ((start = name.find(to_find_occurrences_of, start)) != std::string::npos) {
            ++occurrences;
            start += to_find_occurrences_of.length();
        }

        // pad node text with nl's
        std::string dummy_text = "&nbsp;";
        for (int j = 0; j < occurrences; ++j)
            dummy_text += "<BR/>&nbsp;";

        auto port_id = fmt::format("i{}", i);

        node_key1_text += fmt::format("<tr><td align='right' port='{}'>{}</td></tr>", port_id, name);
        node_key2_text += fmt::format("<tr><td port='{}'>{}</td></tr>", port_id, dummy_text);

        Agedge_t *e = agedge(legend_subgraph, ag_node_key1, ag_node_key2, (char *)port_id.c_str(), 1);
        agsafeset(e, (char *)"tailport", (char *)(port_id + ":e").c_str(), (char *)"");
        agsafeset(e, (char *)"headport", (char *)(port_id + ":w").c_str(), (char *)"");
        agsafeset(e, (char *)"color", (char *)color.c_str(), (char *)"");
        agsafeset(e, (char *)"style", (char *)style.c_str(), (char *)"");
    }

    node_key1_text += "</table>";
    node_key2_text += "</table>";

    agset(ag_node_key1, (char *)"label", agstrdup_html(legend_subgraph, (char *)node_key1_text.c_str()));
    agset(ag_node_key2, (char *)"label", agstrdup_html(legend_subgraph, (char *)node_key2_text.c_str()));
}
