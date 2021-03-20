//
// Created by shoco on 2/27/2021.
//

#ifndef TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP
#define TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP

#include <map>
#include <set>

#include "../data_flow_analyses/data_flow_analyses.hpp"
#include "../data_flow_analyses/data_flow_framework.hpp"
#include "../structure/function.hpp"

struct CopyPropagationDriver {
    using Location = std::pair<int, int>;

    struct Assignment {
        Location location;
        std::string lhs, rhs;
        bool operator<(const Assignment &other) const { return location < other.location; }
        bool operator==(const Assignment &other) const { return location == other.location; }

        static std::string format(const Assignment &ass) {
            return fmt::format("{}={}{}", ass.lhs, ass.rhs, ass.location);
        }
    };

    using Assignments = std::set<Assignment>;
    using AssignmentsMap = std::map<int, Assignments>;
    using AssignmentsMapPair = std::pair<AssignmentsMap, AssignmentsMap>;
    struct IntermediateResults {
        Assignments all_assignments;
        AssignmentsMap id_to_gen, id_to_kill;
        AssignmentsMapPair copy_propagation_on_blocks;
        std::vector<std::tuple<std::string, std::string, Location>> replace_history;
    } ir;

    Function f;

    CopyPropagationDriver(Function &_f) : f(_f) {}

    void run_on_ssa() {
        auto id_to_rpo = f.get_reverse_post_ordering();
        std::map<int, int> rpo_to_id;
        for (auto &[id, rpo] : id_to_rpo)
            rpo_to_id[rpo] = id;

        std::unordered_map<std::string, std::string> copy_map;
        for (int i = 0; i <= 1; i++)
            for (auto &[rpo, id] : rpo_to_id) {
                auto &block = f.id_to_block.at(id);

                for (auto &q : block->quads) {
                    for (auto &op : q.ops) {
                        if (op.is_var())
                            if (copy_map.count(op.value) > 0)
                                op.value = copy_map.at(op.value);
                    }

                    if (q.is_assignment()) {
                        copy_map.erase(q.dest->name);
                        if (q.type == Quad::Type::Assign && q.get_op(0)->is_var())
                            copy_map[q.dest->name] = q.get_op(0)->value;
                    }
                }
            }
    }

    void run_on_non_ssa() {
        collect_all_assignments();
        calculate_gen_and_killed_assignments();
        calculate_copy_propagation_for_blocks();
        run_propagation();
    }

    void collect_all_assignments() {
        for (auto &b : f.basic_blocks)
            for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
                auto &q = b->quads[quad_i];
                if (q.type == Quad::Type::Assign && q.get_op(0)->is_var()) {
                    Location location = {b->id, quad_i};
                    ir.all_assignments.insert(Assignment{location, q.dest->name, q.get_op(0)->value});
                }
            }
    }

    void calculate_gen_and_killed_assignments() {
        for (auto &b : f.basic_blocks) {
            std::set<Assignment> local_gen, local_kill;
            std::set<std::string> redefined_defs;

            // go backwards to simplify 'local_gen' collecting
            for (int quad_i = b->quads.size() - 1; quad_i >= 0; --quad_i) {
                auto &q = b->quads[quad_i];
                Location location = {b->id, quad_i};

                if (q.type == Quad::Type::Assign && q.get_op(0)->is_var()) {
                    auto ass = Assignment{location, q.dest->name, q.get_op(0)->value};
                    // if it won't be killed in this block
                    if (redefined_defs.count(ass.lhs) == 0 && redefined_defs.count(ass.rhs) == 0)
                        local_gen.insert(ass);
                }

                if (q.is_assignment()) {
                    std::string def = q.dest->name;
                    redefined_defs.insert(def);

                    // find killed assignments in other blocks
                    for (auto &ass : ir.all_assignments) {
                        // don't count this block
                        if (b->id == ass.location.first)
                            continue;
                        if (def == ass.lhs || def == ass.rhs)
                            local_kill.insert(ass);
                    }
                }
            }

            ir.id_to_gen[b->id] = local_gen;
            ir.id_to_kill[b->id] = local_kill;
        }
    }

    std::vector<char> print_gen_killed_sets_on_graph() {
        return print_analysis_result_on_graph(f, ir.id_to_gen, ir.id_to_kill, "GenKill copies",
                                              Assignment::format, "Gen", "Kill");
    }

    std::vector<char> print_copy_propagation_for_block() {
        auto &[IN, OUT] = ir.copy_propagation_on_blocks;
        return print_analysis_result_on_graph(f, IN, OUT, "Copy propagated", Assignment::format);
    }

    void calculate_copy_propagation_for_blocks() {
        ir.copy_propagation_on_blocks = data_flow_framework<Assignment>(
            f, Flow::Forwards, Meet::Intersection, ir.all_assignments, [&](auto &IN, auto &OUT, int id) {
                auto X = IN.at(id);
                for (auto &d : ir.id_to_kill.at(id))
                    X.erase(d);
                for (auto &d : ir.id_to_gen.at(id))
                    X.insert(d);
                return X;
            });
    }

    void run_propagation() {
        // Propagation in block
        auto InvalidateCopy = [](auto &copy_map, auto &def) {
            for (auto it = copy_map.begin(); it != copy_map.end();) {
                if (it->first == def || it->second == def)
                    it = copy_map.erase(it);
                else
                    ++it;
            }
        };

        auto &IN = ir.copy_propagation_on_blocks.first;
        for (auto &b : f.basic_blocks) {
            std::map<std::string, std::string> dst_to_src;
            if (IN.count(b->id) > 0)
                for (auto &ass : IN.at(b->id))
                    dst_to_src[ass.lhs] = ass.rhs;

            for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
                auto &q = b->quads[quad_i];

                // replace operands
                for (auto &op : q.ops)
                    if (op.is_var())
                        if (dst_to_src.count(op.value) > 0) {
                            auto &replace = dst_to_src.at(op.value);
                            ir.replace_history.emplace_back(op.value, replace, Location{b->id, quad_i});
                            op.value = replace;
                        }

                // update copies
                if (q.is_assignment()) {
                    InvalidateCopy(dst_to_src, q.dest->name);
                    if (q.type == Quad::Type::Assign && q.get_op(0)->is_var())
                        dst_to_src[q.dest->name] = q.get_op(0)->get_string();
                }
            }
        }
    }
};

#endif // TAC_PARSER_SRC_TAC_WORKER_OPTIMIZATIONS_COPY_PROPAGATION_HPP
