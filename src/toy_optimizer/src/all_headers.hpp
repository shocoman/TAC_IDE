//
// Created by shoco on 3/18/2021.
//

#ifndef TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_ALL_HEADERS_HPP
#define TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_ALL_HEADERS_HPP

#include "optimizations/copy_propagation.hpp"
#include "optimizations/ssa.hpp"
#include "optimizations/value_numbering.hpp"
#include "optimizations/constant_folding.hpp"
#include "optimizations/lazy_code_motion.hpp"
#include "optimizations/operator_strength_reduction.hpp"
#include "optimizations/sparse_conditional_constant_propagation.hpp"
#include "optimizations/useless_code_elimination.hpp"
#include "optimizations/sparse_simple_constant_propagation.hpp"

#include "data_flow_analyses/live_variable_analysis.hpp"
#include "data_flow_analyses/reaching_definitions.hpp"
#include "data_flow_analyses/use_def_graph.hpp"
#include "data_flow_analyses/data_flow_analyses.hpp"
#include "data_flow_analyses/data_flow_framework.hpp"
#include "data_flow_analyses/print_graph.hpp"
#include "data_flow_analyses/dominators.hpp"
#include "data_flow_analyses/set_utilities.hpp"
#include "data_flow_analyses/ssa_graph.hpp"

#endif // TOY_OPTIMIZER_SRC_TOY_OPTIMIZER_SRC_ALL_HEADERS_HPP
