//
// Created by shoco on 10/15/2020.
//

#ifndef TAC_PARSER_DAG_H
#define TAC_PARSER_DAG_H

#include <string>
#include <map>
#include <vector>
#include <random>
#include <ctime>

#include "../DotWriter/DotWriter.h"

enum OpType {
    None, Number, Literal, Add, Sub, Mult, Div,
    Assign,
};
const char *OpTypeText[] = {"None", "Number", "Literal", "Add", "Sub", "Mult", "Div",
                            "Assign",};

struct DagNode {
    std::string name;
    std::string postfix;

    OpType t;
    DagNode *l = nullptr;
    DagNode *r = nullptr;
    bool visited = false;

    DagNode *copy_of = nullptr;
    std::set<DagNode *> base_for;
    int used_times;

    std::string get_full_name(std::string delim = "_") const {
        if (t == Number) return name;
        if (copy_of) return copy_of->get_full_name();
        return name + delim + postfix;
    }

};


std::vector<DagNode *> nodes;
DotWriter writer;
std::map<std::string, std::vector<DagNode *>> last_used;
std::map<std::tuple<OpType, std::string, std::string>, DagNode *> signatures;


DagNode *add_node(const std::string& name, OpType t, std::string l, std::string r) {
    auto *n = new DagNode;
    n->name = name;
    n->postfix = std::to_string(last_used[name].size());
    n->t = t;

    if (!l.empty()) {
        if (last_used.find(l) != last_used.end()) {
            n->l = last_used[l].back();
            n->l->used_times++;
        } else
            n->l = add_node(l, (std::isdigit(l[0]) ? Number : Literal), "", "");
    }
    if (!r.empty()) {
        if (last_used.find(r) != last_used.end()) {
            n->r = last_used[r].back();
            n->r->used_times++;
        } else
            n->r = add_node(r, (std::isdigit(r[0]) ? Number : Literal), "", "");
    }


    std::tuple<OpType, std::string, std::string> signature
            {
                    t,
                    n->l ? n->l->get_full_name() : "",
                    n->r ? n->r->get_full_name() : ""
            };

    if ((!std::get<1>(signature).empty() || !std::get<2>(signature).empty())
        && signatures.find(signature) != signatures.end()) {
        auto node = signatures.at(signature);
        node->postfix += ", " + n->get_full_name();
        n->copy_of = node;
        node->base_for.emplace(n);
    } else {
        signatures[signature] = n;
    }

    last_used[name].emplace_back(n);

    return n;
}


void write_node_to_file(DagNode *n) {
    if (n->visited) return;
    n->visited = true;
    auto name = n->get_full_name();
    writer.set_node_text(name, {"Op: " + std::string(OpTypeText[n->t])});

    if (n->l) {
        write_node_to_file(n->l);
        writer.add_edge(name, n->l->get_full_name());
    }
    if (n->r) {
        write_node_to_file(n->r);
        writer.add_edge(name, n->r->get_full_name());
    }
}


void print(DagNode *n) {
    if (n->visited) return;
    n->visited = true;

    // dead code elimination
    if (!last_used[n->name].empty() && last_used[n->name].back() != n && n->used_times == 0) {
        return;
    }


    if (n->l) print(n->l);
    if (n->r) print(n->r);

    std::string o;
    switch (n->t) {
        case Assign:
            o = n->name + " = " + n->l->name;
            break;
        case Add:
            o = n->name + " = " + n->l->name + " + " + n->r->name;
            break;
        case Sub:
            o = n->name + " = " + n->l->name + " - " + n->r->name;
            break;
        case Mult:
            o = n->name + " = " + n->l->name + " * " + n->r->name;
            break;
        case Div:
            o = n->name + " = " + n->l->name + " / " + n->r->name;
            break;
        case Literal:
            o = "literal " + n->name;
            break;
        case Number:
            o = "number " + n->name;
            break;
        case None:
            break;
    }
    std::cout << o << std::endl;
}


static void common_subexpr_elimination() {
    static int tmp_counter = 0;

    for (int i = 0; i < nodes.size(); i++) {
        auto &n = nodes[i];
        if ((n->t == Add || n->t == Sub || n->t == Div || n->t == Mult) && n->base_for.size() > 1) {
            auto base_node = new DagNode{
                    "$tmp_" + std::to_string(tmp_counter++),
                    " - " + n->postfix,
                    n->t, n->l, n->r};
            for (auto &node : n->base_for) {
                node->copy_of = base_node;
                node->t = Assign;
                node->l = base_node;
                base_node->base_for.emplace(n);
            }
            n->base_for.clear();
            n->copy_of = base_node;
            n->t = Assign;
            n->l = base_node;

            nodes.insert(nodes.begin() + i, base_node);
        }
    }
}


static int test() {
//    nodes.emplace_back(add_node("a", Add, "b", "c"));
//    nodes.emplace_back(add_node("b", Sub, "a", "d"));
//    nodes.emplace_back(add_node("c", Add, "b", "c"));
//    nodes.emplace_back(add_node("d", Sub, "a", "d"));
    nodes.emplace_back(add_node("a", Add, "2", "3"));
//    nodes.emplace_back(add_node("b", Assign, "a", ""));
    nodes.emplace_back(add_node("a", Add, "2", "3"));
//    nodes.emplace_back(add_node("b", Add, "2", "3"));
    nodes.emplace_back(add_node("c", Add, "2", "3"));
    nodes.emplace_back(add_node("d", Add, "a", "b"));

    for (auto n : nodes) {
        if (!n->copy_of) {
            write_node_to_file(n);
        }
    }

    writer.render_to_file("dag.png");
    system("dag.png");


    common_subexpr_elimination();

    std::for_each(nodes.begin(), nodes.end(), [](auto n) { n->visited = false; });
    for (auto n : nodes) { print(n); }
}

#endif //TAC_PARSER_DAG_H
