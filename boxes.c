#include "boxes.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


/*insert a box with given side length and height length to a given box tree
 * time complexity O(log(n*m))*/
void INSERTBOX(boxes_t *boxes, float side, float height)
{
    tree_t *height_tree;
    node_t *found_node;
    int *countptr;
    int ret;

    ret = tree_search(&boxes->sidetree, side, &found_node);
    if (ret) {
        /* side not found - create new side tree */
        height_tree = (tree_t*)malloc(sizeof(*height_tree));
        tree_init(height_tree);
        tree_insert(&boxes->sidetree, side, height_tree);
        /* ... continue to creating new refcount variable */
    } else {
        /* side found - check if height exists */
        height_tree = (tree_t*)tree_node_get_value(found_node);
        ret = tree_search(height_tree, height, &found_node);
        if (!ret) {
            countptr = (int*)tree_node_get_value(found_node);
            ++(*countptr); /* height found - increment refcount */
            return;
        }
    }

    /* create new refcount variable */
    countptr = (int*)malloc(sizeof(*countptr));
    *countptr = 1;
    tree_insert(height_tree, height, countptr);
}

/*remove a specific box from box tree that has side and height length given
 * time complexity O(log(m*n))*/
int REMOVEBOX(boxes_t *boxes, float side, float height)
{
    node_t *side_node, *height_node;
    tree_t *height_tree;
    int *countptr;
    int ret;

    ret = tree_search(&boxes->sidetree, side, &side_node);
    if (ret) {
        return -1; /* side not found */
    }

    height_tree = (tree_t*)tree_node_get_value(side_node);
    ret = tree_search(height_tree, height, &height_node);
    if (ret) {
        return -1; /* height not found */
    }

    /* decrement refcount */
    countptr = (int*)tree_node_get_value(height_node);
    --(*countptr);

    if (*countptr == 0) {
        /* remove entry from height tree */
        tree_delete(height_tree, height_node);
        free(countptr);

        if (tree_is_empty(height_tree)) {
            /* height tree became empty, remove entry from side tree */
            tree_delete(&boxes->sidetree, side_node);
            free(height_tree);
        }
    }

    return 0;
}

static int boxes_find_ub(boxes_t *boxes, float side, float height,
                         float *found_side_p, float *found_height_p,
                         int find_first)
{
    node_t *side_node, *height_node;
    float found_side, found_height;
    float volume, min_volume;
    tree_t *height_tree;
    int is_found;
    int ret;

    ret = tree_ub(&boxes->sidetree, side, &side_node);
    if (ret) {
        return -1; /* side too big*/
    }

    is_found = 0;
    do {
        /* search in height tree */
        found_side  = tree_node_get_key(side_node);
        height_tree = tree_node_get_value(side_node);
        ret = tree_ub(height_tree, height, &height_node);
        if (!ret) {
            /* found in height tree */
            found_height = tree_node_get_key(height_node);
            volume = found_side * found_side * found_height;
            if (!is_found || (volume < min_volume)) {
                min_volume      = volume;
                *found_side_p   = found_side;
                *found_height_p = found_height;
                is_found        = 1;
                if (find_first) {
                    return 0;
                }
            }
        }

        /* not found in height tree, go to next height tree (next side node) */
        ret = tree_successor(&boxes->sidetree, &side_node);
    } while (!ret);

    return is_found ? 0 : -1;
}

int GETBOX(boxes_t *boxes, float side, float height, float *found_side_p,
           float *found_height_p)
{
    return boxes_find_ub(boxes, side, height, found_side_p, found_height_p, 0);
}

int CHECKBOX(boxes_t* boxes, float side, float height)
{
    float found_side, found_height;
    return boxes_find_ub(boxes, side, height, &found_side, &found_height, 1);
}

static void boxes_height_tree_print(int indent, void *value, const char *prefix)
{
    int *refcount = (int*)value;
    printf("%s%*s   + ref=%d\n", prefix, indent, "", *refcount);
}

static void boxes_side_tree_print(int indent, void *value, const char *prefix)
{
    tree_t *height_tree = (tree_t*)value;
    char *inner_prefix, *p;
    int i;

    inner_prefix = malloc(strlen(prefix) + indent + 5);
    p = inner_prefix;
    while (*prefix) {
        *(p++) = *(prefix++);
    }
    for (i = 0; i < indent + 3; ++i) {
        *(p++) = ' ';
    }
    *(p++) = '|';
    *(p++) = '\0';

    tree_print(height_tree, boxes_height_tree_print, inner_prefix);

    free(inner_prefix);
}

void boxes_print(boxes_t *boxes, const char *prefix)
{
    tree_print(&boxes->sidetree, boxes_side_tree_print, prefix);
}

void boxes_init(boxes_t *boxes)
{
    tree_init(&boxes->sidetree);
}

static void boxes_height_tree_cleanup_cb(void *value)
{
    int *refcount = (int*)value;

    assert(refcount != NULL);
    free(refcount);
}

static void boxes_side_tree_cleanup_cb(void *value)
{
    tree_t *height_tree = (tree_t*)value;

    tree_cleanup(height_tree, boxes_height_tree_cleanup_cb);
    free(height_tree);
}

void boxes_cleanup(boxes_t *boxes)
{
    tree_cleanup(&boxes->sidetree, boxes_side_tree_cleanup_cb);
}
