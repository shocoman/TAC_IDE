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
