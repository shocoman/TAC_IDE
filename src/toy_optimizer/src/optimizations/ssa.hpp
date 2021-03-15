//
// Created by shoco on 1/22/2021.
//

#ifndef TAC_PARSER_SSA_HPP
#define TAC_PARSER_SSA_HPP

#include <numeric>
#include <set>

#include "../data_flow_analyses/data_flow_analyses.hpp"
#include "../data_flow_analyses/dominators.hpp"
#include "../data_flow_analyses/print_graph.hpp"
#include "../structure/function.hpp"
#include "../utilities/graph_writer/graph_writer.hpp"
#include "../utilities/new_name_generator.hpp"
#include "../utilities/quad_preparation.hpp"
#include "value_numbering.hpp"

void place_phi_functions(Function &function,
                         std::map<std::string, std::set<BasicBlock *>> &var_to_blocks,
                         std::set<std::string> &global_names);
void rename_variables(Function &function, std::set<std::string> &global_names);

void convert_to_ssa(Function &function);
void convert_from_ssa(Function &function);

struct SSAConvertationDriver {
    struct {
        std::set<std::string> global_names, all_names;
        std::map<std::string, std::set<BasicBlock *>> var_to_block;
         Function  cfg_before_convert, cfg_after_phi_placement, cfg_after_renaming;
    } intermediate_results;

    Function f;

    SSAConvertationDriver(const Function &f_) : f(f_) {}

    void convert_to_ssa() {
        intermediate_results.cfg_before_convert = f;
        find_global_names();
        place_phi_functions();
        intermediate_results.cfg_after_phi_placement = f;
        rename_variables();
        intermediate_results.cfg_after_renaming = f;
    }

    void find_global_names() {
        auto &var_to_block = intermediate_results.var_to_block;
        auto &global_names = intermediate_results.global_names,
             &all_names = intermediate_results.all_names;
        for (auto &b : f.basic_blocks) {
            std::set<std::string> var_kill;
            for (const auto &q : b->quads) {
                for (auto &op : q.get_rhs(false))
                    global_names.insert(op);
                if (auto lhs = q.get_lhs(); lhs.has_value()) {
                    var_kill.insert(lhs.value());
                    var_to_block[lhs.value()].insert(b.get());
                    all_names.insert(lhs.value());
                }
            }
        }
    }

    void place_phi_functions() {
        auto &function = f;
        auto &var_to_blocks = intermediate_results.var_to_block;
        auto &global_names = intermediate_results.global_names;

        ID2IDOM id_to_idom = get_immediate_dominators(function);
        ID2DF id_to_dominance_frontier = get_dominance_frontier(function, id_to_idom);

        //    fmt::print("Global names: {}; All names: {}\n", global_names, all_names);
        //    fmt::print("Dominance Frontier:\n {}\n", id_to_dominance_frontier);

        auto [LiveIn, LiveOut] = live_variable_analyses(function);
        // place phi functions
        for (auto &name : global_names) {
            std::vector<BasicBlock *> work_list;
            auto &work_list_set = var_to_blocks.at(name);
            std::copy(work_list_set.begin(), work_list_set.end(), std::back_inserter(work_list));
            std::set<int> visited_blocks;

            for (int i = 0; i < work_list.size(); ++i) {
                for (auto &d : id_to_dominance_frontier.at(work_list[i]->id)) {
                    if (LiveIn.at(d).count(name) == 0)
                        continue;

                    auto d_block = function.id_to_block.at(d);
                    if (not d_block->has_phi_function(name)) {
                        d_block->add_phi_function(name, {});

                        if (visited_blocks.insert(d_block->id).second)
                            work_list.push_back(d_block);
                    }
                }
            }
        }
    }

    void rename_variables() {
        auto &function = f;
        auto &global_names = intermediate_results.all_names;

        char delim = '_';
        ID2IDOM id_to_idom = get_immediate_dominators(function);

        std::map<std::string, int> name_to_counter;
        std::map<std::string, std::vector<int>> name_to_stack;
        for (auto &name : global_names) {
            name_to_counter[name] = 0;
            name_to_stack[name] = {};
        }

        auto MakeNewSSAName = [&](auto &name) {
            int i = name_to_counter.at(name);
            name_to_counter.at(name)++;
            name_to_stack.at(name).push_back(i);
            return fmt::format("{}{}{}", name, delim, i);
        };

        std::function<void(int)> RenameVars = [&](int block_id) {
            std::vector<std::string> pushed_names;
            auto block = function.id_to_block.at(block_id);

            // rename phi functions
            for (int i = 0; i < block->phi_functions; ++i) {
                auto &phi = block->quads[i];
                pushed_names.push_back(phi.dest->name);
                phi.dest->name = MakeNewSSAName(phi.dest->name);
            }

            // rename other operations of form 'x = y + z'
            for (int i = block->phi_functions; i < block->quads.size(); ++i) {
                auto &q = block->quads[i];
                auto op1 = q.get_op(0), op2 = q.get_op(1);

                for (auto &op : q.ops)
                    if (op.is_var() && name_to_stack.count(op.value) > 0)
                        op.value += fmt::format("{}{}", delim, name_to_stack.at(op.value).back());

                if (q.dest && global_names.count(q.dest->name) > 0) {
                    pushed_names.push_back(q.dest->name);
                    q.dest->name = MakeNewSSAName(q.dest->name);
                }
            }

            // fill phi function parameters for each successor
            for (auto &s : block->successors) {
                for (int i = 0; i < s->phi_functions; ++i) {
                    auto &phi = s->quads[i];
                    auto name = phi.dest->name;
                    auto name_without_suffix = name.substr(0, name.find_first_of(delim, 0));

                    auto stack = name_to_stack.find(name_without_suffix);
                    std::string next_name =
                        (stack != name_to_stack.end() && !stack->second.empty())
                            ? name_without_suffix + fmt::format("{}{}", delim, stack->second.back())
                            : name;

                    Operand op(next_name, Operand::Type::Var, block);
                    phi.ops.push_back(op);
                }
            }

            // call for each successor in dominator tree
            for (auto &[child_id, parent_id] : id_to_idom)
                if (parent_id == block_id)
                    RenameVars(child_id);

            for (auto &n : pushed_names)
                name_to_stack.at(n).pop_back();
        };

        RenameVars(function.get_entry_block()->id);
    }
};

#endif // TAC_PARSER_SSA_HPP
