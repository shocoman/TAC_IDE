//
// Created by shoco on 1/21/2021.
//

#include "function.hpp"

void Function::print_cfg(std::string filename,
                         std::unordered_map<int, std::string> additional_info_above,
                         std::unordered_map<int, std::string> additional_info_below,
                         std::string title) const {
    GraphWriter dot_writer;
    std::unordered_set<std::string> visited;
    // print edges
    for (const auto &n : basic_blocks) {
        auto node_name = n->get_name();

        if (additional_info_above.count(n->id))
            dot_writer.add_info_above(node_name, additional_info_above.at(n->id), true);
        if (additional_info_below.count(n->id))
            dot_writer.add_info_above(node_name, additional_info_below.at(n->id), false);
        if (visited.find(node_name) == visited.end()) {
            visited.insert(node_name);

            std::vector<std::string> quad_lines;

            // print all quads as text
            for (auto &q : n->quads) {
                quad_lines.emplace_back(q.fmt());
            }
            dot_writer.set_node_text(node_name, quad_lines);

            // print edges
            for (auto &s : n->successors) {
                dot_writer.add_edge(node_name, s->get_name(), s->lbl_name.value_or(""));
            }
        }
    }
    filename = "graphs/" + filename;
    dot_writer.set_title(title);
    dot_writer.render_to_file(filename);
    system(("sxiv -g 1000x1000+20+20 " + filename + " &").c_str());
}

void Function::print_to_console() const {
    for (auto &b : basic_blocks) {
        std::cout << " *** BasicBlock: " << b->node_name << "; " << b->lbl_name.value_or("NONE")
                  << "; Jumps to " << b->jumps_to.value_or("NONE")
                  << "; Successors: " << b->successors.size()
                  << "; Predecessors: " << b->predecessors.size() << " \n"
                  << b->fmt();
    }
}

void Function::connect_blocks() {
    for (int i = 0; i < basic_blocks.size(); ++i) {
        if (i != basic_blocks.size() - 1 && basic_blocks[i]->allows_fallthrough()) {
            basic_blocks[i]->add_successor(basic_blocks[i + 1].get());
        }
        if (auto jump_to = basic_blocks[i]->jumps_to; jump_to.has_value()) {
            auto block =
                std::find_if(basic_blocks.begin(), basic_blocks.end(), [&jump_to](auto &e) {
                    return e->lbl_name.has_value() && e->lbl_name.value() == jump_to.value();
                });

            basic_blocks[i]->add_successor(block->get());
        }
    }
}

void Function::add_missing_jumps() {
    for (int i = 0; i < basic_blocks.size() - 1; ++i) {
        if (basic_blocks[i]->quads.empty())
            continue;
        auto &last_q = basic_blocks[i]->quads.back();
        if (last_q.type != Quad::Type::Return && !last_q.is_jump() && !basic_blocks[i]->successors.empty()) {
            Dest dest((*basic_blocks[i]->successors.begin())->lbl_name.value(), {},
                      Dest::Type::JumpLabel);
            Quad jump({}, {}, Quad::Type::Goto, dest);
            basic_blocks[i]->quads.push_back(jump);
        }
    }
}

void Function::add_entry_and_exit_block() {
    // assume there is already one and only one entry block
    // you don't need to add another one
    auto entry_block = std::make_unique<BasicBlock>();
    entry_block->node_name = "Entry";
    entry_block->id = 0;
    entry_block->add_successor(basic_blocks.front().get());
    basic_blocks.insert(basic_blocks.begin(), std::move(entry_block));

    // find blocks without successors (ending blocks) and connect them with exit block
    // if there are more than 1
    std::vector<BasicBlock *> final_blocks;
    for (auto &n : basic_blocks)
        if (n->successors.empty())
            final_blocks.emplace_back(n.get());

    if (!final_blocks.empty()) {
        auto exit_block = std::make_unique<BasicBlock>();
        exit_block->node_name = "Exit";
        //        exit_block->id = basic_blocks.back()->id + 1;
        exit_block->id = 0;
//        exit_block->lbl_name = "EXIT_BLOCK";
        //        exit_block->quads.push_back(Quad(std::string("0"), {}, Quad::Type::Return));

        for (auto &f : final_blocks) {
            f->add_successor(exit_block.get());
        }

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

    for (auto &b : basic_blocks) {
        if (b->predecessors.empty())
            postorder_traversal(b.get());
    }

    for (auto &b : basic_blocks) {
        postorder_traversal(b.get());
    }

    for (auto &b : basic_blocks) {
        block_id_to_rpo[b->id] = counter - 1 - block_id_to_rpo.at(b->id);
        //        b->node_name = b->get_name() + "; RPO: " +
        //        std::to_string(block_id_to_rpo.at(b->id));
    }

    return block_id_to_rpo;
}

std::unordered_map<int, int> Function::get_post_ordering() {
    reverse_graph();
    std::unordered_map<int, int> id_to_post_order = get_reverse_post_ordering();
    reverse_graph();

    return id_to_post_order;
}

void Function::reverse_graph() {
    for (auto &b : basic_blocks)
        std::swap(b->successors, b->predecessors);
}

BasicBlock *Function::find_entry_block() const {
    for (const auto &b : basic_blocks)
        if (b->predecessors.empty())
            return b.get();
    return nullptr;
}

BasicBlock *Function::find_exit_block() const {
    for (const auto &b : basic_blocks)
        if (b->successors.empty())
            return b.get();
    return nullptr;
}

void Function::print_basic_block_info() const {
    for (const auto &b : basic_blocks) {
        std::cout << "ID: " << b->id << "; JUMPS TO: " << b->jumps_to.value_or("NOWHERE")
                  << std::endl;
    }
}

void Function::remove_blocks_without_predecessors() {
    auto entry = find_entry_block();
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
//        if (b->lbl_name.has_value()) b->node_name = b->lbl_name.value();
        id_to_block[b->id] = b.get();
    }
}
