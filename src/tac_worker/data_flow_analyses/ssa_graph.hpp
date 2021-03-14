//
// Created by shoco on 2/27/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_SSA_GRAPH_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_SSA_GRAPH_HPP

#include <map>
#include <optional>
#include <vector>

#include "tac_worker/structure/function.hpp"

struct SSAGraph {
    using Position = std::pair<int, int>;
    struct VariableInfo {
        std::optional<Position> defined_at;
        std::vector<Position> used_at;
    };
    std::map<std::string, VariableInfo> use_info;
    Function &function;
    bool include_constants;

    explicit SSAGraph(Function &f, bool include_constants_)
        : function(f), include_constants(include_constants_) {
        update_use_info();
    }

    void update_use_info() {
        use_info.clear();

        for (auto &b : function.basic_blocks) {
            for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
                auto &q = b->quads[q_index];

                Position pos{b->id, q_index};
                if (!q.is_jump() && q.dest.has_value())
                    use_info[q.dest->name].defined_at = pos;

                for (const auto &op_name : q.get_rhs(include_constants))
                    use_info[op_name].used_at.push_back(pos);
            }
        }
    }

    void print_graph(std::string filename) {
        GraphWriter dot_writer;
        std::unordered_set<std::string> visited;
        for (const auto &[node_name, val] : use_info) {

            if (visited.insert(node_name).second) {
                if (!val.defined_at.has_value()) {
                    dot_writer.set_node_text(node_name, {});
                    continue;
                }

                auto [b_id, q_id] = val.defined_at.value();
                auto &q = function.id_to_block.at(b_id)->quads.at(q_id);
                dot_writer.set_node_text(node_name, {escape_string(q.fmt(true))});

                for (auto &s : q.get_rhs(include_constants))
                    dot_writer.add_edge(node_name, s);
            }
        }

        filename = "graphs/" + filename;
        dot_writer.set_title("UseDef Graph");
        dot_writer.render_to_file(filename);
        system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
    }
};

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_SSA_GRAPH_HPP
