//
// Created by shoco on 1/22/2021.
//

#include "value_numbering.hpp"
#include "constant_folding.hpp"

void local_value_numbering(std::vector<Quad> &quads, ValueNumberTableStack &t) {
    for (auto &q : quads) {
        if (q.is_jump())
            continue;

        if (Quad::is_foldable(q.type))
            constant_folding(q);

        // generate and/or save value number for every operand
        std::vector<int> operand_values;
        for (auto &op : q.get_rhs_names()) {
            if (!t.get_value_number_by_name(op).has_value()) {
                t.set_value_number_for_name(op, t.current_number);
                t.set_name_for_value(t.current_number, op);
                if (q.type != Quad::Type::Assign)
                    t.current_number++;
            }
            operand_values.push_back(*t.get_value_number_by_name(op));
        }

        if (Quad::is_commutative(q.type))
            std::sort(operand_values.begin(), operand_values.end());
        auto op_hash_key = std::tuple{q.type, operand_values};

        // if hash key is already in the table replace current type with a copy operation
        int op_value;
        auto op = t.get_value_number_by_operation(op_hash_key);
        if (op && op == t.get_value_number_by_name(*t.get_name_by_value_number(*op))) {
            op_value = op.value();

            std::string name = *t.get_name_by_value_number(op_value);
            q = Quad(Operand(name), {}, Quad::Type::Assign, q.dest);
        }
        // otherwise insert new value number with hash key
        else {
            t.set_operation_value(op_hash_key, t.current_number);
            op_value = t.current_number;

            if (q.dest.has_value()) {
                t.set_name_for_value(t.current_number, q.dest->name);
                ++t.current_number;
            }
        }

        if (q.dest.has_value())
            t.set_value_number_for_name(q.dest->name, op_value);
    }
}

void superlocal_value_numbering(Function &function) {
    // value numbering on superblocks

    std::vector<BasicBlock *> work_list = {function.get_entry_block()};
    std::unordered_set<int> visited_blocks;

    using SVNFuncType = std::function<void(BasicBlock *, ValueNumberTableStack &)>;
    SVNFuncType SVN = [&](BasicBlock *b, ValueNumberTableStack &t) {
        t.push_table();
        local_value_numbering(b->quads, t);

        for (auto &s : b->successors)
            if (s->predecessors.size() == 1)
                SVN(s, t);
            else if (visited_blocks.insert(b->id).second)
                work_list.push_back(s);

        t.pop_table();
    };

    ValueNumberTableStack t;
    while (!work_list.empty()) {
        BasicBlock *b = work_list.back();
        work_list.pop_back();
        SVN(b, t);
    }
}

////////////////////////////////////////////////////////////////////////
/////////////// DOMINATOR BASED VALUE NUMBERING ////////////////////////
////////////////////////////////////////////////////////////////////////

void GlobalValueNumberingDriver::global_value_numbering(BasicBlock *b,
                                                        GlobalValueNumberingDriver::ValueNumberTableStack &t) {
    t.push_table();

    // process every phi function
    for (int i = 0; i < b->phi_functions; ++i) {
        auto &phi = b->quads.at(i);
        std::set<Operand> operands(phi.ops.begin(), phi.ops.end());
        // phi is meaningless (all operands are equal)
        if (std::equal(phi.ops.begin() + 1, phi.ops.end(), phi.ops.begin())) {
            ir.removed_quads.emplace_back(b->get_name(), phi.fmt());

            t.set_value_number_for_name(phi.dest->name, phi.ops[0].value);
            b->quads.erase(b->quads.begin() + i);
            --b->phi_functions;
            --i;
        }
        // or phi is redundant (same as one of the previous phi functions)
        else if (auto v = t.get_phi_node_by_operation(operands); v.has_value()) {
            ir.removed_quads.emplace_back(b->get_name(), phi.fmt());

            t.set_value_number_for_name(phi.dest->name, v.value());
            b->quads.erase(b->quads.begin() + i);
            --b->phi_functions;
            --i;
        } else {
            t.set_value_number_for_name(phi.dest->name, phi.dest->name);
            t.set_phi_node_for_value(operands, phi.dest->name);
        }
    }

    // work through each assignment of the form 'x = y op z'
    for (int quad_i = b->phi_functions; quad_i < b->quads.size(); ++quad_i) {
        auto &q = b->quads.at(quad_i);
        if (!q.is_assignment())
            continue;

        // overwrite 'x' and 'y' with saved value number
        for (auto &op : q.ops) {
            if (auto v = t.get_value_number_by_name(op.value); v.has_value())
                op = Operand(v.value());
        }

        if (Quad::is_foldable(q.type))
            constant_folding(q);

        std::vector<std::string> expr;
        for (auto &op : q.ops)
            expr.push_back(op.value);
        if (Quad::is_commutative(q.type))
            std::sort(expr.begin(), expr.end());
        auto op_hash_key = std::tuple{q.type, expr};

        auto v = t.get_value_number_by_operation(op_hash_key);
        if (v.has_value()) {
            ir.removed_quads.emplace_back(b->get_name(), q.fmt());

            t.set_value_number_for_name(q.dest->name, v.value());
            b->quads.erase(b->quads.begin() + quad_i);
            --quad_i;
        } else {
            t.set_value_number_for_name(q.dest->name, q.dest->name);
            t.set_operation_for_value_number(op_hash_key, q.dest->name);
        }
    }

    // update phi functions for each successor
    for (auto succ : b->successors) {
        for (int i = 0; i < succ->phi_functions; ++i) {
            auto &phi = succ->quads[i];
            for (auto &op : phi.ops) {
                auto v = t.get_value_number_by_name(op.value);
                if (v.has_value())
                    op = Operand(v.value(), op.type, op.phi_predecessor);
            }
        }
    }

    // call for each child in dominator tree
    for (auto &[child_id, parent_id] : ir.id_to_idom)
        if (b->id == parent_id)
            global_value_numbering(f.id_to_block.at(child_id), t);

    t.pop_table();
}

void GlobalValueNumberingDriver::run() {
    ir.id_to_idom = get_immediate_dominators(f);

    auto entry = f.get_entry_block();
    ValueNumberTableStack t;
    global_value_numbering(entry, t);
}
