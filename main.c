#include "trees.h"
#include "boxes.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 100

/*
 * parse a command with two floating-point arguments
 * syntax: "<command>(<arg1>,<arg2>)"
 * the length of command must be al least the length of the line
 */
static int parse_line(const char *line, char *command, float *arg1_p, float *arg2_p)
{
    const char *p;
    int ret;

    p = line;

    /* read until '(' and copy to 'command' */
    while ((*p != '\0') && (*p != '(')) {
        *(command++) = *(p++);
    }
    *command = '\0';

    ret = sscanf(p, "(%f,%f)", arg1_p, arg2_p);
    if (ret != 2) {
        return -1;
    }

    return 0;
}

static void print_search_result(int ret, float side, float height)
{
    printf("A box which can fit (side: %.2f height: %.2f) is %sfound\n",
            side, height, ret ? "not " : "");
}

static int do_command(boxes_t *boxes, const char *command, float side,
                      float height)
{
    float found_side, found_height;
    int ret;

    LOG("doing %s(%f,%f)", command, side, height);

    if (!strcmp(command,"INSERTBOX")) {
        INSERTBOX(boxes, side, height);
    } else if (!strcmp(command,"REMOVEBOX")) {
        ret = REMOVEBOX(boxes, side, height);
        if (ret) {
            printf("Failed to remove (side: %.2f height: %.2f)\n", side, height);
        }
    } else if (!strcmp(command,"GETBOX")) {
        ret = GETBOX(boxes, side, height, &found_side, &found_height);
        if (!ret) {
            printf("The minimal volume of a box which can fit "
                   "(side: %.2f height: %.2f) is: (side: %.2f height: %.2f)\n",
                   side, height, found_side, found_height);
        } else {
            print_search_result(ret, side, height);
        }
    } else if (!strcmp(command,"CHECKBOX")) {
        ret = CHECKBOX(boxes, side, height);
        print_search_result(ret, side, height);
     } else {
        printf("Invalid command: '%s'\n", command);
        return -1;
    }

#ifdef DEBUG
    printf("   ===== boxes ====\n");
    boxes_print(boxes, "   =  ");
    printf("   ================\n");
#endif
    return 0;
}

int main(int argc, char *argv[])
{
    char command[MAXLINE];
    float side, height;
    boxes_t boxes;
    int ret;

    boxes_init(&boxes);

    /*there is not a file to read from so use menu*/
    if (argc == 1) {
        int cont;

        printf("There are 4 options to cont from: \n");
        printf("INSERTBOX - insert a box with side and height given \n");
        printf("REMOVEBOX- remove a box with side and height given \n");
        printf("GETBOX- return a box with minimal dimensions with at least height and side length given \n");
        printf("CHECKBOX- check if there is a box with at least height and side length given \n");

        do {
            printf("Please choose which command to do: \n");
            scanf("%s", command);
            printf("Choose side length\n");
            scanf("%f", &side);
            printf("Choose height length\n");
            scanf("%f", &height);

            ret = do_command(&boxes, command, side, height);
            if (ret) {
                goto out_cleanup;
            }

            printf("Do you wish to continue? press 1 if yes and 0 if not\n");
            scanf("%d", &cont);
        } while (cont);
    } else if (argc == 2) {
        char line[MAXLINE];
        int line_num;
        FILE *fp;

        fp = fopen(argv[1], "r");
        if (!fp) {
            printf("Failed to open '%s': %m\n", argv[1]);
            ret = -1;
            goto out_cleanup;
        }

        line_num = 1;
        while (fgets(line, sizeof(line), fp) != NULL) {
            ret = parse_line(line, command, &side, &height);
            if (ret) {
                printf("Syntax error in line %d '%s'\n", line_num, line);
                fclose(fp);
                goto out_cleanup;
            }

            ret = do_command(&boxes, command, side, height);
            if (ret) {
                fclose(fp);
                goto out_cleanup;
            }

            ++line_num;
        }

        fclose(fp);
    } else {
        printf("Invalid number of command line arguments\n");
    }

out_cleanup:
    /*free the boxes data structure*/
    boxes_cleanup(&boxes);
    return ret;
}
