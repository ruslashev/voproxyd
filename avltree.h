#pragma once

#include <stddef.h>

struct avl_node_t
{
    int key;
    size_t height;
    struct avl_node_t *left, *right;
};

struct avl_tree_t
{
    struct avl_node_t *root;
};

void avl_tree_construct(struct avl_tree_t *tree);
void avl_tree_destruct(struct avl_tree_t *tree);
void avl_tree_insert(struct avl_tree_t *tree, int key);
void avl_tree_delete(struct avl_tree_t *tree, int key);
int avl_tree_find(struct avl_tree_t *tree, int key);

