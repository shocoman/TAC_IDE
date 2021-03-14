
#include "symbol_table.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace std;

static Symbol *g_symbol_list = nullptr;

Symbol *find_symbol(const char *s) {
    for (Symbol *sp = g_symbol_list; sp != nullptr; sp = sp->next)
        if (!strcmp(sp->name, s))
            return sp;
    return nullptr;
}

void add_symbol(const char *s, SymbolDataType dt, const char *val, const char *count) {
    auto *sp = (Symbol *)malloc(sizeof(Symbol));
    sp->name = emalloc(strlen(s) + 1);
    strcpy(sp->name, s);
    sp->data_type = dt;

    if (val) {
        if (dt == WordSymbolType)
            sp->value.word_value = atoi(val);
        if (dt == LongSymbolType)
            sp->value.long_value = atoi(val);
        if (dt == DoubleSymbolType) {
            string str = string(val);
            std::replace(str.begin(), str.end(), ',', '.');
            sp->value.double_value = atof(str.c_str());
        }
        if (dt == CharSymbolType)
            sp->value.char_value = val[0];
        if (dt == StringSymbolType) {
            sp->value.string_value = emalloc(strlen(val) + 1);
            strcpy(sp->value.string_value, val);
        }
    } else {
        if (dt == WordSymbolType)
            sp->value.word_value = 0;
        if (dt == LongSymbolType)
            sp->value.long_value = 0;
        if (dt == DoubleSymbolType)
            sp->value.double_value = 0.0;
        if (dt == CharSymbolType)
            sp->value.char_value = '0';
        if (dt == StringSymbolType) {
            sp->value.string_value = emalloc(1);
            strcpy(sp->value.string_value, "");
        }
    }
    sp->next = g_symbol_list;
    g_symbol_list = sp;

    if (count) {
        sp->count = emalloc(strlen(count) + 1);
        strcpy(sp->count, count);
    } else {
        sp->count = nullptr;
    }
}

void add_symbol(const char *s, SymbolDataType dt) {
    auto *sp = (Symbol *)malloc(sizeof(Symbol));
    sp->name = emalloc(strlen(s) + 1);
    strcpy(sp->name, s);
    sp->data_type = dt;
    if (dt == WordSymbolType)
        sp->value.word_value = 0;
    if (dt == LongSymbolType)
        sp->value.long_value = 0;
    if (dt == DoubleSymbolType)
        sp->value.double_value = 0;
    if (dt == CharSymbolType)
        sp->value.char_value = '0';
    if (dt == StringSymbolType) {
        sp->value.string_value = emalloc(1);
        strcpy(sp->value.string_value, "");
    }

    sp->next = g_symbol_list;
    sp->count = nullptr;
    g_symbol_list = sp;
}

void edit_symbol(const char *s, const char *val) {
    Symbol *sp = find_symbol(s);
    if (sp->data_type == WordSymbolType)
        sp->value.word_value = (uint16_t)atoi(val);
    if (sp->data_type == LongSymbolType)
        sp->value.long_value = (int32_t)atoi(val);
    if (sp->data_type == DoubleSymbolType) {
        string str = string(val);
        std::replace(str.begin(), str.end(), ',', '.');
        sp->value.double_value = atof(str.c_str());
    }
    if (sp->data_type == CharSymbolType)
        sp->value.char_value = val[0];
    if (sp->data_type == StringSymbolType) {
        sp->value.string_value = emalloc(strlen(val) + 1);
        strcpy(sp->value.string_value, val);
    }
}

SymbolDataType get_symbol_type(const char *s) {
    return find_symbol(s)->data_type;
}

std::string get_symbol_value(const char *s) {
    Symbol *sp = find_symbol(s);
    if (sp->data_type == WordSymbolType) {
        return to_string(sp->value.word_value);
    }
    if (sp->data_type == LongSymbolType) {
        return to_string(sp->value.long_value);
    }
    if (sp->data_type == DoubleSymbolType) {
        return to_string(sp->value.double_value);
    }
    if (sp->data_type == CharSymbolType) {
        return to_string((char)sp->value.char_value);
    }
    return string(sp->value.string_value);
}

void set_symbol_data_type(const char *s, SymbolDataType dt) {
    Symbol *sp = find_symbol(s);
    sp->data_type = dt;
}

void print_symbol_table() {
    printf("---------------------------------------------------------\n");
    printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n", "NAME", "COUNT", "DATATYPE", "wVALUE", "lVALUE",
           "dVALUE", "bVALUE", "aVALUE");
    printf("---------------------------------------------------------\n");
    Symbol *sp;
    for (sp = g_symbol_list; sp != (Symbol *)0; sp = sp->next) {
        string count = "-";
        string wval = "-";
        string lval = "-";
        string dval = "-";
        string cval = "-";
        string sval = "-";
        if (sp->count != nullptr)
            count = string(sp->count);
        string dt = to_string((int)sp->data_type);
        if (sp->data_type == WordSymbolType)
            wval = to_string((uint16_t)sp->value.word_value);
        if (sp->data_type == LongSymbolType)
            lval = to_string((int32_t)sp->value.long_value);
        if (sp->data_type == DoubleSymbolType)
            dval = to_string((double)sp->value.double_value);
        if (sp->data_type == CharSymbolType)
            cval = to_string((char)sp->value.char_value);
        if (sp->data_type == StringSymbolType)
            sval = string(sp->value.string_value);
        string name = sp->name;
        printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n", name.c_str(), count.c_str(), dt.c_str(),
               wval.c_str(), lval.c_str(), dval.c_str(), cval.c_str(), sval.c_str());
    }
}


void remove_symbols() {
    for (Symbol *sp = g_symbol_list; sp != nullptr; sp = sp->next) {
        free(sp->name);
        if (sp->count)
            free(sp->count);
    }
    g_symbol_list = nullptr;
}
