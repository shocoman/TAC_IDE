
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>

#include "semantic_analysis.h"
#include "interpreter.h"

#define ERROR_CODE 10

using namespace std;

extern string g_syntax_errors;
extern string g_warnings;
string g_current_label;

bool semantic_analysis(Node *tree) {
    if (tree->parent) {
        /*if(g_tree_root->data_type==LabelType || g_tree_root->parent->data_type==LabelType)
            line++;*/
        if (!strcmp(tree->parent->value, "call") || !strcmp(tree->parent->value, "goto") ||
            (!strcmp(tree->parent->value, "iftrue") && tree->parent->children[1] == tree)) {
            //?
        } else {
            if (find_label_node(tree->value) != nullptr)
                g_current_label = tree->value;
        }
    }
    //Сначала анализируем child Инструкции
    for (int i = 0; i < get_arity(tree); i++) {
        if (!semantic_analysis(tree->children[i]))
            return false;
    }

    //Запись элементов массива в таблицу идентификаторов с их инициализирующими значениями
    if (!strcmp(tree->value, ".block .byte") || !strcmp(tree->value, ".block .word") ||
        !strcmp(tree->value, ".block .long") || !strcmp(tree->value, ".block .double") ||
        !strcmp(tree->value, ".block .ascii")) {

        if (find_symbol(tree->children[1]->value)) {
            if (find_symbol(tree->children[1]->value)->data_type == DoubleSymbolType ||
                find_symbol(tree->children[1]->value)->data_type == CharSymbolType ||
                find_symbol(tree->children[1]->value)->data_type == StringSymbolType) {
                string error =
                    "Error! Line " + to_string(tree->line) + ". An array dimension must be an integer value.\n";
                g_syntax_errors += error;
                return false;
            }
        } else {
            if (tree->children[1]->data_type == DoubleType || tree->children[1]->data_type == CharType ||
                tree->children[1]->data_type == StringType) {
                string error =
                    "Error! Line " + to_string(tree->line) + ". An array dimension must be an integer value.\n";
                g_syntax_errors += error;
                return false;
            }
        }
    }

    //Предупреждение потери данных при начальном значении больше допустимого для данного типа
    if (is_leaf(tree) && (tree->data_type == IntType || tree->data_type == DoubleType)) {
        SymbolDataType initval = get_operand_type(tree);

        if (initval == DoubleSymbolType &&
            (!strcmp(tree->parent->value, ".word") || !strcmp(tree->parent->value, ".long"))) {
            string error =
                "Warning! Line " + to_string(tree->line) + ". Conversion from 'double', possible loss of data.\n";
            g_warnings += error;
        }

        if (initval == LongSymbolType && !strcmp(tree->parent->value, ".word")) {
            string error =
                "Warning! Line " + to_string(tree->line) + ". Conversion from 'long', possible loss of data.\n";
            g_warnings += error;
        }
    }

    //Обращение к необъявленному идентификатору или метке
    if (is_leaf(tree) && tree->data_type == IdentType && tree->value[0] != '$') {
        if (!strcmp(tree->parent->value, "call") || !strcmp(tree->parent->value, "goto") ||
            (!strcmp(tree->parent->value, "iftrue") && tree->parent->children[1] == tree) ||
            (!strcmp(tree->parent->value, "iffalse") && tree->parent->children[1] == tree)) {
            if (!find_label_node(tree->value)) {
                string error =
                    "Error! Line " + to_string(tree->line) + ". Undeclared Label <" + string(tree->value) + ">\n";
                g_syntax_errors += error;
                return false;
            }
        } else {
            if (!find_symbol(tree->value)) {
                string error =
                    "Error! Line " + to_string(tree->line) + ". Undeclared identifier <" + string(tree->value) + ">\n";
                g_syntax_errors += error;
                return false;
            }
        }
    }

    //Занесение в таблицу идентификаторов временных переменных с указанием типа
    if (is_leaf(tree) && tree->data_type == IdentType && tree->value[0] == '$') {
        if (!strcmp(tree->parent->value, ":=") && tree->parent->children[0] == tree) {
            Node *n = tree->parent;
            if (find_symbol(n->children[0]->value) == nullptr) {
                //Если инициализирующее значение представляет константу или идентификатор
                if (find_symbol(n->children[1]->value) || is_leaf(n->children[1])) {
                    add_symbol(n->children[0]->value, get_operand_type(n->children[1]));
                } else {
                    //Если справа операция
                    if (!strcmp(n->children[1]->value, "call")) {
                        char *function_name = (n->children[1])->children[0]->value;
                        if (!strcmp(function_name, "iread") || !strcmp(function_name, "fread") ||
                            !strcmp(function_name, "cread") || !strcmp(function_name, "sread") ||
                            !strcmp(function_name, "tobyte") || !strcmp(function_name, "toword") ||
                            !strcmp(function_name, "tolong") || !strcmp(function_name, "todouble") ||
                            !strcmp(function_name, "toascii")) {
                            if (!strcmp(function_name, "toword")) {
                                add_symbol(n->children[0]->value, WordSymbolType); //Да, беда...
                            }
                            if (!strcmp(function_name, "iread") || !strcmp(function_name, "tolong")) {
                                add_symbol(n->children[0]->value, LongSymbolType); //Да, беда...
                            }
                            if (!strcmp(function_name, "fread") || !strcmp(function_name, "todouble")) {
                                add_symbol(n->children[0]->value, DoubleSymbolType);
                            }
                            if (!strcmp(function_name, "cread") || !strcmp(function_name, "tobyte")) {
                                add_symbol(n->children[0]->value, CharSymbolType);
                            }
                            if (!strcmp(function_name, "sread") || !strcmp(function_name, "toascii")) {
                                add_symbol(n->children[0]->value, StringSymbolType);
                            }
                        } else {
                            //Если справа пользовательская функция тип которой определяется по "типу метки"
                            if (find_label_node(function_name) == nullptr) {
                                string error = "Error! Line " + to_string((n->children[1])->children[0]->line) +
                                               ". Undeclared Label <" + string(function_name) + ">\n";
                                g_syntax_errors += error;
                                return false;
                            } else
                                add_symbol(n->children[0]->value, find_label(function_name)->label_data_type);
                        }
                    } else {
                        //Если справа бинарная операция
                        if (get_operation_data_type(n->children[1]) != (SymbolDataType)ERROR_CODE)
                            add_symbol(n->children[0]->value, get_operation_data_type(n->children[1]));
                        else {
                            string error = "Error! Line " + to_string(n->line) + ". Invalid operation on 'string'.\n";
                            g_syntax_errors += error;
                            return false;
                        }
                    }
                }
            }
            //Если же это повторное присваивание, то тип следует изменить всё равно, ибо это потом необходимо для
            //понимания типа возвращаемого значения метки-функции
            else {
                if (change_temp_datatype(n) == 0)
                    return false;
            }
        } else {
            if (find_symbol(tree->value) == nullptr) {
                string error =
                    "Error! Line " + to_string(tree->line) + ". Undeclared identifier <" + string(tree->value) + ">\n";
                g_syntax_errors += error;
                return false;
            }
        }
    }

    if (strcmp(tree->value, ":=") == 0) {
        //Определяем тип левого значения
        SymbolDataType left = find_symbol(tree->children[0]->value)->data_type;
        //Определяем тип правого значения
        SymbolDataType right;
        //Здесь, увы, нельзя использовать чисто get_operand_type ибо мне нужны дополнительные действия внутри
        // switch-case Операция
        if (tree->children[1]->data_type == UndefType) {
            if (strcmp(tree->children[1]->value, "call") == 0) {
                char *function_name = (tree->children[1])->children[0]->value;
                if (!strcmp(function_name, "iread") || !strcmp(function_name, "fread") ||
                    !strcmp(function_name, "cread") || !strcmp(function_name, "sread") ||
                    !strcmp(function_name, "tobyte") || !strcmp(function_name, "toword") ||
                    !strcmp(function_name, "tolong") || !strcmp(function_name, "todouble") ||
                    !strcmp(function_name, "toascii")) {
                    if (!strcmp(function_name, "toword")) {
                        right = WordSymbolType; //Да, беда...
                    }
                    if (!strcmp(function_name, "iread") || !strcmp(function_name, "tolong")) {
                        right = LongSymbolType; //Да, беда...
                    }
                    if (!strcmp(function_name, "fread") || !strcmp(function_name, "todouble")) {
                        right = DoubleSymbolType;
                    }
                    if (!strcmp(function_name, "cread") || !strcmp(function_name, "tobyte")) {
                        right = CharSymbolType;
                    }
                    if (!strcmp(function_name, "sread") || !strcmp(function_name, "toascii")) {
                        right = StringSymbolType;
                    }
                } else {
                    right = find_label(function_name)->label_data_type;
                }
            } else {
                if (get_operation_data_type(tree->children[1]) == (SymbolDataType)ERROR_CODE) // 10 - код ошибки
                {
                    //Ошибка! Не допустимы бинарные операции для строк
                    //Если это не сравнение (равны, не равны)
                    string error = "Error! Line " + to_string(tree->line) + ". Invalid operation on 'string'.\n";
                    g_syntax_errors += error;
                    return false;
                } else {
                    right = get_operation_data_type(tree->children[1]);
                    //Чтобы знать тип ошибки при интерпретации.
                    if (right > 0) {
                        if (right < 3)
                            tree->children[1]->data_type = (DataType)1;
                        else
                            tree->children[1]->data_type = (DataType)(right - 1);
                    } else
                        tree->children[1]->data_type = (DataType)right;
                }
            }

        } else {
            right = get_operand_type(tree->children[1]);
        }

        //Проверка совместимости типов
        if ((right == StringSymbolType && right != left) || (left == StringSymbolType && right != left)) {
            string error = "Error! Line " + to_string(tree->line) + ". Types are incompatible.\n";
            g_syntax_errors += error;
            return false;
        }

        if (right == DoubleSymbolType && (left == CharSymbolType || left == WordSymbolType || left == LongSymbolType)) {
            string error =
                "Warning! Line " + to_string(tree->line) + ". Conversion from 'double', possible loss of data.\n";
            g_warnings += error;
        }

        if (right == LongSymbolType && (left == CharSymbolType || left == WordSymbolType)) {
            string error =
                "Warning! Line " + to_string(tree->line) + ". Conversion from 'long', possible loss of data.\n";
            g_warnings += error;
        }

        if (right == WordSymbolType && left == CharSymbolType) {
            string error =
                "Warning! Line " + to_string(tree->line) + ". Conversion from 'word', possible loss of data.\n";
            g_warnings += error;
        }
    }

    if (!strcmp(tree->value, "getparam")) {
    }

    if (!strcmp(tree->value, "return")) {
        //Определяем тип возвращаемого значения
        //Потом по этому значению проверяем совместимость при присваивании
        set_label_data_type(g_current_label.c_str(), get_operand_type(tree->children[0]));
    }

    if (!strcmp(tree->value, "putparam_operation")) {
    }
    if (!strcmp(tree->value, "call")) {
    }
    if (!strcmp(tree->value, "goto")) {
    }
    if (!strcmp(tree->value, "iftrue")) {
    }
    if (!strcmp(tree->value, "iffalse")) {
    }

    return true;
}

//Получение типа результата бинарной/унарной операции (нах.справа)
SymbolDataType get_operation_data_type(Node *n) {

    //Если операция унарная
    if (n->arity == 1) {
        //Нет унарных операций над строками
        if (get_operand_type(n->children[0]) == StringSymbolType)
            return (SymbolDataType)ERROR_CODE;
        else {
            if (!strcmp(n->value, "-"))
                // return get_operand_type(n->children[0]);
                //Отрицание всегда double, ибо мало ли
                return DoubleSymbolType;
            else
                //Отрицание - булево, но использую слово
                return WordSymbolType;
        }
    }

    //Бинарная
    //Результат деления всегда double
    if (!strcmp(n->value, "/"))
        return DoubleSymbolType;
    //Остаток от деления максимально целочисленный
    if (!strcmp(n->value, "%"))
        return LongSymbolType;
    //Логические операции ворд
    if (!strcmp(n->value, "<") || !strcmp(n->value, ">") || !strcmp(n->value, ">=") || !strcmp(n->value, "<=") ||
        !strcmp(n->value, "==") || !strcmp(n->value, "!="))
        return WordSymbolType;

    //Во всех остальных случаях назначаем тип по релевантности
    SymbolDataType one = get_operand_type(n->children[0]);
    SymbolDataType two = get_operand_type(n->children[1]);

    SymbolDataType res = std::max(one, two);

    //Если один операнд строковый, а другой нет - ошибка!
    if (one != two && res == StringSymbolType) {
        return (SymbolDataType)ERROR_CODE;
    }
    //Если оба операнда строковые, но недопустимая операция - ошибка!
    if (one == two && res == StringSymbolType) {
        if (!strcmp(n->value, "==") || !strcmp(n->value, "!="))
            return res;
        else
            return (SymbolDataType)ERROR_CODE;
    }

    return res;
}

//Получение типа данных, передаваемых "справа" (константа или переменная)
SymbolDataType get_operand_type(Node *n) {
    switch (n->data_type) {
        //Константы
        case DataType::CharType: {
            return CharSymbolType;
        }
        case DataType::IntType: {
            if (atoi(n->value) > 65535 || atoi(n->value) < 0)
                return LongSymbolType;
            else
                return WordSymbolType;
        }
        case DataType::DoubleType: {
            return DoubleSymbolType;
        }
        case DataType::StringType: {
            return StringSymbolType;
        }
            //Переменная
        case DataType::IdentType: {
            //Если это идентификатор, то просто назначаем тип идентификатора
            if (find_symbol(n->value))
                return find_symbol(n->value)->data_type;
        } break;
    }
    return (SymbolDataType)ERROR_CODE;
}

bool change_temp_datatype(Node *n) {
    if (find_symbol(n->children[1]->value) || is_leaf(n->children[1])) {
        set_symbol_data_type(n->children[0]->value, get_operand_type(n->children[1]));
    } else {
        //Если справа операция
        if (!strcmp(n->children[1]->value, "call")) {
            char *function_name = (n->children[1])->children[0]->value;

            if (!strcmp(function_name, "iread") || !strcmp(function_name, "fread") || !strcmp(function_name, "cread") ||
                !strcmp(function_name, "sread") || !strcmp(function_name, "tobyte") ||
                !strcmp(function_name, "toword") || !strcmp(function_name, "tolong") ||
                !strcmp(function_name, "todouble") || !strcmp(function_name, "toascii")) {
                if (!strcmp(function_name, "toword")) {
                    set_symbol_data_type(n->children[0]->value, WordSymbolType); //Да, беда...
                }
                if (!strcmp(function_name, "iread") || !strcmp(function_name, "tolong")) {
                    set_symbol_data_type(n->children[0]->value, LongSymbolType); //Да, беда...
                }
                if (!strcmp(function_name, "fread") || !strcmp(function_name, "todouble")) {
                    set_symbol_data_type(n->children[0]->value, DoubleSymbolType);
                }
                if (!strcmp(function_name, "cread") || !strcmp(function_name, "tobyte")) {
                    set_symbol_data_type(n->children[0]->value, CharSymbolType);
                }
                if (!strcmp(function_name, "sread") || !strcmp(function_name, "toascii")) {
                    set_symbol_data_type(n->children[0]->value, StringSymbolType);
                }
            } else {
                if (find_label_node(function_name) == nullptr) {
                    string error = "Error! Line " + to_string((n->children[1])->children[0]->line) +
                                   ". Undeclared Label <" + string(function_name) + ">\n";
                    g_syntax_errors += error;
                    return false;
                } else
                    set_symbol_data_type(n->children[0]->value, find_label(function_name)->label_data_type);
            }
        } else {
            if (get_operation_data_type(n->children[1]) != (SymbolDataType)ERROR_CODE)
                set_symbol_data_type(n->children[0]->value,
                                     (SymbolDataType)get_operation_data_type(n->children[1]));
            else {
                string error = "Error! Line " + to_string(n->line) + ". Invalid operation on 'string'.\n";
                g_syntax_errors += error;
                return false;
            }
        }
    }
    return true;
}