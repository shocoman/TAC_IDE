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
