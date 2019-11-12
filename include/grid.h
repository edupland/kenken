#ifndef GRID_H
#define GRID_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LAST_ELMT -1
#define CONSTRAINT_SIZE 2
#define NB_OPERATORS 4

typedef enum
{
  ROW_INDEX,
  COL_INDEX,
  CELL_SZ
} index_cell;

typedef enum
{
  PLUS,
  MINUS,
  TIMES,
  DIV,
  EQ
} operation_t;

typedef struct
{
  size_t target;
  operation_t op;
} constraint_t;

int find_max(int * data_constraint);
size_t sum_constr(int * data_constraint);
int prod_constr(int * data_constraint);
int diff_constr(int * data_constraint);
double div_constr(int * data_constraint);

typedef struct
{
  size_t dim;
  size_t *rooms;
  size_t nb_constraints;
  constraint_t *constraints;
} grid_t;

void init_grid(grid_t *grid, size_t dim);
void init_constraints(grid_t *grid);
void fill_grid_line(size_t row, size_t *buffer, grid_t *grid);
size_t get_constraint_pos(grid_t grid,
    int ** data_constraint,
    size_t constraint_id);

typedef struct
{
  size_t *board;
  grid_t data;
} completed_grid_t;

size_t get_constraint_val(completed_grid_t grid,
    int ** data_constraint,
    size_t constraint_id);

#endif
