#ifndef SOLVER_H
#define SOLVER_H

#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "grid.h"
#include "output.h"

#define BYTE_SIZE sizeof(char)
#define MAX_BIT BYTE_SIZE * 64

typedef enum
{
  BACK_TRACKING,
  EQ_SYSTEM,
  INEQ_SYSTEM,
  LINEAR_INEQ_SYSTEM,
  LOGIC
} solvers;

typedef struct solve_opt_t
{
  bool all_sols;
  bool verbose_mod;
  bool unique;
  size_t solver;
  char * filename;
} solve_opt_t;

/* back_tracking_container_t */
typedef struct cells_list_t
{
  size_t i;
  size_t j;
  struct cells_list_t *next;
} cells_list_t;

cells_list_t **initialize_rooms(grid_t grid);

typedef struct back_tracking_container_t
{
  grid_t grid;
  size_t *board;
  cells_list_t **rooms;
  solve_opt_t options;
} back_tracking_container_t;

/* logic_container_t */

typedef struct cell_t
{
  size_t i;
  size_t j;
} cell_t;

typedef struct room_t
{
  size_t size;
  constraint_t constraint;
  cell_t *cell_list;
} room_t;

room_t *initialize_rooms_logic(grid_t grid);

typedef struct logic_container_t
{
  size_t dim;
  unsigned long long *board;
  size_t nb_rooms;
  room_t *rooms;
  solve_opt_t options;
} logic_container_t;

/* Solvers */
bool back_tracking(grid_t grid, solve_opt_t solve_options);
bool equations_system(grid_t grid, solve_opt_t solve_options);
bool inequalities_system(grid_t grid, solve_opt_t solve_options);
bool linear_inequalities_system(grid_t grid, solve_opt_t solve_options);
bool logic(grid_t grid, solve_opt_t solve_options);

/* -u option */
bool check_uniqueness(grid_t grid);


#endif
