#include "avl.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

uint32_t max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

AvlNode *avlnew(int id)
{
    AvlNode *node = malloc(sizeof(AvlNode));
    node->id = id;
    node->depth = 1;
    node->count = 1;
    node->left = node->right = node->parent = NULL;
    return node;
}

uint32_t avldepth(AvlNode *node)
{
    return node ? node->depth : 0;
}

uint32_t avlcount(AvlNode *node)
{
    return node ? node->count : 0;
}

void avlupdate(AvlNode *node)
{
    node->depth = 1 + max(avldepth(node->right), avldepth(node->left));
    node->count = 1 + avlcount(node->right) + avlcount(node->left);
}

AvlNode *avlrotl(AvlNode *node)
{
    AvlNode *newnode = node->right;
    if (newnode->left)
        newnode->left->parent = node;
    node->right = newnode->left;
    newnode->parent = node->parent;
    newnode->left = node;
    node->parent = newnode;
    avlupdate(node);
    avlupdate(newnode);

    return newnode;
}

AvlNode *avlrotr(AvlNode *node)
{
    AvlNode *newnode = node->left;
    if (newnode->right)
        newnode->right->parent = node;
    node->left = newnode->right;
    newnode->parent = node->parent;
    newnode->right = node;
    node->parent = newnode;
    avlupdate(node);
    avlupdate(newnode);

    return newnode;
}

AvlNode *avlfixr(AvlNode *node)
{
    if (avldepth(node->right->left) > avldepth(node->right->right))
        node->right = avlrotr(node->right);
    return avlrotl(node);
}

AvlNode *avlfixl(AvlNode *node)
{
    if (avldepth(node->left->right) > avldepth(node->left->left))
        node->left = avlrotl(node->left);
    return avlrotr(node);
}

AvlNode *avlfix(AvlNode *node)
{
    while (1) {
        AvlNode **from = NULL;
        if (node->parent) {
            from = node->parent->left == node ? &node->parent->left : &node->parent->right;
        }
        uint32_t l = avldepth(node->left);
        uint32_t r = avldepth(node->right);
        if (l > r + 1)
            node = avlfixl(node);
        if (r > l + 1)
            node = avlfixr(node);
        avlupdate(node);

        if (!from)
            break;
        *from = node;
        node = node->parent;
    }

    return node;
}

AvlNode *avldel(AvlNode *node)
{
    if (node->right == NULL) {
        if (node->left) {
            node->left->parent = node->parent;
        }
        AvlNode *parent = node->parent;
        if (parent) {
            if (parent->left == node) {
                parent->left = node->left;
            }
            else {
                parent->right = node->left;
            }

            return avlfix(parent);
        }

        return node->left;
    }

    AvlNode *successor = node->right;
    while (successor->left)
        successor = successor->left;
    AvlNode *root = avldel(successor);
    node->id = successor->id;
    free(successor);

    return root;
}

void avlins(AvlTree *tree, AvlNode *node)
{
    if (!tree->root) {
        tree->root = node;
        return;
    }
    AvlNode *root = tree->root;
    AvlNode **from = NULL;
    while (1) {
        from = root->id < node->id ? &root->right : &root->left;
        if (!*from) {
            *from = node;
            node->parent = root;
            break;
        }
        root = *from;
    }
    tree->root = avlfix(root);
}

void avlprint_node(AvlNode *node, uint32_t level)
{
    if (!node) {
        return;
    }
    if (level == 0) {
        printf("[%d] -> [", node->id);
        printf("%d,", node->left ? node->left->id : 0);
        printf("%d", node->right ? node->right->id : 0);
        printf("]\n");
        return;
    }
    avlprint_node(node->left, level - 1);
    avlprint_node(node->right, level - 1);
}

void avlprint(AvlTree *tree)
{
    uint32_t level = 0;
    while (level < tree->root->depth) {
        avlprint_node(tree->root, level++);
    }
}

void avlfree(AvlNode *node)
{
    if (node == NULL)
        return;

    AvlNode *next = node;
    while (next) {
        if (next->left) {
            next = next->left;
        }
        else if (next->right) {
            next = next->right;
        }
        else {
            node = next;
            next = next->parent;
            if (next && next->left == node) {
                next->left = NULL;
            }
            if (next && next->right == node) {
                next->right = NULL;
            }
            free(node);
        }
    }
}

void avlverify(AvlNode *node)
{
    if (!node)
        return;

    if (node->left) {
        assert(node->id >= node->left->id);
        assert(node->left->parent >= node);
    }
    if (node->right) {
        assert(node->id <= node->right->id);
        assert(node->right->parent >= node);
    }

    uint32_t l = avldepth(node->left);
    uint32_t r = avldepth(node->right);
    assert(node->depth == 1 + max(l, r));
    assert(node->count == 1 + avlcount(node->left) + avlcount(node->right));

    avlverify(node->left);
    avlverify(node->right);
}

void avlextract(AvlNode *node, int *idx, int *arr)
{
    if (!node)
        return;

    avlextract(node->left, idx, arr);
    *idx += 1;
    arr[*idx] = node->id;
    avlextract(node->right, idx, arr);
}
