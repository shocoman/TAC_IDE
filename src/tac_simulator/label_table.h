#ifndef __LABTAB_H__
#define __LABTAB_H__

#include "symbol_table.h"
#include "tac_tree.h"

struct Label {
    char *name;                     // Имя метки
    Node *place;                    // Адрес
    SymbolDataType label_data_type; // Тип возвращаемого значения функции(метки)
    Label *next;                    // Связь со следующей записью
};

void add_label(const char *s, Node *node); //Добавление новой метки
Node *find_label_node(const char *s);      //Поиск узла в дереве где начинается метка
Label *find_label(const char *s);          //Получение ссылки на метку по имени
Label *get_next_label(const char *s);      //Получение следующей в списке метки
Label *get_previous_label(const char *s);  //Получение предыдущей в списке метки

//Задать тип возвращаемого значения метки для оператора return. У "main" - это будет NULL
void set_label_data_type(const char *s, SymbolDataType dt);
void remove_labels();
void print_labels();
#endif
