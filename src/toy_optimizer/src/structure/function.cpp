//
// Created by shoco on 1/21/2021.
//

#include "function.hpp"

std::vector<char> Function::print_cfg(std::string filename,
                                      std::unordered_map<int, std::string> additional_info_above,
                                      std::unordered_map<int, std::string> additional_info_below,
                                      std::string title) {
    auto id_to_rpo = get_reverse_post_ordering();
    auto id_to_po = get_post_ordering();

    GraphWriter dot_writer;
    // print edges
    for (const auto &n : basic_blocks) {
        auto node_name = n->get_name();

        if (additional_info_above.count(n->id))
            dot_writer.add_info_above(node_name, additional_info_above.at(n->id), true);
        if (additional_info_below.count(n->id))
            dot_writer.add_info_above(node_name, additional_info_below.at(n->id), false);

        // print all quads as text
        std::vector<std::string> quad_lines;
        for (auto &q : n->quads) {
            quad_lines.emplace_back(escape_string(q.fmt()));
        }
        dot_writer.set_node_text(node_name, quad_lines);

        // attribute for correct branch display
        if (auto branch_target = n->get_jumped_to_successor();
            branch_target && n->successors.size() > 1) {
            dot_writer.set_attribute(node_name, "true_branch", branch_target->get_name());
        }

        // display subscript of the block
        int rpo = id_to_rpo.at(n->id);
        int po = id_to_po.at(n->id);
        dot_writer.set_attribute(node_name, "subscript",
                                 fmt::format("<BR/>id={};rpo={};po={}", n->id, rpo, po));

        // print edges
        for (auto &s : n->successors) {
            std::unordered_map<std::string, std::string> attributes = {
                {"label", s->lbl_name.value_or("")}, // {"color", "brown"}, {"style", "dashed"},
            };

            dot_writer.add_edge(node_name, s->get_name(), attributes);
        }
    }

    //    dot_writer.legend_marks = {{"Forward edge", "black", "solid"},
    //                               {"Back edge", "green", "dashed"},
    //                               {"Cross edge", "cyan", "dotted"}};

    filename = "graphs/" + filename;
    dot_writer.set_title(title.empty() ? filename : title);
    std::vector<char> image_data = dot_writer.render_to_file(filename);

#ifdef DISPLAY_GRAPHS
    system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
#endif
    return image_data;
}

void Function::print_to_console() const {
    for (auto &b : basic_blocks) {
        std::cout << " *** BasicBlock: " << b->node_name << "; " << b->lbl_name.value_or("NONE")
                  << "; Successors: " << b->successors.size()
                  << "; Predecessors: " << b->predecessors.size() << " \n"
                  << b->fmt();
    }
}

void Function::connect_blocks() {
    for (int i = 0; i < basic_blocks.size(); ++i) {
        auto &b = basic_blocks[i];
        if (i < basic_blocks.size() - 1 && b->allows_fallthrough()) {
            b->add_successor(basic_blocks[i + 1].get());
        }

        if (!b->quads.empty() && b->quads.back().is_jump()) {
            auto &jump_target = b->quads.back().dest->name;
            auto block = std::find_if(basic_blocks.begin(), basic_blocks.end(), [&](auto &e) {
                return e->lbl_name.has_value() && e->lbl_name.value() == jump_target;
            });

            b->add_successor(block->get());
        }
    }
}

void Function::add_missing_jumps() {
    for (auto &b : basic_blocks) {
        if (b->quads.empty() || b->type != BasicBlock::Type::Normal)
            continue;
        auto &last_q = b->quads.back();
        if (last_q.type != Quad::Type::Return && !last_q.is_jump() && !b->successors.empty()) {
            Dest dest((*b->successors.begin())->lbl_name.value(), Dest::Type::JumpLabel);
            Quad jump({}, {}, Quad::Type::Goto, dest);
            b->quads.push_back(jump);
        }
    }
}

void Function::add_entry_and_exit_block() {
    bool has_entry = false, has_exit = false;
    for (auto &b : basic_blocks)
        if (b->type == BasicBlock::Type::Entry)
            has_entry = true;
        else if (b->type == BasicBlock::Type::Exit)
            has_exit = true;

    if (!has_entry) {
        auto entry_block = std::make_unique<BasicBlock>();
        entry_block->type = BasicBlock::Type::Entry;
        entry_block->add_successor(basic_blocks.front().get());
        basic_blocks.insert(basic_blocks.begin(), std::move(entry_block));
    }

    if (!has_exit) {
        // find blocks without successors (ending blocks) and connect them with exit block
        std::vector<BasicBlock *> final_blocks;
        for (auto &n : basic_blocks)
            if (n->successors.empty())
                final_blocks.push_back(n.get());

        // if there are no such blocks, get a block with a greatest reverse post order number
        if (final_blocks.empty()) {
            auto id_to_rpo = get_reverse_post_ordering();
            auto block = std::max_element(basic_blocks.begin(), basic_blocks.end(),
                             [&](auto &b1, auto &b2) { return id_to_rpo.at(b1->id) < id_to_rpo.at(b2->id); });
            final_blocks.push_back(block->get());
        }

        auto exit_block = std::make_unique<BasicBlock>();
        exit_block->type = BasicBlock::Type::Exit;

        for (auto &f : final_blocks)
            f->add_successor(exit_block.get());

        basic_blocks.push_back(std::move(exit_block));
    }
}

std::unordered_map<int, int> Function::get_reverse_post_ordering() const {
    std::unordered_map<int, int> block_id_to_rpo;
    int counter = 0;

    std::function<void(BasicBlock *)> postorder_traversal = [&](BasicBlock *b) {
        if (block_id_to_rpo.find(b->id) == block_id_to_rpo.end()) {
            block_id_to_rpo[b->id] = 0;
            for (auto &s : b->successors)
                postorder_traversal(s);
            block_id_to_rpo[b->id] = counter++;
        }
    };

    postorder_traversal(get_entry_block());

    // visit yet unvisited blocks
    for (auto &b : basic_blocks)
        postorder_traversal(b.get());

    for (auto &b : basic_blocks)
        block_id_to_rpo[b->id] = (counter - 1) - block_id_to_rpo.at(b->id);

    return block_id_to_rpo;
}

std::unordered_map<int, int> Function::get_post_ordering() const {
    // rpo on reversed graph
    std::unordered_map<int, int> block_id_to_po;
    int counter = 0;

    std::function<void(BasicBlock *)> postorder_traversal = [&](BasicBlock *b) {
        if (block_id_to_po.insert({b->id, 0}).second) {
            for (auto &s : b->predecessors)
                postorder_traversal(s);
            block_id_to_po[b->id] = counter++;
        }
    };
    postorder_traversal(get_exit_block());

    // visit yet unvisited blocks
    for (auto &b : basic_blocks)
        postorder_traversal(b.get());
    for (auto &b : basic_blocks)
        block_id_to_po[b->id] = (counter - 1) - block_id_to_po.at(b->id);
    return block_id_to_po;
}

void Function::reverse_graph() {
    for (auto &b : basic_blocks) {
        std::swap(b->successors, b->predecessors);
        if (b->type == BasicBlock::Type::Entry)
            b->type = BasicBlock::Type::Exit;
        else if (b->type == BasicBlock::Type::Exit)
            b->type = BasicBlock::Type::Entry;
    }
}

BasicBlock *Function::get_entry_block() const {
    for (const auto &b : basic_blocks)
        if (b->type == BasicBlock::Type::Entry)
            return b.get();
    return nullptr;
}

BasicBlock *Function::get_exit_block() const {
    for (const auto &b : basic_blocks)
        if (b->type == BasicBlock::Type::Exit)
            return b.get();
    return nullptr;
}

void Function::print_basic_block_info() const {
    for (const auto &b : basic_blocks) {
        fmt::print("ID: {:2}; LABEL NAME: {:4}; PREDS: {:2}; SUCCS: {:2}\n", b->id,
                   b->lbl_name.value_or("NONE"), b->predecessors.size(), b->successors.size());
    }
}

void Function::remove_blocks_without_predecessors() {
    auto entry = get_entry_block();
    for (auto it = basic_blocks.begin(); it != basic_blocks.end();) {
        if (it->get()->id != entry->id && it->get()->predecessors.empty()) {
            it = basic_blocks.erase(it);
        } else {
            ++it;
        }
    }
}

void Function::update_block_ids() {
    id_to_block.clear();
    int id = 0;
    for (auto &b : basic_blocks) {
        b->id = id++;
        id_to_block[b->id] = b.get();
    }
}

Quad &Function::get_quad(const std::pair<int, int> &loc) const {
    return id_to_block.at(loc.first)->quads.at(loc.second);
}

std::string Function::get_as_code() const {
    std::string code;
    bool has_function_name = false;
    for (auto &b : basic_blocks) {
        if (b->type == BasicBlock::Type::Normal) {
            if (function_name == b->get_name())
                has_function_name = true;
            code += fmt::format("\n{}:\n", b->get_name());
        }
        for (auto &q : b->quads)
            code += fmt::format("\t{}\n", q.fmt());
    }
    // add function name
    if (!has_function_name)
        code.insert(0, fmt::format("\n{}: // Function name", function_name));

//    fmt::print("{}\n", code);
    return code;
}

void Function::update_phi_predecessors_after_clone() {
    for (auto &b : basic_blocks)
        for (int i = 0; i < b->phi_functions; ++i) {
            auto &phi = b->quads[i];

            for (int op_i = phi.ops.size() - 1; op_i >= 0; --op_i) {
                auto &op = phi.ops[op_i];
                int pred_id = op.phi_predecessor->id;
                op.phi_predecessor = nullptr;
                for (auto &pred : b->predecessors)
                    if (pred->id == pred_id)
                        op.phi_predecessor = pred;
                // remove phi ops without connected block
                if (op.phi_predecessor == nullptr)
                    phi.ops.erase(phi.ops.begin() + op_i);
            }
        }
}
