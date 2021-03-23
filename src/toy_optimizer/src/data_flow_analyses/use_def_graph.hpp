//
// Created by shoco on 3/10/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_USE_DEF_GRAPH_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_USE_DEF_GRAPH_HPP

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "../structure/function.hpp"
#include "data_flow_analyses.hpp"
#include "data_flow_framework.hpp"
#include "reaching_definitions.hpp"

struct UseDefGraph {
    using Location = ReachingDefinitionsDriver::Location;
    using Use = ReachingDefinitionsDriver::Use;
    using Definition = ReachingDefinitionsDriver::Definition;
    using DefinitionsSet = std::set<Definition>;
    using UsesSet = std::set<Use>;

    struct IntermediateResults {
        std::optional<ReachingDefinitionsDriver> reaching_definitions;
    } ir;

    Function &f;
    std::map<Use, DefinitionsSet> use_to_definitions;
    std::map<Definition, UsesSet> definition_to_uses;

    UseDefGraph(Function &f_) : f(f_) { update_graph(); }

    void update_graph() {
        ir.reaching_definitions.emplace(f);
        ir.reaching_definitions->run();

        compute_use_def_chains();
    }

    void compute_use_def_chains() {
        auto &[IN, OUT] = ir.reaching_definitions->get_results();
        for (auto &b : f.basic_blocks) {
            auto in_definitions = IN[b->id];
            std::map<std::string, DefinitionsSet> name_to_defs;
            for (auto &def : in_definitions)
                name_to_defs[def.name].insert(def);

            for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
                auto &q = b->quads[quad_i];

                for (auto &op_name : q.get_rhs_names(false)) {
                    auto use = Use{{b->id, quad_i}, op_name};
                    use_to_definitions[use] = name_to_defs.at(op_name);
                }

                if (q.is_assignment()) {
                    auto &def = ir.reaching_definitions->ir.location_to_definition.at({b->id, quad_i});
                    name_to_defs[q.dest->name] = {def};
                }
            }
        }

        // gather reverse chains - defuse
        for (auto &[use, defs] : use_to_definitions)
            for (auto &def : defs)
                definition_to_uses[def].insert(use);
    }


    void print_to_console_def_use_chains() {
        for (auto &[def, uses] : definition_to_uses) {
            auto &[d_id, d_quad] = def.location;
            fmt::print("{}({},{}): ", def.name, f.id_to_block.at(d_id)->get_name(), d_quad);
            for (auto &use : uses) {
                auto &[u_id, u_quad] = use.location;
                fmt::print("{}({},{}), ", use.name, f.id_to_block.at(u_id)->get_name(), u_quad);
            }
            fmt::print("\n");
        }
    }

    std::vector<char> print_use_def_chains() {
        GraphWriter dot_writer;
        for (const auto &[use, definitions] : use_to_definitions) {

            auto &use_q = f.get_quad(use.location);

            auto [u_b, u_q] = use.location;
            auto loc_str = fmt::format("({}, {})", f.id_to_block.at(u_b)->get_name(), u_q);
            auto src_name = fmt::format("{}| {}", loc_str, escape_string(use_q.fmt(false)));
            dot_writer.set_node_text(src_name, {});

            for (const auto &def : definitions) {
                auto &def_q = f.get_quad(def.location);

                auto [d_b, d_q] = def.location;
                loc_str = fmt::format("({}, {})", f.id_to_block.at(d_b)->get_name(), d_q);
                auto dst_name = fmt::format("{}| {}", loc_str, escape_string(def_q.fmt(false)));
                dot_writer.set_node_text(dst_name, {});

                dot_writer.add_edge(src_name, dst_name, {{"label", def_q.dest->name}});
            }
        }

        std::string filename = "graphs/use_def_chains.png";
        dot_writer.set_title("UseDef Chain");
        auto image_data = dot_writer.render_to_file(filename);
#ifdef DISPLAY_GRAPHS
        system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
#endif
        return image_data;
    }
};

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_USE_DEF_GRAPH_HPP
