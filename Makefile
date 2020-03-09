all: boxes

boxes: main.c trees.c boxes.c
	gcc -Wall -Werror -g main.c trees.c boxes.c -o boxes -lm