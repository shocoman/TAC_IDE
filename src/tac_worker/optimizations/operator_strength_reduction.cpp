//
// Created by shoco on 2/16/2021.
//

#include "operator_strength_reduction.hpp"

OSRDriver::OSRDriver(Function &f) : func(f) {
    id_to_doms = find_dominators(f);
    FillInUseDefGraph();

    OSR();
    //    PrintSSAGraph();

    for (auto &[name, var_info] : useInfo) {
        fmt::print("{}:\n\t Defined at: {}\n\t Used at: {}\n\t Header: {}\nNum: {}\n", name,
                   var_info.defined_at, var_info.used_at, var_info.header, var_info.num);
    }
}

void OSRDriver::OSR() {
    // visit every unvisited node in ssa graph

    std::vector<std::pair<std::string, VariableInfo>> ssa_nodes;
    for (auto &[name, var_info] : useInfo)
        ssa_nodes.emplace_back(name, var_info);

    std::sort(ssa_nodes.begin(), ssa_nodes.end(),
              [](auto &n1, auto &n2) { return n1.second.defined_at < n2.second.defined_at; });

    //    for (auto &[name, var_info] : useInfo)
    for (auto &[name, var_info] : ssa_nodes)
        if (!var_info.visited)
            DFS(name);
}

void OSRDriver::FillInUseDefGraph() {
    for (auto &b : func.basic_blocks) {
        for (int q_index = 0; q_index < b->quads.size(); ++q_index) {
            auto &q = b->quads[q_index];

            VariableInfo::Place place{b->id, q_index};
            if (!q.is_jump() && q.dest.has_value())
                useInfo[q.dest->name].defined_at = place;

            for (const auto &op_name : q.get_rhs(true))
                useInfo[op_name].used_at.push_back(place);
        }
    }
}

auto OSRDriver::GetQuad(VariableInfo::Place p) -> Quad & {
    return func.id_to_block.at(p.first)->quads.at(p.second);
}

// IsCorrectInductionVariableAndRegionConstantPair
bool OSRDriver::IsCorrectIVarAndRConstPair(Operand &mb_iv, Operand &mb_rc) {
    // or loop invariant value
    auto &iv_node = useInfo.at(mb_iv.value);
    if (iv_node.header.empty() || !mb_iv.is_var())
        return false;

    if (mb_rc.is_constant())
        return true;

    // check if region constant definition dominates induction variable header definition
    auto iv_def_block = useInfo.at(iv_node.header).defined_at.first;
    auto constant_def_block = useInfo.at(mb_rc.value).defined_at.first;
    return id_to_doms.at(iv_def_block).count(constant_def_block) > 0;
}

bool OSRDriver::IsCandidateOperation(const std::string &node_name) {
    auto &q = GetQuad(useInfo.at(node_name).defined_at);
    if (q.type == Quad::Type::Add || q.type == Quad::Type::Sub || q.type == Quad::Type::Mult) {
        return IsCorrectIVarAndRConstPair(q.ops[0], q.ops[1]) ||
               IsCorrectIVarAndRConstPair(q.ops[1], q.ops[0]) && q.type != Quad::Type::Sub;
    }
    return false;
}

bool OSRDriver::IsValidUpdate(const std::string &node_name) {
    auto &q = GetQuad(useInfo.at(node_name).defined_at);

    bool induction_var_plus_constant =
        q.type == Quad::Type::Add && (IsCorrectIVarAndRConstPair(q.ops[0], q.ops[1]) ||
                                      IsCorrectIVarAndRConstPair(q.ops[1], q.ops[0]));
    bool induction_var_minus_constant =
        q.type == Quad::Type::Sub && IsCorrectIVarAndRConstPair(q.ops[0], q.ops[1]);

    return induction_var_plus_constant || induction_var_minus_constant ||
           q.type == Quad::Type::PhiNode || q.type == Quad::Type::Assign;
}

void OSRDriver::ClassifyIV(const std::vector<std::string> &SCC) {
    bool is_iv = true;
    for (auto &node_name : SCC)
        if (!IsValidUpdate(node_name))
            is_iv = false;

    if (is_iv) {
        // choose node with lowest rpo number as a header
        std::string header =
            *std::min_element(SCC.begin(), SCC.end(), [&](auto &node_name1, auto &node_name2) {
                return useInfo.at(node_name1).num < useInfo.at(node_name2).num;
            });
        for (auto &node_name : SCC)
            useInfo.at(node_name).header = header;
    } else {
        for (auto &node_name : SCC) {
            if (IsCandidateOperation(node_name))
                Replace(node_name);
            else
                useInfo.at(node_name).header.clear();
        }
    }
}

void OSRDriver::Process(const std::vector<std::string> &SCC) {
    if (SCC.size() == 1) {
        std::string n = SCC.front();
        if (IsCandidateOperation(n))
            Replace(n);
        else
            useInfo.at(n).header.clear();
    } else
        ClassifyIV(SCC);
}

void OSRDriver::DFS(const std::string &name) {
    // Tarjan algorithm for finding strongly connected components in graph
    static std::vector<std::string> stack;
    static int next_num = 0;

    auto &n = useInfo.at(name);
    n.num = next_num++;
    n.visited = true;
    n.low = n.num;

    // skip constant nodes (nodes w/o definition)
    if (n.defined_at == std::pair{-1, -1})
        return;

    stack.push_back(name);

    auto q = GetQuad(n.defined_at);
    for (auto &op_name : q.get_rhs(true)) {
        auto &o = useInfo.at(op_name);
        if (!o.visited) {
            DFS(op_name);
            n.low = std::min(n.low, o.low);
        }
        if (o.num < n.num && std::find(stack.begin(), stack.end(), op_name) != stack.end())
            n.low = std::min(n.low, o.num);
    }

    if (n.low == n.num) {
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

        Process(SCC);
    }
}

void OSRDriver::PrintSSAGraph() {
    GraphWriter dot_writer;
    std::unordered_set<std::string> visited;
    // print edges
    for (const auto &[node_name, val] : useInfo) {

        if (visited.find(node_name) == visited.end()) {
            visited.insert(node_name);

            if (val.defined_at == std::pair{-1, -1}) {
                dot_writer.set_node_text(node_name, {});
                continue;
            }

            auto &q = GetQuad(val.defined_at);
            dot_writer.set_node_text(node_name, {q.fmt(true)});

            for (auto &s : q.get_rhs(true))
                dot_writer.add_edge(node_name, s);
        }
    }

    std::string filename = "graphs/ssa_graph.png";
    dot_writer.set_title("SSA Graph");
    dot_writer.render_to_file(filename);
    system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
}

void OSRDriver::Replace(const std::string &node_name) {
    auto &q = GetQuad(useInfo.at(node_name).defined_at);

    auto [induction_var, region_constant] =
        q.ops[0].is_var() ? std::pair{q.ops[0], q.ops[1]} : std::pair{q.ops[1], q.ops[0]};

    induction_variables.insert(induction_var.value);
    fmt::print("Replace: {}; {}; {}\n", node_name, induction_var.value, region_constant.value);
    //    if (node_name == "s.2" || node_name[0] == '$') return;
    auto result = Reduce(node_name, induction_var, region_constant);

    q.type = Quad::Type::Assign;
    q.clear_op(1);
    q.ops[0] = Operand(result, Operand::Type::Var);
    //    fmt::print(" with: {}; {}\n", q.fmt(), GetQuad(useInfo.at(result).defined_at).fmt());
    //     replace n with a copy from result
    useInfo.at(node_name).header = useInfo.at(induction_var.value).header;
}

std::string OSRDriver::MakeNewName() {
    // find unused register name
    int i = 0;
    std::string register_name;
    do {
        register_name = fmt::format("$t{}", i++);
    } while (useInfo.find(register_name) != useInfo.end());
    useInfo[register_name] = {};
    return register_name;
}

std::string OSRDriver::Reduce(const std::string &node_name, Operand &induction_var,
                              Operand &region_constant) {
    auto op_type = GetQuad(useInfo.at(node_name).defined_at).type;

    auto key = std::tuple{node_name, induction_var.value, region_constant.value};
    if (operations_lookup_table.count(key) == 0) {
        std::string new_name = MakeNewName();
        operations_lookup_table[key] = new_name;

        auto &new_def = useInfo[new_name];
        auto &iv_def = useInfo.at(induction_var.value);
        new_def.defined_at = iv_def.defined_at;
        new_def.used_at = iv_def.used_at;
        new_def.header = iv_def.header;

        for (auto &o : GetQuad(new_def.defined_at).ops) {
            if (useInfo.at(o.value).header == iv_def.header) {
                // rewrite O with Reduce(node_name, o, region_constant)
                auto res = Reduce(node_name, o, region_constant);
                o.value = res;
            } else if (op_type == Quad::Type::Mult ||
                       GetQuad(new_def.defined_at).type == Quad::Type::PhiNode) {
                // replace O with Apply(node_name, o, region_constant)
                auto res = Apply(node_name, o, region_constant);
                o.value = res;
            }
        }
    }
    return operations_lookup_table.at(key);
}

std::string OSRDriver::Apply(const std::string node_name, Operand &op1, Operand &op2) {
    auto key = std::tuple{node_name, op1.value, op2.value};
    if (operations_lookup_table.count(key) == 0) {
        if (IsCorrectIVarAndRConstPair(op1, op2)) {
            return Reduce(node_name, op1, op2);
        } else if (IsCorrectIVarAndRConstPair(op2, op1)) {
            return Reduce(node_name, op2, op1);
        } else {
            std::string new_name = MakeNewName();
            operations_lookup_table[key] = new_name;
            auto q = Quad(op1, op2, GetQuad(useInfo.at(node_name).defined_at).type);
            q.dest = Dest(new_name, {}, Dest::Type::Var);

            int insert_block_id = -1;
            if (op1.is_constant() && op2.is_constant()) {
                constant_folding(q);
                insert_block_id = useInfo.at(node_name).defined_at.first;
            } else {
                int op1_def_block = useInfo.at(op1.value).defined_at.first,
                    op2_def_block = useInfo.at(op2.value).defined_at.first;

                if (op1_def_block == -1) {
                    insert_block_id = op2_def_block;
                } else if (op2_def_block == -1) {
                    insert_block_id = op1_def_block;
                } else {
                    func.reverse_graph();
                    auto id_to_rev_idom = find_immediate_dominators(func);
                    auto id_to_rev_doms = find_dominators(func);
                    int common_postdom_id = get_common_dominator_id(op1_def_block, op2_def_block,
                                                                    id_to_rev_idom, id_to_rev_doms);
                    insert_block_id = common_postdom_id;
                    func.reverse_graph();
                }
            }

            int pos = func.id_to_block.at(insert_block_id)->add_quad_before_jump(q);
            auto &def = useInfo.at(new_name);
            def.defined_at = std::pair{insert_block_id, pos};
            def.header.clear();
        }
    }

    return operations_lookup_table.at(key);
}
