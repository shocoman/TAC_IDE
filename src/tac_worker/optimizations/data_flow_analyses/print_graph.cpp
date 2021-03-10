//
// Created by shoco on 3/2/2021.
//

#include "print_graph.hpp"

void print_dominator_tree(Function &f) {
    auto id_to_idom = get_immediate_dominators(f);
    auto id_to_doms = get_dominance_frontier(f, id_to_idom);

    GraphWriter writer;
    for (const auto &[id1, id2] : id_to_idom) {
        // make a connection between blocks[id1] and blocks[block_id]
        auto name1 = f.id_to_block.at(id2)->get_name();
        auto name2 = f.id_to_block.at(id1)->get_name();
        writer.set_node_name(name1, name1);
        writer.set_node_name(name2, name2);
        writer.set_node_text(name1, {});
        writer.set_node_text(name2, {});
        writer.add_edge(name1, name2);

        writer.set_attribute(name1, "subscript", fmt::format("{}<BR/>df={}", id2, id_to_doms.at(id2)));
        writer.set_attribute(name2, "subscript", fmt::format("{}<BR/>df={}", id1, id_to_doms.at(id1)));
    }

    writer.set_title("Dominator Tree and Dominance Frontier");
    writer.render_to_file("graphs/dominator_tree.png");
    system("sxiv -g 1000x1000+20+20 graphs/dominator_tree.png &");
}

void print_postdominator_tree(Function &f) {
    f.reverse_graph();
    auto id_to_idom = get_immediate_dominators(f);
    auto id_to_doms = get_dominance_frontier(f, id_to_idom);
    f.reverse_graph();

    GraphWriter writer;
    for (const auto &[id1, id2] : id_to_idom) {
        // make a connection between blocks[id1] and blocks[block_id]
        auto name1 = f.id_to_block.at(id2)->get_name();
        auto name2 = f.id_to_block.at(id1)->get_name();
        writer.set_node_name(name1, name1);
        writer.set_node_name(name2, name2);
        writer.set_node_text(name1, {});
        writer.set_node_text(name2, {});
        writer.add_edge(name1, name2);

        writer.set_attribute(name1, "subscript", fmt::format("{}<BR/>df={}", id2, id_to_doms.at(id2)));
        writer.set_attribute(name2, "subscript", fmt::format("{}<BR/>df={}", id1, id_to_doms.at(id1)));
    }

    writer.set_title("Postdominator Tree and Postdominance Frontier");
    writer.render_to_file("graphs/dominator_tree.png");
    system("sxiv -g 1000x1000+20+20 graphs/dominator_tree.png &");
}

void print_control_dependence(Function &f) {
    f.reverse_graph();
    auto id_to_postdom = get_immediate_dominators(f);
    f.reverse_graph();

    auto GetControlDependence = [&](std::pair<int, int> edge) {
        std::vector<int> depends;
        auto [b, s] = edge;
        int x = s;
        while (x != id_to_postdom.at(b)) {
            depends.push_back(x);
            x = id_to_postdom.at(x);
        }
        return depends;
    };

    GraphWriter writer;
    for (auto &b : f.basic_blocks) {
        writer.set_attribute(b->get_name(), "subscript", fmt::format("{}", b->id));
        writer.set_node_text(b->get_name(), {});

        for (auto &succ : b->successors) {
            auto ctrl_dep = GetControlDependence({b->id, succ->id});
            std::vector<std::string> ctrl_dep_str;
            std::transform(ctrl_dep.begin(), ctrl_dep.end(), std::back_inserter(ctrl_dep_str),
                           [&](auto id) { return f.id_to_block.at(id)->get_name(); });
            writer.add_edge(b->get_name(), succ->get_name(),
                            {{"label", fmt::format("{}", ctrl_dep_str)}});
        }
    }

    writer.set_title("Control Dependence");
    writer.render_to_file("graphs/control_dependence.png");
    system("sxiv -g 1000x1000+20+20 graphs/control_dependence.png &");
}

void print_available_expressions(Function &f) {
    auto [AvailIn, AvailOut] = available_expressions(f);
    print_analysis_result_on_graph(f, AvailIn, AvailOut, "Available expressions", print_expression);
}

void print_anticipable_expressions(Function &f) {
    auto [AntIn, AntOut] = anticipable_expressions(f);
    print_analysis_result_on_graph(f, AntIn, AntOut, "Anticipable expressions", print_expression);
}

void print_live_variable(Function &f) {
    auto [IN, OUT] = live_variable_analyses(f);
    print_analysis_result_on_graph(f, IN, OUT, "Live variable analyses", [](auto &v) { return v; });
}

void print_depth_first_search_tree(Function &f) {
    auto id_to_doms = get_dominators(f);

    int preorder = 1;
    int rpostorder = f.basic_blocks.size();
    struct NodeInfo {
        int preorder = 0;
        int rpostorder = 0;
    };
    enum class EdgeType { Forward, Back, Retreating, Tree, Cross };
    std::unordered_map<int, NodeInfo> node_infos;
    std::map<std::pair<int, int>, EdgeType> edge_types;

    std::function<void(BasicBlock *)> DFS = [&](BasicBlock *b) {
        node_infos[b->id].preorder = preorder++;

        for (auto &succ : b->successors) {
            if (node_infos[succ->id].preorder == 0) {
                edge_types[{b->id, succ->id}] = EdgeType::Tree;
                DFS(succ);
            } else if (id_to_doms.at(b->id).count(succ->id) > 0)
                edge_types[{b->id, succ->id}] = EdgeType::Back;
            else if (node_infos[succ->id].rpostorder == 0)
                edge_types[{b->id, succ->id}] = EdgeType::Retreating;
            else if (node_infos[b->id].preorder < node_infos[succ->id].preorder)
                edge_types[{b->id, succ->id}] = EdgeType::Forward;
            else
                edge_types[{b->id, succ->id}] = EdgeType::Cross;
        }

        node_infos[b->id].rpostorder = rpostorder--;
    };

    DFS(f.get_entry_block());

    GraphWriter dot_writer;
    dot_writer.legend_marks = {
        {"Forward edge<BR/>(node is already processed)", "black", "solid"},
        {"Back (and retreating) edge<BR/>(retreat edge to dominator)", "indigo", "dashed"},
        {"Retreat edge<BR/>(node has started processing<BR/>but hasn't finished yet)", "gray", "dashed"},
        {"Tree edge<BR/>(it's first time when node<BR/>is being visited)", "brown", "bold"},
        {"Cross edge<BR/>(edge goes from one DFS<BR/>subtree to another)", "sienna", "dotted"}};

    // print edges
    for (const auto &n : f.basic_blocks) {
        auto node_name = n->get_name();

        // node name
        dot_writer.set_node_text(node_name, {});

        // subscript
        auto [pre, rpost] = node_infos.at(n->id);
        dot_writer.set_attribute(node_name, "subscript",
                                 fmt::format("<BR/>preorder={};rpostorder={}", pre, rpost));

        // edges
        for (auto &s : n->successors) {
            auto edge_type = edge_types.at({n->id, s->id});
            std::string color = dot_writer.legend_marks[(int)edge_type][1],
                        style = dot_writer.legend_marks[(int)edge_type][2];

            std::unordered_map<std::string, std::string> attributes = {
                {"label", s->lbl_name.value_or("")},
                {"color", color},
                {"style", style},
            };

            dot_writer.add_edge(node_name, s->get_name(), attributes);
        }
    }

    dot_writer.set_title("Depth-First Search Spanning Tree");
    std::string filename = "graphs/dfs_tree.png";
    dot_writer.render_to_file(filename);
    system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
}

void print_ue_de_and_killed_expressions(Function &f) {
    auto all_expressions = get_all_expressions(f);
    auto [downward_exposed, killed] = get_downward_exposed_and_killed_expressions(f);
    auto upward_exposed = get_upward_exposed_and_killed_expressions(f).first;

    std::unordered_map<int, std::string> above, below;
    for (auto &[id, ue] : upward_exposed) {
        auto ue_str = "UE: " + print_into_string_with(ue, print_expression);
        auto de_str = "DE: " + print_into_string_with(downward_exposed[id], print_expression);
        auto killed_str = "Killed: " + print_into_string_with(killed[id], print_expression);

        above[id] = fmt::format("{}", split_long_string(ue_str, 25));
        below[id] =
            fmt::format("{}<BR/>{}", split_long_string(de_str, 25), split_long_string(killed_str, 25));
    }

    std::string title = "UE, DE and killed expressions";
    title += "<BR/>All expressions: " + print_into_string_with(all_expressions, print_expression);
    f.print_cfg("expressions_info.png", above, below, title);
}


void print_lazy_code_motion_graphs(Function &f) {
    auto [AntIn, AntOut] = anticipable_expressions(f);
    auto [AvailIn, AvailOut] = available_expressions_lazy_code_motion(f);
    auto earliest_exprs = earliest_expressions(AntIn, AvailIn);
    auto latest_exprs = latest_expressions(f);
    auto [PostIn, PostOut] = postponable_expressions(f);
    auto [UsedIn, UsedOut] = used_expressions(f);

    print_analysis_result_on_graph(f, AntIn, AntOut, "Anticipable Expressions (1)", print_expression);
    print_analysis_result_on_graph(f, AvailIn, AvailOut, "Available Expressions (lcm) (2)", print_expression);
    print_analysis_result_on_graph(f, earliest_exprs, {}, "Earliest Expressions (3)", print_expression);
    print_analysis_result_on_graph(f, PostIn, PostOut, "Postponable Expressions (4)", print_expression);
    print_analysis_result_on_graph(f, latest_exprs, {}, "Latest Expressions (5)", print_expression);
    print_analysis_result_on_graph(f, UsedIn, UsedOut, "Used Expressions (6)", print_expression);
    print_ue_de_and_killed_expressions(f);
}
