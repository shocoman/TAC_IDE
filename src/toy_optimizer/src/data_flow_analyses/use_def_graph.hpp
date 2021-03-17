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

    struct IntermediateResults {
        std::optional<ReachingDefinitionsDriver> reaching_definitions;
    } ir;

    Function &f;
    std::map<Use, DefinitionsSet> use_to_definitions;

    UseDefGraph(Function &f_) : f(f_) { update_graph(); }

    void update_graph() {
        ir.reaching_definitions.emplace(f);
        ir.reaching_definitions->run();

        compute_use_def_chains();
    }

    void compute_use_def_chains() {
        auto &[IN, OUT] = ir.reaching_definitions->reaching_definitions;
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
                    name_to_defs[q.dest->name] = {
                        ir.reaching_definitions->ir.location_to_definition.at({b->id, quad_i})};
            }
        }
    }
};

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_DATA_FLOW_ANALYSES_USE_DEF_GRAPH_HPP
