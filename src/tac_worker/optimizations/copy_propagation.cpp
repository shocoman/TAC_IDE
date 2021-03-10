//
// Created by shoco on 2/27/2021.
//

#include "copy_propagation.hpp"

void copy_propagation_on_ssa(Function &f) {
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

void copy_propagation_on_non_ssa(Function &f) {
    // forward data-flow analysis
    using Location = std::pair<int, int>;
    struct Assignment {
        Location location;
        std::string lhs, rhs;
        bool operator<(const Assignment &rhs) const { return location < rhs.location; }
        bool operator==(const Assignment &rhs) const { return location == rhs.location; }
    };

    // collect all assignments (universal set)
    std::set<Assignment> all_assignments;
    for (auto &b : f.basic_blocks)
        for (int quad_i = 0; quad_i < b->quads.size(); ++quad_i) {
            auto &q = b->quads[quad_i];
            if (q.type == Quad::Type::Assign && q.get_op(0)->is_var()) {
                Location location = {b->id, quad_i};
                all_assignments.insert(Assignment{location, q.dest->name, q.get_op(0)->value});
            }
        }

    // collect gen and kill sets
    std::map<int, std::set<Assignment>> id_to_gen, id_to_kill;
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
                for (auto &ass : all_assignments) {
                    // don't count this block
                    if (b->id == ass.location.first)
                        continue;
                    if (def == ass.lhs || def == ass.rhs)
                        local_kill.insert(ass);
                }
            }
        }

        id_to_gen[b->id] = local_gen;
        id_to_kill[b->id] = local_kill;
    }

    auto [IN, OUT] = data_flow_framework<Assignment>(f, Flow::Forwards, Meet::Intersection,
                                                     all_assignments, [&](auto &IN, auto &OUT, int id) {
                                                         auto X = IN.at(id);
                                                         for (auto &d : id_to_kill.at(id))
                                                             X.erase(d);
                                                         for (auto &d : id_to_gen.at(id))
                                                             X.insert(d);
                                                         return X;
                                                     });

//    fmt::print("All copies: ");
//    for (auto &ass : all_assignments)
//        fmt::print("{}={}{}, ", ass.lhs, ass.rhs, ass.location);

//    auto print_assignment = [&](auto &ass) {
//        return fmt::format("{}={}{}", ass.lhs, ass.rhs, ass.location);
//    };
//    print_analysis_result_on_graph(f, id_to_gen, id_to_kill, "GenKill copies", print_assignment, "Gen",
//                                   "Kill");
//    print_analysis_result_on_graph(f, IN, OUT, "Copy propagated", print_assignment);


    // propagate
    for (auto &b : f.basic_blocks) {
        std::map<std::string, std::string> dst_to_src, src_to_dst;
        for (auto &ass : IN[b->id]) {
            dst_to_src[ass.lhs] = ass.rhs;
            src_to_dst[ass.rhs] = ass.lhs;
        }

        for (auto &q : b->quads) {
            for (auto &op : q.ops)
                if (op.is_var())
                    if (dst_to_src.count(op.value) > 0)
                        op.value = dst_to_src.at(op.value);

            if (q.is_assignment()) {
                auto def = q.dest->name;

                if (dst_to_src.count(def) > 0) {
                    src_to_dst.erase(dst_to_src.at(def));
                    dst_to_src.erase(def);
                }
                if (src_to_dst.count(def) > 0) {
                    dst_to_src.erase(src_to_dst.at(def));
                    src_to_dst.erase(def);
                }

                if (q.type == Quad::Type::Assign && q.get_op(0)->is_var()) {
                    dst_to_src[def] = q.get_op(0)->get_string();
                    src_to_dst[q.get_op(0)->get_string()] = def;
                }
            }
        }
    }
}
