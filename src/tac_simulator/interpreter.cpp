#include <iostream>
#include <string>

#include "interpreter.h"
#include "semantic_analysis.h"

extern Label g_label_list;
extern Symbol g_symbol_list;
extern std::string g_syntax_errors;

void Interpreter::next_step() {
    m_pausing_var.notify_one();
    m_state = InterpreterState::None;

    //    switch (m_state) {
    //        case InterpreterState::Wait:
    //            m_pausing_var.notify_one();
    //            break;
    //        case InterpreterState::None:
    //            break;
    //        case InterpreterState::WaitToPrint:
    //            // print something
    //            m_pausing_var.notify_one();
    //            break;
    //        case InterpreterState::WaitToRead:
    //            // wright something somewhere
    //            m_pausing_var.notify_one();
    //            break;
    //    }
}

void Interpreter::pause(InterpreterState state) {
    m_state = state;
    std::unique_lock<std::mutex> lock(m_pausing_mutex);
    m_pausing_var.wait(lock);
    m_state = InterpreterState::None;
}

bool Interpreter::begin_interpretation(Node *tree, std::string current_state, bool top_level) {
    // проверяем, если текущая операция - метка
    if (find_label_node(tree->value) != nullptr) {
        current_state = std::string(tree->value);
    }

    if (!is_leaf(tree)) {
        for (int i = 0; i < get_arity(tree); i++) {
            if (!is_leaf(tree->children[i])) {
                if (!begin_interpretation(tree->children[i], current_state))
                    return false;
            }
        }
    }

    if (!strcmp(tree->value, current_state.c_str())) {
        if (get_next_label(tree->value) != nullptr) {
            if (!begin_interpretation(get_next_label(tree->value)->place, current_state)) {
                //                if (top_level) std::cout << "DOMEDSADASDS" << std::endl;

                return false;
            }
        }
    }

    //Запись элементов массива в таблицу идентификаторов с их инициализирующими значениями
    if (!strcmp(tree->value, ".block .byte") || !strcmp(tree->value, ".block .word") ||
        !strcmp(tree->value, ".block .long") || !strcmp(tree->value, ".block .double") ||
        !strcmp(tree->value, ".block .ascii")) {

        int count; // размер массива
        if (find_symbol(tree->children[1]->value)) {
            string value = get_symbol_value(tree->children[1]->value);
            count = atoi(value.c_str());
        } else {
            count = atoi(tree->children[1]->value);
        }

        string value; // начальное значение элементов массива
        if (find_symbol(tree->children[2]->value)) {
            value = get_symbol_value(tree->children[2]->value);
        } else {
            value = string(tree->children[2]->value);
        }

        // Вставляем поэлементно массив в виде обычных переменных
        for (int i = 0; i < count; i++) {
            string tmp = string(tree->children[0]->value) + "[" + to_string(i) + "]";
            SymbolDataType symbol = find_symbol(tree->children[0]->value)->data_type;
            add_symbol(tmp.c_str(), symbol, value.c_str(), nullptr);
        }
    }

    if (!strcmp(tree->value, ":=")) {
        if (!assignment_operation(tree, current_state)) {
            return false;
        }
    } else if (!strcmp(tree->value, "getparam")) {
        if (m_parameters_queue.empty()) {
            string error = "Getparam error! Line " + to_string(tree->line) + ". No input parameters.\n";
            g_syntax_errors += error;
            return false;
        }
        getparam_operation(tree, current_state);
    } else if (!strcmp(tree->value, "return")) {
        putparam_operation(tree, current_state);
        return false;
    } else if (!strcmp(tree->value, "putparam")) {
        putparam_operation(tree, current_state);
    }

    if (!strcmp(tree->value, "call")) {
        char *function_name = tree->children[0]->value;
        if (!strcmp(function_name, "iwrite") || !strcmp(function_name, "fwrite") || !strcmp(function_name, "cwrite") ||
            !strcmp(function_name, "swrite")) {
            if (m_parameters_queue.empty()) {
                g_syntax_errors += "Call error! Line " + to_string(tree->line) + ". No input parameters.\n";
                return false;
            }
            write_operation(tree, current_state);
        } else if (strcmp(tree->parent->value, ":=") != 0) {
            // вызов пользовательской функции (в этом случае возвращаемое значение не сохраняется)
            auto params_count = (int)atof(tree->children[1]->value);
            if (m_parameters_queue.size() < params_count) {
                g_syntax_errors += "Error! Line " + to_string(tree->line) + ". Not enough input parameters.\n";
                return false;
            }
            user_function(tree, current_state);
        }
    }

    if (!strcmp(tree->value, "goto")) {
        begin_interpretation(find_label_node(tree->children[0]->value), current_state);
        return false;
    } else if (!strcmp(tree->value, "iftrue")) {
        if (if_operation(true, tree->children[0], tree->children[1], current_state)) {
            return false;
        }
    } else if (!strcmp(tree->value, "iffalse")) {
        if (if_operation(false, tree->children[0], tree->children[1], current_state)) {
            return false;
        }
    }

    //    if (top_level) std::cout << "DOMEDSADASDS" << std::endl;

    return true;
}

bool Interpreter::if_operation(bool if_true, Node *ch, Node *n, std::string currentState) {
    if (is_leaf(ch)) {
        if (if_true) {
            if (find_symbol(ch->value)->value.word_value != 0) {
                begin_interpretation(find_label_node(n->value), currentState);
                return true;
            }
        } else {
            if (find_symbol(ch->value)->value.word_value == 0) {
                begin_interpretation(find_label_node(n->value), currentState);
                return true;
            }
        }
    }

    return false;
}

bool Interpreter::assignment_operation(Node *n, std::string current_state) {
    std::string name;
    std::string lvalue_name;

    //Определяем, что у нас слева
    if (is_leaf(n->children[0])) {
        lvalue_name = n->children[0]->value;
    } else {
        lvalue_name = format_array_name(n->children[0]);
    }

    //Тип временных переменных должен меняться "на лету"
    if (lvalue_name[0] == '$')
        change_temp_datatype(n);

    if (is_leaf(n->children[1])) {
        if (n->children[1]->data_type != IdentType) {
            // Если присваивание константы
            edit_symbol(lvalue_name.c_str(), n->children[1]->value);
        } else {
            // или переменной
            name = get_symbol_value(n->children[1]->value);
            edit_symbol(lvalue_name.c_str(), name.c_str());
        }
        return true;
    }
    //Элемента массива
    if (!is_leaf(n->children[1]) && n->children[1]->data_type == IdentType) {
        name = format_array_name(n->children[1]);
        if (!find_symbol(name.c_str())) {
            string error = "Error! Line " + to_string(n->line) + ". Undeclared identifier <" + name + ">.\n";
            g_syntax_errors += error;
            return false;
        }

        name = get_symbol_value(name.c_str());
        edit_symbol(lvalue_name.c_str(), name.c_str());
        return true;
    }

    //Присваивание результата арифметической операции
    if (!strcmp((n->children[1])->value, "+") || !strcmp((n->children[1])->value, "-") ||
        !strcmp((n->children[1])->value, "*") || !strcmp((n->children[1])->value, "/") ||
        !strcmp((n->children[1])->value, "%")) {
        name = arithmetic_operation(n->children[1], current_state);
        edit_symbol(lvalue_name.c_str(), name.c_str());
        return true;
    }

    // Присваивание результата работы функции
    if (!strcmp((n->children[1])->value, "call")) {
        //Считывание ввода
        char *function_name = n->children[1]->children[0]->value;
        if (!strcmp(function_name, "iread") || !strcmp(function_name, "fread") || !strcmp(function_name, "cread") ||
            !strcmp(function_name, "sread")) {
            name = read_operation(n->children[1], current_state);
            edit_symbol(lvalue_name.c_str(), name.c_str());
        } else if (!strcmp(function_name, "tobyte") || !strcmp(function_name, "toword") ||
                   !strcmp(function_name, "tolong") || !strcmp(function_name, "todouble") ||
                   !strcmp(function_name, "toascii")) {
            //Преобразования
            name = type_cast_operation(n->children[1], current_state);
            if (strcmp(function_name, "toascii") != 0 && name.empty())
                return false;
            edit_symbol(lvalue_name.c_str(), name.c_str());
        } else {
            // Пользовательская функция
            name = user_function(n->children[1], current_state);
            edit_symbol(lvalue_name.c_str(), name.c_str());
        }
        return true;
    }

    //Присваивание результата логической операции или операции сравнения (любой другой случай)
    name = logical_or_comparison_operation(n->children[1], current_state);
    edit_symbol(lvalue_name.c_str(), name.c_str());
    return true;
}

std::string Interpreter::read_operation(Node *n, std::string current_state) {

    std::string res;
    if (m_remote_mode) {
        pause(InterpreterState::WaitToRead);
        res = m_output;
    } else {
        getline(std::cin, res);
    }
    return res;
}

std::string Interpreter::logical_or_comparison_operation(Node *n, std::string current_state) {
    string s1 = "0", s2 = "0";
    double a = 0, b = 0;

    //Унарный NOT
    if (!n->children[1]) {
        if (n->children[0]->data_type == IdentType) {
            s2 = get_symbol_value(n->children[0]->value);
        } else {
            s2 = n->children[0]->value;
        }
    } else {
        //Бинарная операция
        if ((n->children[0]->data_type == IntType && n->children[1]->data_type == IntType) ||
            (n->children[0]->data_type == DoubleType && n->children[1]->data_type == DoubleType)) {
            s1 = n->children[0]->value;
            s2 = n->children[1]->value;
        }
        if ((n->children[0]->data_type == IdentType && n->children[1]->data_type == IntType) ||
            (n->children[0]->data_type == IdentType && n->children[1]->data_type == DoubleType)) {
            s1 = get_symbol_value(n->children[0]->value);
            s2 = n->children[1]->value;
        }
        if ((n->children[0]->data_type == IntType && n->children[1]->data_type == IdentType) ||
            (n->children[0]->data_type == DoubleType && n->children[1]->data_type == IdentType)) {
            s1 = n->children[0]->value;
            s2 = get_symbol_value(n->children[1]->value);
        }
        if (n->children[0]->data_type == IdentType && n->children[1]->data_type == IdentType) {
            s1 = get_symbol_value(n->children[0]->value);
            s2 = get_symbol_value(n->children[1]->value);
        }
    }

    if (n->data_type == StringType) {
        if (!strcmp(n->value, "==")) {
            return (s1 == s2) ? "1" : "0";
        }
        if (!strcmp(n->value, "!=")) {
            return (s1 != s2) ? "1" : "0";
        }
    } else {
        a = atof(s1.c_str());
        b = atof(s2.c_str());
    }

    if (!strcmp(n->value, ">=")) {
        return (a >= b) ? "1" : "0";
    }
    if (!strcmp(n->value, "<=")) {
        return (a <= b) ? "1" : "0";
    }
    if (!strcmp(n->value, "==")) {
        return (a == b) ? "1" : "0";
    }
    if (!strcmp(n->value, "!=")) {
        return (a != b) ? "1" : "0";
    }

    if (!strcmp(n->value, "&&")) {
        return (a && b) ? "1" : "0";
    }
    if (!strcmp(n->value, "||")) {
        return (a || b) ? "1" : "0";
    }

    switch (n->value[0]) {
    case '>': {
        return (a > b) ? "1" : "0";
    }
    case '<': {
        return (a < b) ? "1" : "0";
    }
    case '^': {
        return (a != b) ? "1" : "0";
    }
    case '!': {
        return (b >= 1) ? "1" : "0";
    }
    }

    return "";
}

std::string Interpreter::arithmetic_operation(Node *n, std::string current_state) {
    std::string res;
    double a = 0, b = 0;

    //Унарный минус
    if (n->arity == 1) {
        if (n->children[0]->data_type == IdentType) {
            res = get_symbol_value(n->children[0]->value);
            b = atof(res.c_str());
        } else {
            b = atof(n->children[0]->value);
        }
    } else {
        //Бинарная операция
        if ((n->children[0]->data_type == IntType && n->children[1]->data_type == IntType) ||
            (n->children[0]->data_type == DoubleType && n->children[1]->data_type == DoubleType)) {
            a = atof(n->children[0]->value);
            b = atof(n->children[1]->value);
        }
        if ((n->children[0]->data_type == IdentType && n->children[1]->data_type == IntType) ||
            (n->children[0]->data_type == IdentType && n->children[1]->data_type == DoubleType)) {
            res = get_symbol_value(n->children[0]->value);
            a = atof(res.c_str());
            b = atof(n->children[1]->value);
        }
        if ((n->children[0]->data_type == IntType && n->children[1]->data_type == IdentType) ||
            (n->children[0]->data_type == DoubleType && n->children[1]->data_type == IdentType)) {
            a = atof(n->children[0]->value);
            res = get_symbol_value(n->children[1]->value);
            b = atof(res.c_str());
        }
        if (n->children[0]->data_type == IdentType && n->children[1]->data_type == IdentType) {
            res = get_symbol_value(n->children[0]->value);
            a = atof(res.c_str());
            res = get_symbol_value(n->children[1]->value);
            b = atof(res.c_str());
        }
    }

    switch (n->value[0]) {
    case '+':
        res = to_string(a + b);
        break;
    case '-':
        res = to_string(a - b);
        break;
    case '*':
        res = to_string(a * b);
        break;
    case '/': {
        if (n->children[0]->data_type == IntType ||
            n->children[0]->data_type == IdentType && get_symbol_type(n->children[0]->value) == WordSymbolType)
            res = to_string((double)(int(a / b)));
        if (n->children[0]->data_type == DoubleType ||
            n->children[0]->data_type == IdentType && get_symbol_type(n->children[0]->value) == DoubleSymbolType)
            res = to_string((double)(a / b));
    } break;
    case '%':
        res = to_string((int)a % (int)b);
        break;
    }

    return res;
}

void Interpreter::putparam_operation(Node *n, std::string current_state) {
    SymbolValue temp{};

    if (is_leaf(n->children[0]) && n->children[0]->data_type != IdentType) {
        switch (n->children[0]->data_type) {
        case IntType:
            temp.long_value = (long)atoi(n->children[0]->value);
            m_parameters_queue.push(temp);
            break;
        case DoubleType:
            temp.double_value = atof(n->children[0]->value);
            m_parameters_queue.push(temp);
            break;
        case CharType:
            temp.char_value = n->children[0]->value[0];
            m_parameters_queue.push(temp);
            break;
        case StringType:
            temp.string_value = n->children[0]->value;
            m_parameters_queue.push(temp);
            break;
        }
    } else {
        //Обычный идентификатор y $t1
        if (is_leaf(n->children[0])) {
            // Все целые значения сливаются в лонг значение переменной, потому
            // что вывод организован для лонга. Дабы не путаться.
            m_parameters_queue.push(find_symbol(n->children[0]->value)->value);
            if (find_symbol(n->children[0]->value)->data_type == WordSymbolType) {
                m_parameters_queue.front().long_value = m_parameters_queue.front().word_value;
            }
        }
        //Элемент массива arr[...]
        else {
            string name = format_array_name(n->children[0]);
            m_parameters_queue.push(find_symbol(name.c_str())->value);
            // Все целые значения сливаются в лонг значение переменной, потому
            // что вывод организован для лонга. Дабы не путаться.
            if (find_symbol(name.c_str())->data_type == WordSymbolType) {
                m_parameters_queue.front().long_value = m_parameters_queue.front().word_value;
            }
        }
    }
}

void Interpreter::getparam_operation(Node *n, std::string current_state) {
    //Обычный идентификатор y $t1
    if (is_leaf(n->children[0])) {
        find_symbol(n->children[0]->value)->value = m_parameters_queue.front();
    }
    //Элемент массива arr[...]
    else {
        string s = format_array_name(n->children[0]);
        char *name = emalloc((s.length() + 1) * sizeof(char));
        strcpy(name, s.c_str());
        find_symbol(name)->value = m_parameters_queue.front();
        free(name);
    }
    m_parameters_queue.pop();
}

void Interpreter::write_operation(Node *n, std::string current_state) {
    m_output.clear();
    int count = atoi(n->children[1]->value);
    if (!strcmp(n->children[0]->value, "iwrite")) {
        for (int i = 0; i < count; i++) {
            //            printf("%d\n", m_parameters_queue.front().long_value);
            m_output = to_string(m_parameters_queue.front().long_value);
            m_parameters_queue.pop();
        }
    }

    if (!strcmp(n->children[0]->value, "fwrite")) {
        for (int i = 0; i < count; i++) {
            //            printf("%g\n", m_parameters_queue.front().double_value);
            // %e Экспоненциальное представление числа (в виде мантиссы и порядка)
            // %f Десятичное число с плавающей точкой
            // %g Использует более короткий из форматов %e или %f
            m_output = to_string(m_parameters_queue.front().double_value);
            m_parameters_queue.pop();
        }
    }

    if (!strcmp(n->children[0]->value, "cwrite")) {
        for (int i = 0; i < count; i++) {
            //            printf("%c\n", m_parameters_queue.front().char_value);
            m_output = to_string(m_parameters_queue.front().char_value);
            m_parameters_queue.pop();
        }
    }

    if (!strcmp(n->children[0]->value, "swrite")) {
        for (int i = 0; i < count; i++) {
            //            printf("%s\n", m_parameters_queue.front().string_value);
            char *str = m_parameters_queue.front().string_value;
            if (str != nullptr)
                m_output = string(str);
            m_parameters_queue.pop();
        }
    }

    if (!m_remote_mode) {
        std::cout << m_output << std::endl;
    }
    if (!m_output.empty() && m_remote_mode) {
        pause(InterpreterState::WaitToPrint);
    }
}

std::string Interpreter::user_function(Node *n, std::string current_state) {

    begin_interpretation(find_label_node(n->children[0]->value), current_state);
    string result;
    if (m_parameters_queue.front().word_value) {
        result = to_string(m_parameters_queue.front().word_value);
        m_parameters_queue.pop();
    } else if (m_parameters_queue.front().long_value) {
        result = to_string(m_parameters_queue.front().long_value);
        m_parameters_queue.pop();
    } else if (m_parameters_queue.front().double_value > 0) {
        result = to_string(m_parameters_queue.front().double_value);
        m_parameters_queue.pop();
    } else if (m_parameters_queue.front().char_value) {
        result = std::string(1, (char)m_parameters_queue.front().char_value);
        m_parameters_queue.pop();
    } else if (m_parameters_queue.front().string_value) {
        result = m_parameters_queue.front().string_value;
        m_parameters_queue.pop();
    }
    //Пусть
    return result;
}

string Interpreter::format_array_name(Node *arr) {
    string name;
    if (arr->children[0]->data_type != IdentType) {
        //[0]
        name = string(arr->value) + "[" + string(arr->children[0]->value) + "]";
    } else {
        //[y]
        //[$t0]
        name = string(arr->value) + "[" + get_symbol_value(arr->children[0]->value) + "]";
    }
    return name;
}

std::string Interpreter::type_cast_operation(Node *n, std::string current_state) {
    string result;
    string operand;

    if (m_parameters_queue.front().word_value)
        operand = to_string(m_parameters_queue.front().word_value);
    if (m_parameters_queue.front().long_value)
        operand = to_string(m_parameters_queue.front().long_value);
    if (m_parameters_queue.front().double_value > 1e-6)
        operand = to_string(m_parameters_queue.front().double_value);
    if (m_parameters_queue.front().char_value)
        operand = to_string(m_parameters_queue.front().char_value);
    if (m_parameters_queue.front().string_value)
        operand = string(m_parameters_queue.front().string_value);
    m_parameters_queue.pop();

    if (!strcmp(n->children[0]->value, "tobyte")) {
        //Три миллиона вариантов. Оставлю пока этот.
        result = to_string((uint8_t)operand[0]);
    } else if (!strcmp(n->children[0]->value, "toword")) {
        short i = atoi(operand.c_str());
        if (i == 0 && operand != "0") {
            string error = "Error! Line " + to_string(n->line) + ". Error format conversion. (toword)\n";
            g_syntax_errors += error;
            return "";
        } else {
            result = to_string((uint16_t)i);
        }
    } else if (!strcmp(n->children[0]->value, "tolong")) {
        int i = atol(operand.c_str());
        if (i == 0 && operand != "0") {
            string error = "Error! Line " + to_string(n->line) + ". Format conversion error. (tolong)\n";
            g_syntax_errors += error;
            return "";
        } else {
            result = to_string((int32_t)i);
        }
    } else if (!strcmp(n->children[0]->value, "todouble")) {
        double i = atof(operand.c_str());
        if (i == 0.0 && (operand != "0" || operand != "0.0")) {
            string error = "Error! Line " + to_string(n->line) + ". Error format conversion. (todouble)\n";
            g_syntax_errors += error;
            return "";
        } else {
            result = to_string((double)i);
        }
    } else if (!strcmp(n->children[0]->value, "toascii")) {
        return operand;
    }
    
    return result;
}

void Interpreter::reset(Node *g_tree_root) {
    remove_nodes(g_tree_root);
    remove_symbols();
    remove_labels();
    g_tree_root = nullptr;
}
