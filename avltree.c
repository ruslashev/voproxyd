#include "avltree.h"

#include <stdlib.h>

static void construct_avl_node(struct avl_node_t *node, int key, void *data)
{
    node->key = key;
    node->data = data;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
}

#define min(a, b) \
({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; \
})

#define max(a, b) \
({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; \
})

static size_t height(struct avl_node_t *x)
{
    return x != NULL ? x->height : 0;
}

static int diff(struct avl_node_t *x)
{
    return height(x->right) - height(x->left);
}

static struct avl_node_t* right_rotate(struct avl_node_t *x)
{
    struct avl_node_t *y = x->left;

    x->left = y->right;

    y->right = x;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

static struct avl_node_t* left_rotate(struct avl_node_t *x)
{
    struct avl_node_t *y = x->right;

    x->right = y->left;

    y->left = x;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

static struct avl_node_t* balance(struct avl_node_t *x)
{
    x->height = max(height(x->left), height(x->right)) + 1;

    if (diff(x) >= 2) {
        if (diff(x->right) < 0) {
            x->right = right_rotate(x->right);
        }

        return left_rotate(x);
    } else if (diff(x) <= -2) {
        if (diff(x->left) > 0) {
            x->left = left_rotate(x->left);
        }

        return right_rotate(x);
    }

    return x;
}

static struct avl_node_t* insert(struct avl_node_t *x, int key, void *data)
{
    struct avl_node_t *new;

    if (x == NULL) {
        new = malloc(sizeof(struct avl_node_t));
        construct_avl_node(new, key, data);
        return new;
    }

    if (key < x->key) {
        x->left = insert(x->left, key, data);
    } else {
        x->right = insert(x->right, key, data);
    }

    return balance(x);
}

static struct avl_node_t* min_node(struct avl_node_t *x)
{
    return x->left != NULL ? min_node(x->left) : x;
}

static struct avl_node_t* delete_min(struct avl_node_t *x)
{
    if (x->left == NULL) {
        return x->right;
    }

    x->left = delete_min(x->left);

    return balance(x);
}

static struct avl_node_t* delete_key(struct avl_node_t *x, int key)
{
    struct avl_node_t *l, *r, *m;

    if (x == NULL) {
        return NULL;
    }

    if (key < x->key) {
        x->left = delete_key(x->left, key);
    } else if (key > x->key) {
        x->right = delete_key(x->right, key);
    } else {
        l = x->left;
        r = x->right;

        free(x);

        if (r == NULL) {
            return l;
        }

        m = min_node(r);
        m->right = delete_min(r);
        m->left = l;

        return balance(m);
    }

    return balance(x);
}

static int find(struct avl_node_t *root, int key)
{
    if (root == NULL) {
        return 0;
    }

    if (key < root->key) {
        return find(root->left, key);
    }

    if (key == root->key) {
        return 1;
    }

    return find(root->right, key);
}

static void destruct(struct avl_node_t *node)
{
    if (node == NULL) {
        return;
    }

    destruct(node->left);
    destruct(node->right);

    free(node);
}

void avl_tree_construct(struct avl_tree_t *tree)
{
    tree->root = NULL;
}

void avl_tree_destruct(struct avl_tree_t *tree)
{
    destruct(tree->root);
}

void avl_tree_insert(struct avl_tree_t *tree, int key, void *data)
{
    tree->root = insert(tree->root, key, data);
}

void avl_tree_delete(struct avl_tree_t *tree, int key)
{
    tree->root = delete_key(tree->root, key);
}

int avl_tree_find(struct avl_tree_t *tree, int key)
{
    return find(tree->root, key);
}

