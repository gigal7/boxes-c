/*
 *
 *
 *
 */

#ifndef _TREES_H
#define _TREES_H


/* Red-Black color type */
typedef enum {
    RED,
    BLACK
} color_t;


/* Red-Black tree node */
typedef struct node_s node_t;
struct node_s {
    float     key;
    color_t   color;
    void      *value;
    node_t    *parent;
    node_t    *left;
    node_t    *right;
};


/* Red-Black tree */
typedef struct tree_s {
    node_t    *root;
    node_t    nil;
} tree_t;


typedef void (*tree_cleanup_cb_t)(void *value);

typedef void (*tree_print_cb_t)(int indent, void *value, const char *prefix);


/*
 * init the tree
 */
void tree_init(tree_t *tree);


/*
 * cleanup the tree, call 'cb' for each removed key/value pair
 */
void tree_cleanup(tree_t *tree, tree_cleanup_cb_t cb);


/* @return nonzero if the tree is empty */
int tree_is_empty(const tree_t *tree);


/*
 * print the tree, call 'cb' for each value to print
 */
void tree_print(const tree_t *tree, tree_print_cb_t cb, const char *prefix);


/*
 * returns 0 on success, -1 on failure
 * */
int tree_search(const tree_t *tree, float key, node_t **node_p);


/*
 * insert <key,value> into the tree
 * returns 0 on success, -1 on failure
 */
int tree_insert(tree_t *tree, float key, void *value);


/*
 * removes a key from the tree
 * fills *value_p if found
 */
void tree_delete(tree_t *tree, node_t *node);


/*
 * find lowest key which is larger or equal to the provided "ub"
 * returns 0 on success, -1 on failure
 */
int tree_ub(const tree_t *tree, float key, node_t **node_p);


/* move node_p to point to the tree successor node
 * return 0 if success, -1 if no successor (*node_p was last node in the tree)
 */
int tree_successor(const tree_t *tree, node_t **node_p);


/* Get key/value of node pointer */
float tree_node_get_key(node_t *node);
void* tree_node_get_value(node_t *node);


#endif
