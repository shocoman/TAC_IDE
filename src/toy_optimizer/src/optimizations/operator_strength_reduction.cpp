//
// Created by shoco on 2/16/2021.
//

#include "operator_strength_reduction.hpp"

void OSRDriver::fill_in_use_def_graph() {
    for (auto &b : f.basic_blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];

            VariableInfo::Place place{b->id, q_index};
            if (!q.is_jump() && q.dest.has_value())
                ir.use_def_graph[q.dest->name].defined_at = place;

            for (const auto &op_name : q.get_rhs(true))
                ir.use_def_graph[op_name]; // insert dummy value if doesnt exist
        }
    }
}

void OSRDriver::run_osr() {
    // visit every unvisited node in ssa graph

    std::vector<std::pair<std::string, VariableInfo>> ssa_nodes;
    for (auto &[name, var_info] : ir.use_def_graph)
        ssa_nodes.emplace_back(name, var_info);

    std::sort(ssa_nodes.begin(), ssa_nodes.end(),
              [](auto &n1, auto &n2) { return n1.second.defined_at < n2.second.defined_at; });

    for (auto &[name, var_info] : ssa_nodes)
        if (!var_info.visited)
            DFS(name);
}

void OSRDriver::DFS(const std::string &name) {
    // Tarjan algorithm for finding strongly connected components in graph
    static std::vector<std::string> stack;
    static int next_num = 0;

    auto &n = ir.use_def_graph.at(name);
    n.num = next_num++;
    n.visited = true;
    n.lowlink = n.num;

    // skip constant nodes (nodes w/o definition)
    if (n.defined_at == std::pair{-1, -1}) {
        n.header.clear();
        return;
    }

    stack.push_back(name);

    auto q = f.get_quad(n.defined_at);
    for (auto &op_name : q.get_rhs(true)) {
        auto &o = ir.use_def_graph.at(op_name);
        if (!o.visited) {
            DFS(op_name);
            n.lowlink = std::min(n.lowlink, o.lowlink);
        }
        if (o.num < n.num && std::find(stack.begin(), stack.end(), op_name) != stack.end())
            n.lowlink = std::min(n.lowlink, o.num);
    }

    if (n.lowlink == n.num) {
        // collect strongly connected component
        std::vector<std::string> SCC;
        std::string x;
        do {
            x = stack.back();
            stack.pop_back();
            SCC.push_back(x);
        } while (x != name);
        // if (SCC.size() > 1)
        fmt::print("SCC: {}\n", SCC);

        ProcessSCC(SCC);
    }
}

void OSRDriver::ProcessSCC(const std::vector<std::string> &SCC) {
    if (SCC.size() == 1) {
        std::string n = SCC.front();
        if (IsCandidateOperation(n, ir.use_def_graph.at(n).header.empty() ? n : ir.use_def_graph.at(n).header))
            Replace(n);
        else
            ir.use_def_graph.at(n).header.clear();
    } else
        ClassifyIV(SCC);
}

void OSRDriver::ClassifyIV(const std::vector<std::string> &SCC) {
    // choose node with lowest rpo number as a header
    std::string header = *std::min_element(SCC.begin(), SCC.end(), [&](auto &n1, auto &n2) {
        return ir.use_def_graph.at(n1).num < ir.use_def_graph.at(n2).num;
    });

    if (IsSCCValidIV(SCC, header)) {
        for (auto &node_name : SCC)
            ir.use_def_graph.at(node_name).header = header;
    } else {
        for (auto &node_name : SCC) {
            if (IsCandidateOperation(node_name, header))
                Replace(node_name);
            else
                ir.use_def_graph.at(node_name).header.clear();
        }
    }
}

bool OSRDriver::IsCandidateOperation(const std::string &node_name, std::string header) {
    auto &q = f.get_quad(ir.use_def_graph.at(node_name).defined_at);
    if (q.type == Quad::Type::Add || q.type == Quad::Type::Sub || q.type == Quad::Type::Mult) {
        return q.ops[0].is_var() && IsRegionConst(q.ops[1].value, header) ||
               q.ops[1].is_var() && IsRegionConst(q.ops[0].value, header) && q.type != Quad::Type::Sub;
    }
    return false;
}

bool OSRDriver::IsRegionConst(std::string &name, std::string &header) {
    // literally constant
    if (ir.use_def_graph.at(name).defined_at.first == -1)
        return true;

    // region constant should strictly dominate header
    auto iv_def_block = ir.use_def_graph.at(header).defined_at.first;
    auto constant_def_block = ir.use_def_graph.at(name).defined_at.first;
    return constant_def_block != iv_def_block &&
           ir.id_to_doms.at(iv_def_block).count(constant_def_block) > 0;
};

bool OSRDriver::IsSCCValidIV(const std::vector<std::string> &SCC, std::string header) {
    for (auto &node_name : SCC) {
        auto &q = f.get_quad(ir.use_def_graph.at(node_name).defined_at);
        if (q.type != Quad::Type::PhiNode && q.type != Quad::Type::Add && q.type != Quad::Type::Sub &&
            q.type != Quad::Type::Assign) {
            return false;
        } else {
            for (auto &op : q.get_rhs(true)) {
                if (std::find(SCC.begin(), SCC.end(), op) == SCC.end() && !IsRegionConst(op, header))
                    return false;
            }
        }
    }

    return true;
}

void OSRDriver::PrintSSAGraph() {
    GraphWriter dot_writer;
    std::unordered_set<std::string> visited;
    // print edges
    for (const auto &[node_name, val] : ir.use_def_graph) {

        if (visited.find(node_name) == visited.end()) {
            visited.insert(node_name);

            if (val.defined_at == std::pair{-1, -1}) {
                dot_writer.set_node_text(node_name, {});
                continue;
            }

            auto &q = f.get_quad(val.defined_at);
            dot_writer.set_node_text(node_name, {escape_string(q.fmt(true))});

            for (auto &s : q.get_rhs(true))
                dot_writer.add_edge(node_name, s);
        }
    }

    std::string filename = "graphs/ssa_graph.png";
    dot_writer.set_title("SSA Graph");
    dot_writer.render_to_file(filename);
#ifdef DISPLAY_GRAPHS
    system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
#endif
}

void OSRDriver::Replace(const std::string &node_name) {
    auto &q = f.get_quad(ir.use_def_graph.at(node_name).defined_at);

    auto &o1 = ir.use_def_graph.at(q.ops[0].value);
    auto &o2 = ir.use_def_graph.at(q.ops[1].value);
    auto [induction_var, region_constant] =
        !o2.header.empty() && IsRegionConst(q.ops[0].value, o2.header) ? std::pair{q.ops[1], q.ops[0]}
                                                                       : std::pair{q.ops[0], q.ops[1]};

    fmt::print("Replace: {}; {}; {} => {}\n", node_name, induction_var.value, region_constant.value,
               q.fmt());

    fmt::print("Before => Suspicious quad: {}\n", q.fmt());

    auto result = Reduce(node_name, induction_var, region_constant);
    fmt::print("After =>\n");
    //    fmt::print("Suspicious quad: {}\n", q.fmt());

    auto &quad = f.get_quad(ir.use_def_graph.at(node_name).defined_at);
    quad.type = Quad::Type::Assign;
    quad.clear_op(1);
    quad.ops[0] = Operand(result, Operand::Type::Var);
    ir.use_def_graph.at(node_name).header = ir.use_def_graph.at(induction_var.value).header;
}

std::string OSRDriver::Reduce(const std::string &node_name, Operand &induction_var, Operand &reg_const) {
    auto op_type = f.get_quad(ir.use_def_graph.at(node_name).defined_at).type;

    auto key = std::tuple{node_name, induction_var.value, reg_const.value};
    if (ir.operations_lookup_table.count(key) == 0) {
        std::string new_name = ir.new_name_generator->make_new_name();
        ir.operations_lookup_table[key] = new_name;

        // make copy of induction variable operation ???
        auto &iv_def = ir.use_def_graph.at(induction_var.value);
        auto quad_copy = f.get_quad(iv_def.defined_at);
        quad_copy.dest = Dest(new_name, {}, Dest::Type::Var);

        // insert it into the block after induction variable ???
        auto &block = f.id_to_block.at(iv_def.defined_at.first);
        block->quads.insert(block->quads.begin() + iv_def.defined_at.second + 1, quad_copy);
        if (quad_copy.type == Quad::Type::PhiNode)
            block->phi_functions++;
        fill_in_use_def_graph(); // update ssa graph
        auto &new_def = ir.use_def_graph[new_name];
        new_def.header = iv_def.header;

        //        new_def.defined_at = iv_def.defined_at;
        //        new_def.used_at = iv_def.used_at;

        for (auto &o : f.get_quad(new_def.defined_at).ops) {
            if (ir.use_def_graph.at(o.value).header == iv_def.header && !iv_def.header.empty()) {
                // rewrite O with Reduce(node_name, o, region_constant)
                auto res = Reduce(node_name, o, reg_const);
                auto pred = o.phi_predecessor;
                o = Operand(res);
                o.phi_predecessor = pred;
            } else if (op_type == Quad::Type::Mult ||
                       f.get_quad(new_def.defined_at).type == Quad::Type::PhiNode) {
                // replace O with Apply(node_name, o, region_constant)
                auto res = Apply(node_name, o, reg_const);
                auto pred = o.phi_predecessor;
                o = Operand(res);
                o.phi_predecessor = pred;
            }
        }
    }
    return ir.operations_lookup_table.at(key);
}

std::string OSRDriver::Apply(const std::string &node_name, Operand &op1, Operand &op2) {
    auto key = std::tuple{node_name, op1.value, op2.value};
    if (ir.operations_lookup_table.count(key) == 0) {
        auto &o1 = ir.use_def_graph.at(op1.value);
        auto &o2 = ir.use_def_graph.at(op2.value);

        if (!o1.header.empty() && IsRegionConst(op2.value, o1.header)) {
            return Reduce(node_name, op1, op2);
        } else if (!o2.header.empty() && IsRegionConst(op1.value, o2.header)) {
            return Reduce(node_name, op2, op1);
        } else {
            std::string new_name = ir.new_name_generator->make_new_name();
            ir.operations_lookup_table[key] = new_name;
            auto q = Quad(op1, op2, f.get_quad(ir.use_def_graph.at(node_name).defined_at).type);
            q.dest = Dest(new_name, {}, Dest::Type::Var);

            auto [op1_block, op1_quad] = ir.use_def_graph.at(op1.value).defined_at;
            auto [op2_block, op2_quad] = ir.use_def_graph.at(op2.value).defined_at;

            if (op1.is_constant() && op2.is_constant()) {
                constant_folding(q);
                auto &block = f.id_to_block.at(ir.use_def_graph.at(node_name).defined_at.first);
                block->quads.insert(block->quads.begin() + block->phi_functions, q);
            } else if (op1.is_constant()) {
                auto &block = f.id_to_block.at(op2_block);
                block->quads.insert(block->quads.begin() + op2_quad + 1, q);
            } else if (op2.is_constant()) {
                auto &block = f.id_to_block.at(op1_block);
                block->quads.insert(block->quads.begin() + op1_quad + 1, q);
            } else if (is_dominated_by(ir.id_to_doms, op1_block, op2_block)) {
                // insert after op1
                auto &block = f.id_to_block.at(op1_block);
                block->quads.insert(block->quads.begin() + op1_quad + 1, q);
            } else if (is_dominated_by(ir.id_to_doms, op2_block, op1_block)) {
                // insert after op2
                auto &block = f.id_to_block.at(op2_block);
                block->quads.insert(block->quads.begin() + op2_quad + 1, q);
            } else {
                // Insert into postdominator of two blocks
                assert("We shouldn't be here!");
            }

            ir.use_def_graph.at(new_name).header.clear();
            fill_in_use_def_graph();
        }
    }

    return ir.operations_lookup_table.at(key);
}
