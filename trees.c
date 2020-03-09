#include "trees.h"
#include "assert.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


static int float_equal(float n1, float n2)
{
    float delta = 0.001;
    return fabs(n1 - n2) < delta;
}

void tree_init(tree_t *tree)
{
    tree->root       = &tree->nil;
    tree->nil.color  = BLACK;
    tree->nil.left   = NULL;
    tree->nil.right  = NULL;
    tree->nil.parent = NULL;
}

static void tree_do_cleanup(tree_t *tree, node_t *root,
                            tree_cleanup_cb_t cb)
{
    if (root == &tree->nil) {
        return;
    }

    cb(root->value);
    tree_do_cleanup(tree, root->left, cb);
    tree_do_cleanup(tree, root->right, cb);
    free(root);
}

void tree_cleanup(tree_t *tree, tree_cleanup_cb_t cb)
{
    tree_do_cleanup(tree, tree->root, cb);
    tree->root = &tree->nil;
}

int tree_is_empty(const tree_t *tree)
{
    return tree->root == &tree->nil;
}

static void tree_do_print(const tree_t *tree, node_t *root, char type,
                          int indent, tree_print_cb_t cb, const char *prefix)
{
    if (root == &tree->nil) {
        return;
    }

    printf("%s%*s[%c] %.2f\n", prefix, indent, "", type, root->key);
    cb(indent + 2, root->value, prefix);

    tree_do_print(tree, root->left,  'l', indent + 2, cb, prefix);
    tree_do_print(tree, root->right, 'r', indent + 2, cb, prefix);
}

void tree_print(const tree_t *tree, tree_print_cb_t cb, const char *prefix)
{
    tree_do_print(tree, tree->root, '*', 0, cb, prefix);
}

static node_t* tree_do_search(const tree_t *tree, node_t *root, float key)
{
    if ((root == &tree->nil) || float_equal(key, root->key)) {
        return root;
    } else {
        node_t *node = tree_do_search(tree, root->left, key);
        if (node != &tree->nil) {
            return node;
        } else {
            return tree_do_search(tree, root->right, key);
        }
    }
}

int tree_search(const tree_t *tree, float key, node_t **node_p)
{
    node_t *node = tree_do_search(tree, tree->root, key);
    if (node == &tree->nil) {
        return -1; /* not found */
    } else {
        *node_p = node;
        return 0;
    }
}

static node_t *tree_new_node(tree_t *tree, float key, void *value,
                             color_t color)
{
    node_t *node;

    node = (node_t*)malloc(sizeof(*node));
    node->key    = key;
    node->value  = value;
    node->color  = color;
    node->left   = &tree->nil;
    node->right  = &tree->nil;
    node->parent = &tree->nil;
    return node;
}

/*rotation of a node to the left
 * time complexity o(1)*/
void tree_left_rotate(tree_t *tree, node_t *x)
{
    node_t *y;

    y        = x->right;
    x->right = y->left;

    if (y->left != &tree->nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == &tree->nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left   = x;
    x->parent = y;
}

/*rotation of a node to the right
 * time complexity o(1)*/
void tree_right_rotate(tree_t *tree, node_t *x)
{
    node_t *y;

    y       = x->left;
    x->left = y->right;

    if (y->right != &tree->nil) {
        y->right->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == &tree->nil) {
        tree->root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }

    y->right  = x;
    x->parent = y;
}
/*
 * fix red black tree (with n nodes) violations of inserting a new node
 * time complexity o(logn)
 **/
static void tree_insert_fixup(tree_t *tree, node_t *z)
{
    node_t *y;

    while ((z != tree->root) && (z->parent->color == RED)) {
        if (z->parent == z->parent->parent->left) {
            y = z->parent->parent->right;
            if (y->color == RED) {
                z->parent->color         = BLACK;
                y->color                 = BLACK;
                z->parent->parent->color = RED;
                z                        = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    tree_left_rotate(tree, z);
                }
                z->parent->color         = BLACK;
                z->parent->parent->color = RED;
                tree_right_rotate(tree, z->parent->parent);
            }
        } else {
            y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color         = BLACK;
                y->color                 = BLACK;
                z->parent->parent->color = RED;
                z                        = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    tree_right_rotate(tree, z);
                }
                z->parent->color         = BLACK;
                z->parent->parent->color = RED;
                tree_left_rotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
}

int tree_insert(tree_t *tree, float key, void *value)
{
    node_t *x, *y, *z;

    z = tree_new_node(tree, key, value, RED);
    y = &tree->nil;
    x = tree->root;

    while (x != &tree->nil){
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == &tree->nil) {
        tree->root = z;
    } else if (z->key < y->key) {
        y->left = z;
    } else if (z->key > y->key) {
        y->right = z;
    } else {
        return -1; /* already exists */
    }

    tree_insert_fixup(tree, z);
    return 0;
}

/*find a minimum key in a tree with n nodes
 * time complexity o(logn)*/
static node_t* tree_find_min(const tree_t* tree, node_t *root)
{
    node_t *x;

    if (root == &tree->nil) {
        return root;
    }

    for (x = root; x->left != &tree->nil; x = x->left);
    return x;
}

static node_t* tree_get_successor(const tree_t *tree, node_t *x)
{
    node_t *y;

    if (x->right != &tree->nil) {
        /* right tree nonempty, so successor is there */
        return tree_find_min(tree, x->right);
    }

    /* climb up */
    y = x->parent;
    while ((y != &tree->nil) && (x == y->right)) {
        x = y;
        y = y->parent;
    }
    return y;
}

void tree_delete_fixup(tree_t *tree, node_t *x)
{
    node_t *w;

    while ((x != tree->root) && (x->color == BLACK)) {
        if (x == x->parent->left) {
            w = x->parent->right;
            if (w->color == RED) {
                w->color         = BLACK;
                x->parent->color = RED;
                tree_left_rotate(tree, x->parent);
                w = x->parent->right;
            }
            if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
                w->color = RED;
                x        = x->parent;
            } else {
                if (w->right->color == BLACK){
                    w->left->color = BLACK;
                    w->color       = RED;
                    tree_right_rotate(tree, w);
                    w = x->parent->right;
                }

                w->color =               x->parent->color;
                x->parent->parent->color = BLACK;
                w->right->color          = BLACK;
                tree_left_rotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            w = x->parent->left;
            if (w->color == RED) {
                w->color         = BLACK;
                x->parent->color = RED;
                tree_right_rotate(tree, x->parent);
                w = x->parent->left;
            }
            if ((w->right->color == BLACK) && (w->left->color == BLACK)) {
                w->color = RED;
                x        = x->parent;
            } else {
                if (w->left->color == BLACK){
                    w->right->color = BLACK;
                    w->color        = RED;
                    tree_left_rotate(tree, w);
                    w = x->parent->left;
                }

                w->color =               x->parent->color;
                x->parent->parent->color = BLACK;
                w->left->color           = BLACK;
                tree_right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    tree->root->color = BLACK;
}

void tree_delete(tree_t *tree, node_t *node)
{
    node_t *x, *y;

    if ((node->left == &tree->nil) || (node->right == &tree->nil)) {
        y = node;
    } else {
        y = tree_get_successor(tree, node);
    }

    if (y->left != &tree->nil) {
        x = y->left;
    } else {
        x = y->right;
    }

    x->parent = y->parent;
    if (y->parent == &tree->nil) {
        tree->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    if (y != node) {
        node->key   = y->key;
        node->value = y->value;
    }

    if (y->color == BLACK) {
        tree_delete_fixup(tree, x);
    }

    free(y);
}

static node_t *tree_do_ub(const tree_t *tree, node_t *root, float key)
{
    if ((root == &tree->nil) || float_equal(root->key, key)) {
        return root;
    } else if (key < root->key) {
        /* key is lower than root, so upper bound is either in the left
         * subtree, or the root itself
         */
        node_t *node = tree_do_ub(tree, root->left, key);
        if (node != &tree->nil) {
            return node;
        } else {
            return root;
        }
    } else {
        /* key is larger than root, so the upper bound, if exists, must be
         * on the right subtree.
         */
        return tree_do_ub(tree, root->right, key);
    }
}

float tree_node_get_key(node_t *node)
{
    return node->key;
}

void* tree_node_get_value(node_t *node)
{
    return node->value;
}

int tree_ub(const tree_t *tree, float key, node_t **node_p)
{
    node_t *node = tree_do_ub(tree, tree->root, key);

    if (node == &tree->nil) {
        return -1;
    } else {
        assert(node->key >= key);
        *node_p = node;
        return 0;
    }
}

int tree_successor(const tree_t *tree, node_t **node_p)
{
    node_t *succ = tree_get_successor(tree, *node_p);
    if (succ == &tree->nil) {
        return -1;
    } else {
        *node_p = succ;
        return 0;
    }
}
