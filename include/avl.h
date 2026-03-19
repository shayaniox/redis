#ifndef _AVL_H
#define _AVL_H

#include <stdint.h>

typedef struct AvlNode {
    int id;
    uint32_t depth;
    uint32_t count;
    struct AvlNode *left;
    struct AvlNode *right;
    struct AvlNode *parent;
} AvlNode;

AvlNode *avlnew(int id);
uint32_t avldepth(AvlNode *node);
uint32_t avlcount(AvlNode *node);
void avlupdate(AvlNode *node);
AvlNode *avlrotl(AvlNode *node);
AvlNode *avlrotr(AvlNode *node);
AvlNode *avlfixr(AvlNode *node);
AvlNode *avlfixl(AvlNode *node);
AvlNode *avlfix(AvlNode *node);

typedef struct AvlTree {
    AvlNode *root;
} AvlTree;

AvlNode *avldel(AvlNode *tree);
void avlins(AvlTree *tree, AvlNode *node);
void avlprint(AvlTree *tree);
void avlprint_node(AvlNode *node, uint32_t depth);
void avlfree(AvlNode *node);
void avlverify(AvlNode *node);
void avlextract(AvlNode *node, int *idx, int *arr);

#endif
