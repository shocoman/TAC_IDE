#include <iostream>

#include "driver/driver.hpp"
#include "tac_worker/dataflow_graph.hpp"



struct MyNode;

struct MyEdge {
    std::string name;
    MyNode *source;
    MyNode *sink;
    std::optional<MyEdge *> next_successor;
    std::optional<MyEdge *> next_predecessor;
};

struct MyNode {
    int data;
    std::string name;
    std::optional<MyEdge *> successor;
    std::optional<MyEdge *> predecessor;

//    MyNode(int d) : data(d) {}
    MyNode() : data(0) {}
};


void foo() {
    std::map<std::string, std::unique_ptr<MyNode>> nodes;
    std::map<std::string, std::unique_ptr<MyEdge>> edges;

    int n_counter = 1;
    int e_counter = 1;

    auto add_node = [&](MyNode *prev, MyNode *next) {
        MyNode* next_node = next;
        // check if node already exists in map
        if (next->name.empty() || nodes.find(next->name) == nodes.end()) {
            next->name = "N" + std::to_string(n_counter++);
            next_node = nodes.emplace(next->name, next).first->second.get();
        }

        auto e = std::make_unique<MyEdge>();
        e->name = "E" + std::to_string(e_counter++);
        e->source = prev;
        e->sink = next;

        auto succ = prev->successor;
        while (succ.has_value()) {succ = succ.value()->next_successor; }
        succ = e.get();

        auto pred = next->predecessor;
        while (pred.has_value()) {pred = pred.value()->next_predecessor; }
        pred = e.get();

        edges.emplace(e->name, std::move(e));

        return next_node;
    };


    auto n = std::make_unique<MyNode>();
    n->name = "N" + std::to_string(n_counter++);
    auto retn0 = nodes.emplace(n->name, std::move(n)).first->second.get();



    auto ret_n = add_node(retn0, new MyNode);
    auto snd_node2 = add_node(ret_n, new MyNode);
    auto thr_node = add_node(ret_n, new MyNode);


    std::cout << "Nodes" << std::endl;
    for (auto &[name, node] : nodes) {
        std::cout << name << std::endl;
    }

    std::cout << "Edges" << std::endl;
    for (auto &[name, edge] : edges) {
        std::cout << name << "; From: " << edge->source->name << " to " << edge->sink->name << std::endl;
    }
}





int main(int argc, char *argv[]) {
//    int res = 0;
    driver drv;
//    for (int i = 1; i < argc; ++i)
//        if (argv[i] == std::string ("-p"))
//            drv.trace_parsing = true;
//        else if (argv[i] == std::string ("-s"))
//            drv.trace_scanning = true;
//        else if (!drv.parse (argv[i]))
//            std::cout << "Parsing result: " << drv.result << '\n';
//        else
//            res = 1;

    drv.parse("../myfile");


//    std::map<int, std::string> labels_rev;
//    for (auto &[a, b] : drv.labels) {
////        std::cout << a << ": " << b << std::endl;
//        labels_rev.emplace(b, a);
//    }
//    int i = 0;
//    for (auto &q : drv.quadruples) {
//        if (auto lbl = labels_rev.find(i); lbl != labels_rev.end()) std::cout << lbl->second << ": \n";
//        std::cout << "  " << q.fmt() << std::endl;
//        i++;
//    }
//    func(drv);


foo();

    return 0;
}
