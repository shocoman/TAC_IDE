#ifndef __3ACTREE_H__
#define __3ACTREE_H__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
using namespace std;

enum DataType {
    CharType,   // 0
    IntType,    // 1
    DoubleType, // 2
    StringType, // 3
    LabelType,
    IdentType,
    UndefType
};

struct Node {
    int line;
    bool is_leaf;
    DataType data_type;
    char *value;

    Node *parent;
    Node **children;
    int arity;
};

char *emalloc(unsigned n);

Node *create_node(const char *val, DataType dt);
Node *create_node(const char *val, DataType dt, Node *fst_child, Node *snd_child);
Node *create_node(const char *val, DataType dt, list<Node *> children);

bool is_leaf(Node *node);

int get_arity(Node *node);
void print_tree(FILE *stream, Node *tree, int spaces);
void set_line_numbers_for_all_instructions(Node *tree);
void set_line_number(Node *tree, int line);
void remove_nodes(Node* n);
#endif
