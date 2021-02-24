//
// Created by shoco on 2/5/2021.
//

#include "dominators.hpp"

ID2DF find_dominance_frontier(const BasicBlocks &blocks, ID2IDOM &id_to_immediate_dominator) {
    std::unordered_map<int, std::unordered_set<int>> id_to_dominance_frontier;
    for (const auto &b : blocks)
        id_to_dominance_frontier[b->id] = {};
    for (const auto &b : blocks) {
        if (b->predecessors.size() > 1) {
            for (const auto &pred : b->predecessors) {
                int runner_id = pred->id;

                while (runner_id != id_to_immediate_dominator.at(b->id)) {
                    id_to_dominance_frontier[runner_id].insert(b->id);
                    runner_id = id_to_immediate_dominator.at(runner_id);
                }
            }
        }
    }
    return id_to_dominance_frontier;
}

ID2IDOM find_immediate_dominators(Function &function) {
    BasicBlocks &blocks = function.basic_blocks;
    ID2Block &id_to_block = function.id_to_block;

    std::unordered_map<int, int> id_to_rpo = function.get_reverse_post_ordering();

    // We use std::map here for its sorting capabilities
    // rpo_to_id will contain ids sorted in reverse post order.
    // It's important for the algorithm (convergence)
    std::map<int, int> rpo_to_id;
    for (auto &[id, rpo] : id_to_rpo)
        rpo_to_id.emplace(rpo, id);

    ID2IDOM id_to_idom;
    auto intersect = [&](BasicBlock *i, BasicBlock *j) {
        auto finger1 = i->id;
        auto finger2 = j->id;
        while (finger1 != finger2) {
            while (id_to_rpo.at(finger1) > id_to_rpo.at(finger2))
                finger1 = id_to_idom.at(finger1);
            while (id_to_rpo.at(finger2) > id_to_rpo.at(finger1))
                finger2 = id_to_idom.at(finger2);
        }
        return id_to_block.at(finger1);
    };

    auto entry_node_id = function.find_entry_block()->id;
    id_to_idom[entry_node_id] = entry_node_id;

    int iterations = 0;
    bool changed = true;
    while (changed) {
        changed = false;
        iterations++;

        for (auto &[rpo, id] : rpo_to_id) {
            auto &b = id_to_block.at(id);
            if (b->id == entry_node_id)
                continue;

            auto preds = b->predecessors.begin();
            while (preds != b->predecessors.end() &&
                   id_to_idom.find((*preds)->id) == id_to_idom.end())
                std::advance(preds, 1);
            if (preds == b->predecessors.end())
                continue;
            auto new_idom = *preds;

            for (std::advance(preds, 1); preds != b->predecessors.end(); std::advance(preds, 1)) {
                if (id_to_idom.find((*preds)->id) != id_to_idom.end())
                    new_idom = intersect(*preds, new_idom);
            }

            if (id_to_idom.find(b->id) == id_to_idom.end() ||
                id_to_idom.at(b->id) != new_idom->id) {
                id_to_idom[b->id] = new_idom->id;
                changed = true;
            }
        }
    }

    bool graph_is_irreducible = iterations > 2;
    // region Print IDom set
    //    std::cout << __PRETTY_FUNCTION__ << std::endl;
    //    std::cout << "Iterations: " << iterations << std::endl;
    //    for (auto &[id, idom_id] : id_to_idom) {
    //        std::cout << id_to_block.at(id)->get_name() << " -> " <<
    //        id_to_block.at(idom_id)->get_name()
    //                  << std::endl;
    //    }
    // endregion

    // entry node has no idom
    id_to_idom.erase(entry_node_id);
    return id_to_idom;
}

ID2DOMS find_dominators(Function &function) {
    // forward data-flow problem
    // n ← |N| - 1
    // Dom(0) ← {0}
    // for i ← 1 to n
    //   Dom(i) ← N
    // changed ← true
    // while (changed)
    //   changed ← false
    //      for i ← 1 to n
    //          temp ← {i} ∪ ( j ∈ preds(i) Dom( j ) )
    //          if temp ̸= Dom(i) then
    //              Dom(i) ← temp
    //              changed ← true

    auto &blocks = function.basic_blocks;
    auto &id_to_block = function.id_to_block;
    auto id_to_rpo = function.get_reverse_post_ordering();
    std::map<int, int> sorted_by_rpo_block_ids;
    for (auto &[id, rpo] : id_to_rpo)
        sorted_by_rpo_block_ids.emplace(rpo, id);

    std::unordered_map<int, std::unordered_set<int>> id_to_dominators;
    std::unordered_set<int> N;
    for (auto &b : blocks)
        N.insert(b->id);

    for (auto &b : blocks)
        id_to_dominators[b->id] = N;

    auto entry = function.find_entry_block();
    id_to_dominators[entry->id] = {entry->id};

    int iterations = 0;
    bool changed = true;
    while (changed) {
        changed = false;
        iterations++;

        for (auto &[_, id] : sorted_by_rpo_block_ids) {
            if (id == entry->id)
                continue;

            std::vector<std::unordered_set<int>> predecessors_doms;
            for (auto &pred : id_to_block.at(id)->predecessors)
                predecessors_doms.push_back(id_to_dominators.at(pred->id));
            auto pred_intersection = intersection_of_sets(predecessors_doms);

            pred_intersection.insert(id);
            if (pred_intersection != id_to_dominators[id]) {
                id_to_dominators[id] = pred_intersection;
                changed = true;
            }
        }
    }

    // region Print Dominators
//    std::cout << "-- PRINT --" << std::endl;
//    for (auto &[id, doms] : id_to_dominators) {
//        std::cout << id << " (" << function.id_to_block.at(id)->get_name() << "): ";
//        for (auto &d : doms) {
//            std::cout << function.id_to_block.at(d)->get_name() << ", ";
//        }
//        std::cout << std::endl;
//    }
//    std::cout << "-- END_PRINT --" << std::endl;
    // endregion

    return id_to_dominators;
}

void print_dominator_tree(ID2Block &id_to_block, ID2IDOM &id_to_idom) {
    GraphWriter writer;
    for (const auto &[id1, id2] : id_to_idom) {
        // make a connection between blocks[id1] and blocks[block_id]
        const auto name1 = id_to_block.at(id2)->node_name;
        const auto name2 = id_to_block.at(id1)->node_name;
        writer.set_node_name(name1, name1);
        writer.set_node_name(name2, name2);
        writer.set_node_text(name1, {});
        writer.set_node_text(name2, {});
        writer.add_edge(name1, name2);
    }

    writer.render_to_file("dominator_tree.png");
    system("feh dominator_tree.png &");
}


// check if 'b' dominates 'a'
bool is_dominated_by(const ID2DOMS &id_to_doms, int a_id, int b_id) {
    auto &dominators_of_a = id_to_doms.at(a_id);
    return dominators_of_a.find(b_id) != dominators_of_a.end();
}

int get_common_dominator_id(int Z_id, int B_id, const ID2IDOM &id_to_idom, const ID2DOMS &id_to_doms) {
    // pseudo code
    // if B dominates Z then
    //   return B
    // while Z does not dominate B do
    //   Z = idom(Z)
    // return Z

    if (is_dominated_by(id_to_doms, Z_id, B_id))
        return B_id;
    while (!is_dominated_by(id_to_doms, B_id, Z_id))
        Z_id = id_to_idom.at(Z_id);
    return Z_id;
}
