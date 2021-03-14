
#include "label_table.h"
#include <string.h>

static Label *g_label_list = nullptr;

Node *find_label_node(const char *s) {
    for (Label *sp = g_label_list; sp != nullptr; sp = sp->next)
        if (!strcmp(sp->name, s))
            return sp->place;
    return nullptr;
}

void add_label(const char *s, Node *node) {
    auto *sp = (Label *)emalloc(sizeof(Label));
    sp->name = emalloc(strlen(s) + 1);
    strcpy(sp->name, s);
    sp->place = node;
    sp->next = g_label_list;
    g_label_list = sp;
}

Label *get_next_label(const char *s) {
    for (Label *sp = g_label_list; sp != nullptr; sp = sp->next)
        if (!strcmp(sp->name, s))
            return (sp->next);
    return nullptr;
}

Label *find_label(const char *s) {
    for (Label *sp = g_label_list; sp != nullptr; sp = sp->next)
        if (!strcmp(sp->name, s))
            return sp;
    return nullptr;
}

Label *get_previous_label(const char *s) {
    for (Label *sp = g_label_list; sp != nullptr; sp = sp->next)
        if (get_next_label(sp->name))
            if (!strcmp(get_next_label(sp->name)->name, s))
                return sp;
    return nullptr;
}

void set_label_data_type(const char *s, SymbolDataType dt) {
    for (Label *sp = g_label_list; sp != nullptr; sp = sp->next)
        if (!strcmp(sp->name, s)) {
            sp->label_data_type = dt;
            auto previous_label = get_previous_label(sp->name);
            if (previous_label) {
                auto previous_label_last_child =
                    previous_label->place->children[previous_label->place->arity - 1];
                if (strcmp(previous_label_last_child->value, "return") != 0) {
                    previous_label->label_data_type = dt;
                }
            }
        }
}

void remove_labels() {
    for (Label *sp = g_label_list; sp != nullptr; sp = sp->next) {
        free(sp->name);
        free(sp->place);
    }
    g_label_list = nullptr;
}

void print_labels() {
    std::cout << "Labels: " << std::endl;
    for (Label *sp = g_label_list; sp != nullptr; sp = sp->next)
        std::cout << sp->name << std::endl;
}