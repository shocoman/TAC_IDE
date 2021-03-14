
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "tac_tree.h"
using namespace std;

char *emalloc(unsigned n) {
    char *p = (char *)malloc(n);
    if (!p) {
        printf("ERROR: out of memory\n");
        exit(1);
    }
    return p;
}

bool is_leaf(Node *node) { return node->is_leaf; }
int get_arity(Node *node) { return node->arity; }

Node *create_node(const char *val, DataType dt) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (!node)
        return nullptr;
    node->arity = 0;
    node->data_type = dt;
    node->is_leaf = true;

    node->value = emalloc(strlen(val) + 1);

    std::string s(val);
    // конвертируем строку в нижний регистр, полагаем что она ascii
    if (dt != DataType::StringType && dt != DataType::CharType)
        std::transform(s.begin(), s.end(), s.begin(), [](uint8_t c) { return std::tolower(c); });
    strcpy(node->value, s.c_str());

    node->parent = nullptr;
    node->children = nullptr;
    return node;
}

Node *create_node(const char *val, DataType dt, Node *fst_child, Node *snd_child) {
    auto node = (Node *)malloc(sizeof(Node));
    if (!node)
        return nullptr;

    node->is_leaf = false;

    node->data_type = dt;
    node->value = emalloc(strlen(val) + 1);
    strcpy(node->value, val);
    node->parent = nullptr;
    node->arity = snd_child ? 2 : 1;

    if (snd_child)
        node->children = (Node **)malloc(2 * sizeof(Node *));
    else
        node->children = (Node **)malloc(1 * sizeof(Node *));

    if (fst_child) {
        node->children[0] = fst_child;
        node->children[0]->parent = node;
    } else {
        node->children[0] = nullptr;
    }
    if (snd_child) {
        node->children[1] = snd_child;
        node->children[1]->parent = node;
    }

    return node;
}
Node *create_node(const char *val, DataType dt, list<Node *> children) {
    auto node = (Node *)malloc(sizeof(Node));
    if (!node)
        return nullptr;

    node->is_leaf = false;
    node->data_type = dt;
    node->value = emalloc(strlen(val) + 1);
    strcpy(node->value, val);

    node->parent = nullptr;
    node->arity = children.size();
    node->children = (Node **)malloc(children.size() * sizeof(Node *));

    auto it = children.begin();
    for (int i = (int)children.size() - 1; i >= 0; i--, it++) {
        node->children[i] = *it;
        node->children[i]->parent = node;
    }

    return node;
}

void print_tree(FILE *stream, Node *tree, int spaces) {
    fprintf(stream, "%*s", spaces, " ");
    fprintf(stream, "Line %d: ", tree->line);
    fprintf(stream, "%s", tree->value);
    fprintf(stream, "\n");

    if (!is_leaf(tree))
        for (int i = 0; i < get_arity(tree); i++) {
            print_tree(stream, tree->children[i], spaces + 2);
        }
}

void set_line_numbers_for_all_instructions(Node *tree) {
    int line = 0;
    tree->line = line;

    for (int i = 0; i < get_arity(tree); i++) {
        line++;
        tree->children[i]->line = line;
        for (int j = 0; j < get_arity(tree->children[i]); j++) {
            line++;
            set_line_number(tree->children[i]->children[j], line);
        }
    }
}

void set_line_number(Node *tree, int line) {
    tree->line = line;
    for (int i = 0; i < get_arity(tree); i++) {
        set_line_number(tree->children[i], line);
    }
}

void remove_nodes(Node* n) {
    if (n == nullptr) return;

    for (int i = 0; i < get_arity(n); i++) {
        remove_nodes(n->children[i]);
    }
    free(n->value);
    free(n->children);
}