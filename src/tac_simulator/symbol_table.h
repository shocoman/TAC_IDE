#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include "tac_tree.h"

enum SymbolDataType {
    CharSymbolType,     // CharType
    WordSymbolType,     // IntType
    LongSymbolType,     // IntType
    DoubleSymbolType,   // DoubleType
    StringSymbolType    // StringType
};

union SymbolValue {
    uint8_t char_value;
    uint16_t word_value;
    int32_t long_value;
    double double_value;
    char *string_value;
};

struct Symbol {
    char *name;    //Имя переменной
    char *count;   //Если это массив (а не его элемент), здесь хранится кол-во элементов (в виде строки)
    SymbolDataType data_type;   //Тип
    SymbolValue value;          //Текущее значение
    Symbol *next;               //Связь со следующей записью
};

Symbol *find_symbol(const char *s);

void add_symbol(const char *s, SymbolDataType dt, const char *val, const char *count);

void add_symbol(const char *s, SymbolDataType dt);

void edit_symbol(const char *s, const char *val);

SymbolDataType get_symbol_type(const char *s);

std::string get_symbol_value(const char *s);

void set_symbol_data_type(const char *s, SymbolDataType dt);

void print_symbol_table();
void remove_symbols();

#endif
