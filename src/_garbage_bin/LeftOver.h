//
// Created by shoco on 10/9/2020.
//

#ifndef TAC_PARSER_LEFTOVER_H
#define TAC_PARSER_LEFTOVER_H



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


void andersen_points_to_analysis(Function &f) {
    std::map<std::string, std::set<std::string>> points_to;
    std::map<std::string, std::set<std::string>> deref_operator_edges;
    std::map<std::string, std::set<std::string>> deref_assign_edges;

    for (auto &b : f.basic_blocks)
        for (auto &q : b->quads) {
            if (!q.is_assignment())
                continue;

            auto &op = q.get_op(0)->value;
            auto &dest = q.dest->name;
            if (q.type == Quad::Type::Ref) { // a = &b
                points_to[dest].insert(op);
            } else if (q.type == Quad::Type::Assign && q.dest->type != Dest::Type::Deref) { // a = b
                for (auto &pnt : points_to[op])
                    points_to[dest].insert(pnt);
            } else if (q.type == Quad::Type::Deref) { // a = *b
                deref_operator_edges[op].insert(dest);
            } else if (q.type == Quad::Type::Assign && q.dest->type == Dest::Type::Deref) { // *a = b
                deref_assign_edges[dest].insert(op);
            }
        }

    std::queue<std::string> work_queue;
    for (auto &[v, pnt_to] : points_to)
        if (!pnt_to.empty())
            work_queue.push(v);

    while (!work_queue.empty()) {
        std::string v = work_queue.front();
        work_queue.pop();

        for (auto &a : points_to[v]) {
            for (auto &p : deref_operator_edges[v])
                if (points_to[a].insert(p).second) {
                    fmt::print("Edge: ({}, {})\n", a, p);
                    work_queue.push(a);
                }

            for (auto &q : deref_assign_edges[v])
                if (points_to[q].insert(a).second) {
                    fmt::print("Edge: ({}, {})\n", q, a);
                    work_queue.push(q);
                }
        }

        for (auto &q : points_to[v]) {
            bool changed = false;
            for (auto &z : points_to[v])
                changed |= points_to[q].insert(z).second;
            if (changed)
                work_queue.push(q);
        }
    }

    fmt::print("{}\n", points_to);
}


#endif //TAC_PARSER_LEFTOVER_H
