//
// Created by shoco on 1/22/2021.
//

#include "sparse_simple_constant_propagation.hpp"

void sparse_simple_constant_propagation(BasicBlocks &blocks) {
    struct Place {
        int block_num;
        int quad_num;
    };

    struct VarInfo {
        enum class ValueType { Bottom, Constant, Top };

        Place defined_at = {-1, -1};
        std::vector<Place> used_at{};

        ValueType value_type = ValueType::Top;
        Operand constant;
    };

    using ValType = VarInfo::ValueType;
    using VarName = std::string;
    std::map<VarName, VarInfo> usingInfo;

    for (auto b_index = 0; b_index < blocks.size(); ++b_index) {
        auto &b = blocks[b_index];
        for (int i = 0; i < b->quads.size(); ++i) {
            auto &q = b->quads[i];
            if (!q.dest.has_value() || q.dest->type == Dest::Type::None)
                continue;

            Place place{b_index, i};
            //            if (!q.is_jump())
            usingInfo[q.dest->name].defined_at = place;

            if (Quad::is_foldable(q.type))
                constant_folding(q);

            for (const auto &op : q.ops) {
                if (op.is_var())
                    usingInfo[op.value].used_at.push_back(place);
            }
        }
    }

    // initialization phase
    std::vector<std::string> work_list;
    for (auto &[name, useInfo] : usingInfo) {
        if (useInfo.defined_at.block_num == -1)
            continue;

        auto &q = blocks.at(useInfo.defined_at.block_num)->quads.at(useInfo.defined_at.quad_num);

        if (q.type == Quad::Type::PhiNode) {
            useInfo.value_type = ValType::Top;
        } else if (q.type == Quad::Type::Assign && q.get_op(0)->is_constant()) {
            useInfo.value_type = ValType::Constant;
            useInfo.constant = q.get_op(0).value();
        } else {
            useInfo.value_type = ValType::Top;
        }

        if (useInfo.value_type != ValType::Top) {
            work_list.push_back(name);
        }
    }

    // propagation phase
    while (!work_list.empty()) {
        std::string name = work_list.back();
        work_list.pop_back();

        for (auto &[block_num, quad_num] : usingInfo.at(name).used_at) {
            auto &q = blocks.at(block_num)->quads.at(quad_num);
            if (q.is_jump())
                continue;

            auto &lhs = usingInfo.at(q.dest->name);
            if (lhs.value_type != ValType::Bottom) {
                auto tmp = std::pair{lhs.value_type, lhs.constant};
                auto &lhs_q =
                    blocks.at(lhs.defined_at.block_num)->quads.at(lhs.defined_at.quad_num);

                lhs.value_type = ValType::Top;
                lhs.constant = Operand();

                // interpretation over lattice
                if (lhs_q.type == Quad::Type::PhiNode) {
                    for (auto &op : lhs_q.ops) {
                        if (lhs.value_type == ValType::Bottom)
                            break;

                        auto &var = usingInfo.at(op.value);
                        if (var.value_type == ValType::Top) {
                        } else if (var.value_type == ValType::Constant &&
                                   (lhs.value_type == ValType::Top ||
                                    lhs.value_type == ValType::Constant &&
                                        lhs.constant.value == var.constant.value)) {
                            lhs.value_type = var.value_type;
                            lhs.constant = var.constant;
                        } else {
                            lhs.value_type = ValType::Bottom;
                        }
                    }
                } else {
                    //  if (lhs_q.ops.size() >= 2)

                    auto fst = lhs_q.get_op(0);
                    auto snd = lhs_q.get_op(1);
                    if (fst && fst->is_var() &&
                        usingInfo.at(fst->value).value_type == ValType::Constant)
                        fst = Operand(usingInfo.at(fst->value).constant);
                    if (snd && snd->is_var() &&
                        usingInfo.at(snd->value).value_type == ValType::Constant)
                        snd = Operand(usingInfo.at(snd->value).constant);

                    auto tmp_q = Quad();
                    tmp_q.ops = {fst.value_or(Operand()), snd.value_or(Operand())};
                    tmp_q.type = lhs_q.type;
                    auto prev = tmp_q;
                    constant_folding(tmp_q);

                    // SOMETHING CHANGED, UPDATE LHS
                    if (tmp_q != prev) {
                        lhs.value_type = ValType::Top;
                        lhs.constant = Operand();

                        // presume that lhs is unary
                        if (tmp_q.ops[0].is_constant()) {
                            lhs.value_type = ValType::Constant;
                            lhs.constant = tmp_q.ops[0];
                        } else {
                            auto saved = usingInfo.at(tmp_q.ops[0].value);
                            lhs.value_type = saved.value_type;
                            lhs.constant = saved.constant;
                        }
                    }
                }

                if (tmp != std::pair{lhs.value_type, lhs.constant}) {
                    work_list.push_back(q.dest->name);
                }
            }
        }
    }

    //    for (auto &[name, useInfo] : usingInfo) {
    //        std::cout << "Var: " << name << " defined at: (" << useInfo.defined_at.block_num << ";
    //        "
    //                  << useInfo.defined_at.quad_num << "); " << "used at: ";
    //        for (auto &u : useInfo.used_at) {
    //            std::cout << "(" << u.block_num << "; " << u.quad_num << "), ";
    //        }
    //        std::cout << " ValType: " << (int) useInfo.value_type << "; " <<
    //        useInfo.constant.value << std::endl;
    //    }

    for (auto &[name, useInfo] : usingInfo) {
        if (useInfo.value_type == ValType::Constant) {
            auto &q =
                blocks.at(useInfo.defined_at.block_num)->quads.at(useInfo.defined_at.quad_num);
            q.type = Quad::Type::Assign;
            q.ops[0] = useInfo.constant;
            q.clear_op(1);
        }
    }

    // update phi Function positions
    for (auto &b : blocks) {
        b->phi_functions = 0;
        for (int i = 0; i < b->quads.size(); ++i) {
            auto &q = b->quads[i];
            if (q.type == Quad::Type::PhiNode) {
                std::swap(b->quads[i], b->quads[b->phi_functions]);
                b->phi_functions++;
            }
        }
    }
}