//
// Created by shoco on 1/3/2021.
//

#ifndef TAC_SIMULATOR_TEST_TAC_PROJECT_H
#define TAC_SIMULATOR_TEST_TAC_PROJECT_H

#include <condition_variable>
#include <fstream>
#include <mutex>
#include <thread>

#include "interpreter.h"
#include "semantic_analysis.h"

#include "tac_tree.h"
#include "interpreter.h"
#include "semantic_analysis.h"
#include "label_table.h"

extern Node *g_tree_root;

bool interpret_string(std::string code);

#endif //TAC_SIMULATOR_TEST_TAC_PROJECT_H
