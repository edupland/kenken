#ifndef PARSER_H
#define PARSER_H

#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "grid.h"

#define MAX_GRID_SIZE 50
#define INT_LEFT_SHIFT(i) (i * 10)
#define ASCII_TO_INT(c) (c - 48)
#define READ_INT_ERROR -1
#define GET_LINE_ERROR -1

/* Parsing */
bool parse(FILE *file, grid_t *grid);

#endif
