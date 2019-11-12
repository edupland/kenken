#include "grid.h"

size_t sum_constr(int * data_constraint)
{
  size_t sum = 0;

  for (size_t i = 0; data_constraint[i] != LAST_ELMT; ++i)
  {
    sum += data_constraint[i];
  }

  return sum;
}

int prod_constr(int * data_constraint)
{
  int product = 1;

  for (size_t i = 0; data_constraint[i] != LAST_ELMT; ++i)
  {
    product *= data_constraint[i];
  }

  return product;
}

int diff_constr(int * data_constraint)
{
  size_t max = find_max(data_constraint);
  int difference = max + max;

  for (size_t i = 0; data_constraint[i] != LAST_ELMT; ++i)
  {
    difference -= data_constraint[i];
  }

  return difference;
}

double div_constr(int * data_constraint)
{
  size_t max = find_max(data_constraint);
  double quotient = max * max;

  for (size_t i = 0; data_constraint[i] != LAST_ELMT; ++i)
  {
    quotient /= data_constraint[i];
  }

  return quotient;
}

/* Finds Row, Column for current constraint's cells
   and returns how many cells have constraint_id */
size_t get_constraint_pos(grid_t grid,
    int ** data_constraint,
    size_t constraint_id)
{
  size_t count = 0;

  for (size_t i = 0; i < grid.dim; ++i)
  {
    for (size_t j = 0; j < grid.dim; ++j)
    {
      if (grid.rooms[i * grid.dim + j] == constraint_id)
      {
        (*data_constraint)[count + ROW_INDEX] = i;
        (*data_constraint)[count + COL_INDEX] = j;

        count += CELL_SZ;
      }
    }
  }

  return count / CELL_SZ;
}

size_t get_constraint_val(completed_grid_t grid,
    int ** data_constraint,
    size_t constraint_id)
{
  size_t count = 0;

  for (size_t i = 0; i < grid.data.dim; ++i)
  {
    for (size_t j = 0; j < grid.data.dim; ++j)
    {
      if (grid.data.rooms[i * grid.data.dim + j] == constraint_id)
      {
        (*data_constraint)[count] = grid.board[i * grid.data.dim + j];
        ++count;
      }
    }
  }

  return count;
}

/* Finds Maximal value in the current room */
int find_max(int * data_constraint)
{
  int result = 0;

  for (size_t i = 0; data_constraint[i] != LAST_ELMT; ++i)
  {
    if (data_constraint[i] > result)
    {
      result = data_constraint[i];
    }
  }

  return result;
}

/* grid_t */
void init_grid(grid_t *grid, size_t dim)
{
  grid->dim = dim;
  grid->rooms = malloc(sizeof(size_t) * dim * dim);

  if (grid->rooms == NULL)
  {
    printf("Error: file: grid_t.c, function: init_grid().\n"
        "Memory for rooms grid can't be allocated\n");
    exit(1);
  }

  for (size_t i = 0; i < dim * dim; ++i)
  {
    grid->rooms[i] = 0;
  }
}

void init_constraints(grid_t *grid)
{
  size_t max_room_id = 0;
  for (size_t i = 0; i < grid->dim * grid->dim; ++i)
  {
    if (grid->rooms[i] > max_room_id)
    {
      max_room_id = grid->rooms[i];
    }
  }

  grid->nb_constraints = max_room_id;
  grid->constraints =
    malloc(grid->nb_constraints * sizeof(constraint_t));

  if (grid->constraints == NULL)
  {
    printf("Error: file: grid_t.c, function: init_constraints().\n"
        "Memory for constraints grid can't be allocated\n");
    exit(1);
  }
}

void fill_grid_line(size_t row, size_t *buffer, grid_t *grid)
{
  for (size_t i = 0; i < grid->dim; ++i)
  {
    grid->rooms[row * grid->dim + i] = buffer[i];
  }
}
