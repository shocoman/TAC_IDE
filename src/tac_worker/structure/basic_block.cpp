//
// Created by shoco on 10/22/2020.
//

#include "basic_block.hpp"

#include <utility>
std::string BasicBlock::get_name() const {
    if (lbl_name.has_value())
        return lbl_name.value();
    else if (!node_name.empty())
        return node_name;
    else
        return fmt::format("BB #{}", id);
}
std::string BasicBlock::fmt() const {
    std::string out;
    for (auto &q : quads)
        out += q.fmt() + "\n";
    return out;
}
void BasicBlock::add_successor(BasicBlock *s) {
    successors.emplace(s);
    s->predecessors.emplace(this);
}
void BasicBlock::remove_successors() {
    for (auto &s : successors)
        s->predecessors.erase(this);
    successors.clear();
}
void BasicBlock::remove_predecessors() {
    for (auto &s : predecessors)
        s->successors.erase(this);
    predecessors.clear();
}
BasicBlock *BasicBlock::get_fallthrough_successor() {
    auto s2 = get_name();
    if (successors.empty())
        return nullptr;
    else if (allows_fallthrough() && successors.size() == 1)
        return *successors.begin();

    auto &jump_target = quads.back().dest->name;
    for (auto &s : successors)
        if (!s->lbl_name.has_value() || *s->lbl_name != jump_target)
            return s;
    return nullptr;
}
BasicBlock *BasicBlock::get_jumped_to_successor() {
    if (quads.empty() || !quads.back().is_jump())
        return nullptr;

    auto &jump_target = quads.back().dest->name;
    for (auto &s : successors)
        if (s->lbl_name.has_value() && *s->lbl_name == jump_target)
            return s;
    return nullptr;
}

bool BasicBlock::allows_fallthrough() {
    return quads.empty() ||
           quads.back().type != Quad::Type::Goto && quads.back().type != Quad::Type::Return;
}

bool BasicBlock::has_phi_function(std::string name) {
    for (int i = 0; i < phi_functions; ++i) {
        if (quads[i].type == Quad::Type::PhiNode && quads[i].dest->name == name)
            return true;
    }
    return false;
}

Quad &BasicBlock::get_phi_function(std::string name) {
    for (int i = 0; i < phi_functions; ++i) {
        if (quads[i].type == Quad::Type::PhiNode && quads[i].dest.value().name == name)
            return quads[i];
    }
    return quads[0];
}

void BasicBlock::add_phi_function(std::string phi_name, const std::vector<std::string> &ops) {
    Quad phi({}, {}, Quad::Type::PhiNode);
    phi.dest = Dest(std::move(phi_name), {}, Dest::Type::Var);
    std::vector<Operand> operands;
    for (auto &n : ops)
        operands.emplace_back(n);
    phi.ops = operands;

    quads.insert(quads.begin() + phi_functions, phi);
    phi_functions++;
}

int BasicBlock::append_quad(Quad q) {
    if (quads.empty() || !quads.back().is_jump()) {
        quads.push_back(q);
        return quads.size() - 1;
    } else {
        quads.insert(quads.end() - 1, q);
        return quads.size() - 2;
    }
}

void BasicBlock::update_phi_positions() {
    std::vector<Quad> phi_nodes;
    std::copy_if(quads.begin(), quads.end(), std::back_inserter(phi_nodes),
                 [](auto &q) { return q.type == Quad::Type::PhiNode; });
    quads.erase(std::remove_if(quads.begin(), quads.end(),
                               [](auto &q) { return q.type == Quad::Type::PhiNode; }),
                quads.end());
    quads.insert(quads.begin(), phi_nodes.begin(), phi_nodes.end());
    phi_functions = phi_nodes.size();
}

void BasicBlock::print_phi_nodes() {
    for (auto &q : quads)
        if (q.type == Quad::Type::PhiNode) {
            std::vector<std::string> operators;
            for (auto &o : q.ops)
                operators.push_back(fmt::format("{}({})", o.value, o.phi_predecessor->get_name()));
            fmt::print("{} = phi {}", q.dest->name, operators);
        }
}
