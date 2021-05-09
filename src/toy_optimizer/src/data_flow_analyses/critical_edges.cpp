//
// Created by shoco on 5/7/2021.
//

#include "critical_edges.hpp"

void CriticalEdgesDriver::run() {
    ir.critical_edges.clear();
    for (auto &block : f.basic_blocks) {
        for (auto &succ : block->successors) {
            bool is_critical = block->successors.size() > 1 && succ->predecessors.size() > 1;
            if (is_critical)
                ir.critical_edges.insert({block->id, succ->id});
        }
    }
}

std::vector<char> CriticalEdgesDriver::print_critical_edges() {
    GraphWriter dot_writer;
    dot_writer.legend_marks = {{"Critical edge", "red", "solid"}, {"Regular edge", "black", "solid"}};

    for (const auto &n : ir.f_before_split.basic_blocks) {
        auto node_name = n->get_name();

        dot_writer.set_attribute(node_name, "subscript", fmt::format("id={}", n->id));

        // print all quads as text
        std::vector<std::string> quad_lines;
        for (int i = 0; i < n->quads.size(); ++i) {
            std::string s = escape_string(n->quads[i].fmt());
            quad_lines.emplace_back(s);
        }
        dot_writer.set_node_text(node_name, quad_lines);

        for (auto &s : n->successors) {
            std::unordered_map<std::string, std::string> attributes = {
                {"label", s->label_name.value_or("")},
            };
            if (ir.critical_edges.count({n->id, s->id}))
                attributes["color"] = "red";

            dot_writer.add_edge(node_name, s->get_name(), attributes);
        }
    }

    std::string filename = "graphs/critical_edges.png";
    dot_writer.set_title("Critical Edges");
    auto image_data = dot_writer.render_to_file(filename);
#ifdef DISPLAY_GRAPHS
    system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
#endif
    return image_data;
}

void CriticalEdgesDriver::split_critical_edges() {

    auto InsertAfter = [&](auto &&new_block, int after_block_id) {
        auto iter_pos = std::find_if(f.basic_blocks.begin(), f.basic_blocks.end(),
                                     [&](auto &b) { return b->id == after_block_id; });

        return f.basic_blocks.insert(iter_pos + 1, std::forward<decltype(new_block)>(new_block));
    };

    auto ReplaceJumpTargetWith = [&](BasicBlock *block, BasicBlock *prev_target, BasicBlock *new_target) {
        std::string prev_name = prev_target->get_name();

        auto jump_target = block->get_jumped_to_successor();
        if (jump_target != nullptr && jump_target->label_name.value_or("") == prev_name) {
            block->quads.back().dest->name = new_target->get_name();
        }

        block->successors.erase(prev_target);
        prev_target->predecessors.erase(block);
        block->add_successor(new_target);
        new_target->add_successor(prev_target);
    };

    std::unordered_map<BasicBlock *, BasicBlock *> jump_targets;
    for (auto &[a_id, b_id] : ir.critical_edges) {
        auto &a = f.id_to_block.at(a_id);
        auto &b = f.id_to_block.at(b_id);

        auto new_block = std::make_unique<BasicBlock>();
        auto jump_quad = Quad({}, {}, Quad::Type::Goto);
        jump_quad.dest = Dest(b->get_name(), Dest::Type::JumpLabel);
        new_block->quads.push_back(jump_quad);
        new_block->id = (int)f.basic_blocks.size();

        auto block_pos = InsertAfter(std::move(new_block), a->id);

        jump_targets[a] = block_pos->get();

        ReplaceJumpTargetWith(a, b, block_pos->get());
    }

    f.update_block_ids();

    for (auto &b : f.basic_blocks) {
        if (jump_targets.count(b.get()) > 0) {
            b->quads.back().dest->name = jump_targets.at(b.get())->get_name();
        }
    }
}
