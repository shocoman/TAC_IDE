#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "label_table.h"
#include "symbol_table.h"
#include "tac_tree.h"

enum class InterpreterState {
    None,
    Wait,
    WaitToPrint,
    WaitToRead,
    Done,
};

struct Interpreter {
    // очередь для параметров функций
    std::queue<SymbolValue> m_parameters_queue;

    std::condition_variable m_pausing_var;
    std::mutex m_pausing_mutex;

    InterpreterState m_state;
    std::string m_output;
    bool m_remote_mode = false;

    bool begin_interpretation(Node *tree, std::string current_state, bool top_level = false);

    bool assignment_operation(Node *n, std::string current_state);
    bool if_operation(bool if_true, Node *ch, Node *n, std::string currentState);
    void putparam_operation(Node *n, std::string current_state);
    void write_operation(Node *n, std::string current_state);
    void getparam_operation(Node *n, std::string current_state);
    std::string arithmetic_operation(Node *n, std::string current_state);
    std::string read_operation(Node *n, std::string current_state);
    std::string logical_or_comparison_operation(Node *n, std::string current_state);

    std::string user_function(Node *n, std::string current_state);
    string format_array_name(Node *arr);

    std::string type_cast_operation(Node *n, std::string current_state);

    void reset(Node *g_tree_root);
    void next_step();
    void pause(InterpreterState state);
};

#endif