//
// Created by shoco on 3/16/2021.
//

#ifndef TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_DATA_FLOW_ANALYSES_REACHING_DEFINITIONS_HPP
#define TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_DATA_FLOW_ANALYSES_REACHING_DEFINITIONS_HPP

struct ReachingDefinitionsDriver {
    using Location = std::pair<int, int>;
    struct Definition {
        Location location;
        std::string name;
        int counter;

        static std::string format(const Definition &def) {
            return fmt::format("{}(d{})", def.name, def.counter);
        }

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
    using DefinitionsSetMap = std::map<int, DefinitionsSet>;
    using PairOfDefinitionsSetMaps = std::pair<DefinitionsSetMap, DefinitionsSetMap>;

    struct IntermediateResults {
        std::vector<Definition> all_definitions;
        std::map<Location, Definition> location_to_definition;
        PairOfDefinitionsSetMaps get_kill_sets;
    } ir;

    Function &f;
    PairOfDefinitionsSetMaps reaching_definitions;

    ReachingDefinitionsDriver(Function &f_) : f(f_) {}

    PairOfDefinitionsSetMaps &run() {
        collect_definitions();
        calculate_gen_kill_sets();
        calculate_reaching_definitions();
        return reaching_definitions;
    }

    void collect_definitions() {
        int counter = 1;
        for (auto &b : f.basic_blocks)
            for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
                auto &q = b->quads[quad_i];
                if (q.is_assignment()) {
                    auto location = Location{b->id, quad_i};

                    auto definition = Definition{location, q.dest->name, counter++};
                    ir.all_definitions.push_back(definition);
                    ir.location_to_definition[location] = definition;
                }
            }
    }

    void calculate_gen_kill_sets() {
        DefinitionsSetMap id_to_gen, id_to_kill;
        for (auto &b : f.basic_blocks) {
            DefinitionsSet gen, kill;

            for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
                if (ir.location_to_definition.count({b->id, quad_i}) == 0)
                    continue;

                auto &local_def = ir.location_to_definition.at({b->id, quad_i});

                gen.insert(local_def);
                // find killed definitions
                for (auto &def : ir.all_definitions)
                    if (local_def.counter != def.counter && local_def.name == def.name) {
                        kill.insert(def);
                        gen.erase(def);
                    }
            }

            id_to_gen[b->id] = gen;
            id_to_kill[b->id] = kill;
        }
        ir.get_kill_sets = {id_to_gen, id_to_kill};
    }

    void calculate_reaching_definitions() {
        auto &[id_to_gen, id_to_kill] = ir.get_kill_sets;

        // forward data-flow analysis
        reaching_definitions = data_flow_framework<Definition>(f, Flow::Forwards, Meet::Union, {},
                                                               [&](auto &IN, auto &OUT, int id) {
                                                                   auto X = IN.at(id);
                                                                   for (auto &d : id_to_kill.at(id))
                                                                       X.erase(d);
                                                                   for (auto &d : id_to_gen.at(id))
                                                                       X.insert(d);
                                                                   return X;
                                                               });
    }

    std::vector<char> print_gen_kill_defs() {
        auto &[id_to_gen, id_to_kill] = ir.get_kill_sets;
        return print_analysis_result_on_graph(f, id_to_gen, id_to_kill, "KillGen definitions",
                                              Definition::format, "Gen", "Kill");
    }

    std::vector<char> print_reaching_definitions() {
        auto &[IN, OUT] = reaching_definitions;
        std::unordered_map<int, std::string> above, below;
        for (auto &[id, in] : IN) {
            auto s = fmt::format("IN: {}", print_into_string_with(in, Definition::format));
            above.emplace(id, split_long_string(s, 25));
        }
        for (auto &[id, out] : OUT) {
            auto s = fmt::format("OUT: {}", print_into_string_with(out, Definition::format));
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
                if (ir.location_to_definition.count(location) > 0)
                    escaped_string = fmt::format(
                        "d{}) {}", ir.location_to_definition.at(location).counter, escaped_string);
                quad_lines.push_back(escaped_string);
            }
            dot_writer.set_node_text(node_name, quad_lines);

            dot_writer.set_attribute(node_name, "subscript", fmt::format("<BR/>id={}", b->id));

            for (auto &s : b->successors)
                dot_writer.add_edge(node_name, s->get_name(), {{"label", s->lbl_name.value_or("")}});
        }

        std::string filename = "graphs/reach_defs.png";
        dot_writer.set_title("Reaching definitions");
        auto image_data = dot_writer.render_to_file(filename);
#ifdef DISPLAY_GRAPHS
        system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
#endif
        return image_data;
    }
};

#endif // TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_DATA_FLOW_ANALYSES_REACHING_DEFINITIONS_HPP
