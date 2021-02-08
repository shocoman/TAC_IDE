#include <vector>
#include <iostream>
#include <algorithm>
#include <list>
#include <map>

struct LoopFinder {
    std::vector<int> stack;
    std::map<int, std::list<int>> a;
    std::vector<std::list<int>> b;
    std::vector<bool> blocked;
    int n;
    int s;

    std::vector<std::vector<int>> loops;

    static void Start() {
        std::map<int, std::list<int>> a;
        a.emplace(0, std::list<int>{1}); // 0
        a.emplace(1, std::list<int>{2, 3});  // 1
        a.emplace(2, std::list<int>{1});  // 2
        a.emplace(3, std::list<int>{4}); // 3
        a.emplace(4, std::list<int>{}); // 4

        LoopFinder cf = LoopFinder(a);
        cf.find();

        std::cout << "Unique loops: " << std::endl;
        for (const auto &loop : cf.loops) {
            for (const auto &el : loop) {
                std::cout << el << " ";
            }
            std::cout << std::endl;
        }

    }

    LoopFinder(const std::map<int, std::list<int>> &a) : a(a), n(a.size()) {
        for (int i = 0; i < n; i++) {
            b.emplace_back();
            blocked.emplace_back(false);
        }
    }

    void unblock(int u) {
        blocked[u] = false;
        while (!b[u].empty()) {
            int w = b[u].front();
            b[u].pop_front();
            if (blocked[w]) unblock(w);
        }
    }

    bool circuit(int v) {
        bool f = false;
        stack.push_back(v);
        blocked[v] = true;
        // L1
        for (int w : a[v]) {
            if (w == s) {
                add_loop();
                f = true;
            } else if (!blocked[w])
                if (circuit(w)) f = true;
        }

        // L2
        if (f)
            unblock(v);
        else {
            for (int w : a[v])
                if (std::find(b[w].begin(), b[w].end(), v) == b[w].end())
                    b[w].push_back(v);
        }
        stack.pop_back();
        return f;
    }

    void add_loop() {
        std::vector<int> loop;
        for (int i : stack) {
//            std::cout << i << " ";
            loop.push_back(i);
        }
        loops.emplace_back(loop);
//        std::cout << s << std::endl;
    }

    void find() {
        s = 0;
        while (s < n) {
            if (!a.empty()) {
                for (auto &[i, l] : a) {
                    b[i].clear();
                    blocked[i] = false;
                }
                // L3
                circuit(s);
                a.erase(s);
                for (auto &[i, l] : a)
                    if (auto it = std::find(a[i].begin(), a[i].end(), s); it != a[i].end())
                        a[i].erase(it);
                s++;
            } else
                s = n;
        }
    }
};


//void print_loops(BasicBlocks &nodes) {
//    // generate ids for node names
//    std::map<int, std::string> name_by_id;
//    std::map<std::string, int> id_by_name;
//    int counter = 0;
//    for (const auto &n : nodes) {
//        name_by_id[counter] = n->node_name;
//        id_by_name[n->node_name] = counter;
//        ++counter;
//    }
//
//    std::map<int, std::list<int>> adjacency_list;
//    for (const auto &n : nodes) {
//        adjacency_list[id_by_name[n->node_name]] = {};
//        for (const auto &s : n->successors) {
//            adjacency_list[id_by_name[n->node_name]].emplace_back(id_by_name[s->node_name]);
//        }
//    }
//
//    LoopFinder l = LoopFinder(adjacency_list);
//    l.find();
//
//    std::cout << "LOOPS: " << std::endl;
//    for (const auto &loop : l.loops) {
//        for (const auto &i : loop) {
//            std::cout << name_by_id.at(i) << " -> ";
//        }
//        std::cout << std::endl;
//    }
//}

void find_loop_bob(BasicBlock *B, std::unordered_map<int,int> id_to_rpo) {
    // prolog
//    functions[0].print_cfg("before.png");
//    auto id_to_rpo = functions[0].get_reverse_post_ordering();
//    find_loop(functions[0].find_exit_node(), id_to_rpo);

    std::vector<BasicBlock *> loop;
    std::vector<BasicBlock *> queue;

    for (auto &P : B->predecessors) {
        auto is_back_edge = id_to_rpo.at(B->id) > id_to_rpo.at(P->id);
        if (is_back_edge) {
            if (std::find(loop.begin(), loop.end(), P) == loop.end() && P != B) {
                queue.push_back(P);
                loop.push_back(P);
            }
        }
    }

    while (!queue.empty()) {
        auto X = queue[0];
        queue.erase(std::remove_if(queue.begin(), queue.end(), [&](auto a) { return a == X; }),
                    queue.end());

        for (auto &P : X->predecessors) {
            if (std::find(loop.begin(), loop.end(), P) == loop.end() && P != B) {
                queue.push_back(P);
                loop.push_back(P);
            }
        }
    }

    loop.push_back(B);

    std::cout << "Loop?" << std::endl;
    for (auto &b : loop) {
        std::cout << b->get_name() << ", ";
    }
    std::cout << std::endl;
}


std::unordered_set<int> construct_natural_loop_dragon(Function &function, int n, int d) {
    // n -> d: back edge
    auto id_to_doms = find_dominators(function);
    assert(function.id_to_block.at(n)->successors.count(function.id_to_block.at(d)));
    assert(id_to_doms.at(n).count(d) > 0); // assert that 'd' dominates 'n'

    auto id_to_post_order = function.get_post_ordering();

    std::unordered_set<int> visited = {d};
    std::function<void(BasicBlock *)> walker = [&](BasicBlock *b) {
        if (visited.count(b->id) == 0) {
            visited.insert(b->id);
            for (auto &s : b->predecessors)
                walker(s);
        }
    };
    walker(function.id_to_block.at(n));

    // region Print Nodes Inside the Loop
    std::cout << "Visited: " << std::endl;
    for (auto &a : visited) {
        std::cout << function.id_to_block.at(a)->get_name() << ", ";
    }
    std::cout << ' ' << std::endl;
    // endregion
    return visited;
}