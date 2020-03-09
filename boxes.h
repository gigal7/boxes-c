#ifndef _BOXES_H
#define _BOXES_H

#include "trees.h"


typedef struct boxes_s {
    tree_t  sidetree;
} boxes_t;


void boxes_init(boxes_t *boxes);
void boxes_cleanup(boxes_t *boxes);
void boxes_print(boxes_t *boxes, const char *prefix);

void INSERTBOX(boxes_t *boxes, float side, float height);
int REMOVEBOX(boxes_t *boxes, float side, float height);

/* get minimal box which can contain (side,height)
 *
 * @return 0 if found, -1 if not found
 */
int GETBOX(boxes_t *boxes, float side, float height, float *found_side_p,
           float *found_height_p);

/* @return 0 if found, -1 if not found
 */
int CHECKBOX(boxes_t *boxes, float side, float height);

#endif
