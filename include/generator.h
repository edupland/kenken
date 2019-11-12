#ifndef GENERATOR_H
#define GENERATOR_H

#include <math.h>
#include <stdbool.h>
#include <time.h>

#include "grid.h"
#include "output.h"
#include "solver.h"

#define MAX_CHAR_GEN 3
#define MAX_SZ_NAME 100
#define NB_DIGITS(a) floor(log10(a)) + 1

/* Rooms' Macros */
#define EMPTY 0
#define NB_MOVES 4
#define FIRST_CELL 1

/* Rooms' enum */
typedef enum
{
  ROW,
  COL
} data_set;

typedef enum
{
  RIGHT,
  DOWN,
  LEFT,
  UP,
  NO_DIR
} directions;

typedef struct gen_opt_t
{
  size_t size;
  size_t min_room_sz;
  size_t max_room_sz;
  bool operators[4];
  bool unique;
  char * filename;
} gen_opt_t;

bool generate(gen_opt_t gen_options);

/* Uses the latin square rule : swapping 2 columns or 2 rows in a latin square
   doesn't break its properties */
bool generate_board(completed_grid_t * grid);

bool generate_rooms(completed_grid_t * grid, gen_opt_t gen_options);

bool generate_constraints(completed_grid_t * grid, gen_opt_t gen_options);

#endif
