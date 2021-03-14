//
// Created by shoco on 3/10/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_USE_DEF_GRAPH_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_USE_DEF_GRAPH_HPP

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "data_flow_analyses.hpp"
#include "data_flow_framework.hpp"
#include "../structure/function.hpp"

struct UseDefGraph {
    using Location = std::pair<int, int>;
    struct Definition {
        Location location;
        std::string name;
        int counter;

        bool operator<(const Definition &r) const { return location < r.location; }
        bool operator==(const Definition &r) const { return location == r.location; }
    };
    struct Use {
        Location location;
        std::string name;

        bool operator<(const Use &r) const {
            return std::tie(location, name) < std::tie(r.location, r.name);
        }
        bool operator==(const Use &r) const {
            return std::tie(location, name) == std::tie(r.location, r.name);
        }
    };
    using DefinitionsSet = std::set<Definition>;
    using DefinitionsSetMap = std::map<int, std::set<Definition>>;

    Function &f;
    std::vector<Definition> all_definitions;
    std::map<Location, Definition> location_to_definition;
    std::map<Use, DefinitionsSet> use_to_definitions;

    UseDefGraph(Function &f_) : f(f_) { update_graph(); }

    void update_graph() {
        all_definitions.clear();
        location_to_definition.clear();
        collect_definitions();
        compute_use_def_chains();
    }

    void collect_definitions() {
        int counter = 1;
        for (auto &b : f.basic_blocks) {
            for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
                auto &q = b->quads[quad_i];
                if (q.is_assignment()) {
                    auto location = Location{b->id, quad_i};

                    auto definition = Definition{location, q.dest->name, counter++};
                    all_definitions.push_back(definition);
                    location_to_definition[location] = definition;
                }
            }
        }
    }

    std::pair<DefinitionsSetMap, DefinitionsSetMap> get_gen_kill_sets() {
        DefinitionsSetMap id_to_gen, id_to_kill;
        for (auto &b : f.basic_blocks) {
            DefinitionsSet gen, kill;

            for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
                if (location_to_definition.count({b->id, quad_i}) == 0)
                    continue;

                auto &local_def = location_to_definition.at({b->id, quad_i});

                gen.insert(local_def);
                // find killed definitions
                for (auto &def : all_definitions)
                    if (local_def.counter != def.counter && local_def.name == def.name) {
                        kill.insert(def);
                        gen.erase(def);
                    }
            }

            id_to_gen[b->id] = gen;
            id_to_kill[b->id] = kill;
        }
        return {id_to_gen, id_to_kill};
    }

    std::pair<DefinitionsSetMap, DefinitionsSetMap> get_reaching_definitions() {
        auto [id_to_gen, id_to_kill] = get_gen_kill_sets();

        // forward data-flow analysis
        auto [IN, OUT] = data_flow_framework<Definition>(f, Flow::Forwards, Meet::Union, {},
                                                         [&](auto &IN, auto &OUT, int id) {
                                                             auto X = IN.at(id);
                                                             for (auto &d : id_to_kill.at(id))
                                                                 X.erase(d);
                                                             for (auto &d : id_to_gen.at(id))
                                                                 X.insert(d);
                                                             return X;
                                                         });
        return {IN, OUT};
    }

    void compute_use_def_chains() {
        auto [IN, OUT] = get_reaching_definitions();
        for (auto &b : f.basic_blocks) {
            auto in_definitions = IN[b->id];
            std::map<std::string, DefinitionsSet> name_to_defs;
            for (auto &def : in_definitions)
                name_to_defs[def.name].insert(def);

            for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
                auto &q = b->quads[quad_i];

                for (auto &op_name : q.get_rhs(false)) {
                    auto use = Use{{b->id, quad_i}, op_name};
                    use_to_definitions[use] = name_to_defs.at(op_name);
                }

                if (q.is_assignment())
                    name_to_defs[q.dest->name] = {location_to_definition.at({b->id, quad_i})};
            }
        }
    }

    void print_reaching_definitions() {
        // print graph
        auto format_def = [](auto &d) { return fmt::format("{}(d{})", d.name, d.counter); };

        // print gen-kill graph
        auto [id_to_gen, id_to_kill] = get_gen_kill_sets();
        print_analysis_result_on_graph(f, id_to_gen, id_to_kill, "KillGen definitions", format_def,
                                       "Gen", "Kill");

        // print reach-def graph
        auto [IN, OUT] = get_reaching_definitions();
        std::unordered_map<int, std::string> above, below;
        for (auto &[id, in] : IN) {
            auto s = fmt::format("IN: {}", print_into_string_with(in, format_def));
            above.emplace(id, split_long_string(s, 25));
        }
        for (auto &[id, out] : OUT) {
            auto s = fmt::format("OUT: {}", print_into_string_with(out, format_def));
            below.emplace(id, split_long_string(s, 25));
        }

        GraphWriter dot_writer;
        for (const auto &b : f.basic_blocks) {
            auto node_name = b->get_name();

            if (above.count(b->id))
                dot_writer.add_info_above(node_name, above.at(b->id), true);
            if (below.count(b->id))
                dot_writer.add_info_above(node_name, below.at(b->id), false);

            std::vector<std::string> quad_lines;
            for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
                auto &q = b->quads[quad_i];
                std::pair<int, int> location{b->id, quad_i};
                std::string escaped_string = escape_string(q.fmt());
                if (location_to_definition.count(location) > 0)
                    escaped_string = fmt::format("d{}) {}", location_to_definition.at(location).counter,
                                                 escaped_string);
                quad_lines.push_back(escaped_string);
            }
            dot_writer.set_node_text(node_name, quad_lines);

            dot_writer.set_attribute(node_name, "subscript", fmt::format("<BR/>id={}", b->id));

            for (auto &s : b->successors)
                dot_writer.add_edge(node_name, s->get_name(), {{"label", s->lbl_name.value_or("")}});
        }

        std::string filename = "graphs/reach_defs.png";
        dot_writer.set_title("Reaching definitions");
        dot_writer.render_to_file(filename);
        system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
    }
};

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_USE_DEF_GRAPH_HPP
