#include "generator.h"

/* Searchs a cell without constraint in grid. Returns false if each cell
   have a constrait */
static bool get_empty_cell(completed_grid_t grid, size_t (* empty_cell)[2])
{
  bool grid_is_full = true;

  for (size_t i = 0; i < grid.data.dim; ++i)
  {
    for (size_t j = 0; j < grid.data.dim; ++j)
    {
      if (grid.data.rooms[i * grid.data.dim + j] == EMPTY)
      {
        grid_is_full = false;
        (*empty_cell)[ROW] = i;
        (*empty_cell)[COL] = j;
        /* Stops loops */
        i = grid.data.dim;
        j = grid.data.dim;
      }
    }
  }

  return grid_is_full;
}

/* Returns a random index in boolean array without paying attention to
   false elements */
static size_t pick_rd_index(bool * array, size_t array_size)
{
  size_t rd_nb = rand() % array_size;
  size_t index = 0;
  size_t cpt = 0;

  /* Jumps over first false booleans */
  while (!array[index])
  {
    index = (index + 1) % array_size;
  }
  while (cpt < rd_nb)
  {
    index = (index + 1) % array_size;
    if (array[index])
    {
      cpt++;
    }
  }
  /* Jumps over last false booleans */
  while (!array[index])
  {
    index = (index + 1) % array_size;
  }

  return index;
}

/* Sets the is_whithout_cstr array according to the placement of cell
   in grid */
static void get_dir_sides(completed_grid_t grid,
    size_t cell[2],
    bool * is_whithout_cstr)
{
  const size_t left_side = 0;
  const size_t down_side = grid.data.dim - 1;
  const size_t right_side = grid.data.dim - 1;
  const size_t up_side = 0;

  if (cell[ROW] == up_side)
  {
    is_whithout_cstr[UP] = false;
    if (cell[COL] == left_side)
    {
      is_whithout_cstr[LEFT] = false;
    }
    else if (cell[COL] == right_side)
    {
      is_whithout_cstr[RIGHT] = false;
    }
  }
  else if (cell[ROW] == down_side)
  {
    is_whithout_cstr[DOWN] = false;
    if (cell[COL] == left_side)
    {
      is_whithout_cstr[LEFT] = false;
    }
    else if (cell[COL] == right_side)
    {
      is_whithout_cstr[RIGHT] = false;
    }
  }
  else if (cell[COL] == left_side)
  {
    is_whithout_cstr[LEFT] = false;
  }
  else if (cell[COL] == right_side)
  {
    is_whithout_cstr[RIGHT] = false;
  }
}

/* Sets the is_whithout_cstr array according to its neighbors' constraints */
static void get_dir_neighbors(completed_grid_t grid,
    size_t cell[2],
    bool * is_whithout_cstr)
{
  const size_t left_side = 0;
  const size_t down_side = grid.data.dim - 1;
  const size_t right_side = grid.data.dim - 1;
  const size_t up_side = 0;

  if (cell[ROW] < down_side)
  {
    if (grid.data.rooms[(cell[ROW] + 1) * grid.data.dim + cell[COL]] != EMPTY)
    {
      is_whithout_cstr[DOWN] = false;
    }
  }
  if (cell[ROW] > up_side)
  {
    if (grid.data.rooms[(cell[ROW] - 1) * grid.data.dim + cell[COL]] != EMPTY)
    {
      is_whithout_cstr[UP] = false;
    }
  }
  if (cell[COL] < right_side)
  {
    if (grid.data.rooms[cell[ROW] * grid.data.dim + cell[COL] + 1] != EMPTY)
    {
      is_whithout_cstr[RIGHT] = false;
    }
  }
  if (cell[COL] > left_side)
  {
    if (grid.data.rooms[cell[ROW] * grid.data.dim + cell[COL] - 1] != EMPTY)
    {
      is_whithout_cstr[LEFT] = false;
    }
  }
}

/* Returns a direction where a cell has no constraint */
static size_t get_direction(completed_grid_t grid, size_t cell[2])
{
  size_t direction = NO_DIR;
  /* is_whithout_cstr is a boolean array which represents cell's neighbors
     in the grid. If an index is true, cell's neighbor hasn't a
     constraint and can potentially have the same as the current cell. */
  bool is_whithout_cstr[] = {true, true, true, true};

  get_dir_sides(grid, cell, is_whithout_cstr);

  get_dir_neighbors(grid, cell, is_whithout_cstr);

  if (is_whithout_cstr[RIGHT] ||
      is_whithout_cstr[DOWN] ||
      is_whithout_cstr[LEFT] ||
      is_whithout_cstr[UP])
  {
    direction = pick_rd_index(is_whithout_cstr, NB_MOVES);
  }

  return direction;
}

/* Fills the constraint of the current cell's neighbor by the current cell's
   constraint */
static void fill_cell_cstr(completed_grid_t * grid,
    size_t (* cell)[2],
    size_t direction)
{
  switch (direction)
  {
    case RIGHT:
      grid->data.rooms[(*cell)[ROW] * grid->data.dim + (*cell)[COL] + 1] =
        grid->data.nb_constraints;
      (*cell)[COL]++;
      break;

    case DOWN:
      grid->data.rooms[((*cell)[ROW] + 1) * grid->data.dim + (*cell)[COL]] =
        grid->data.nb_constraints;
      (*cell)[ROW]++;
      break;

    case LEFT:
      grid->data.rooms[(*cell)[ROW] * grid->data.dim + (*cell)[COL] - 1] =
        grid->data.nb_constraints;
      (*cell)[COL]--;
      break;

    case UP:
      grid->data.rooms[((*cell)[ROW] - 1) * grid->data.dim + (*cell)[COL]] =
        grid->data.nb_constraints;
      (*cell)[ROW]--;
      break;

    default:
      break;
  }
}

bool generate_rooms(completed_grid_t * grid, gen_opt_t gen_options)
{
  bool result = true;

  size_t cell[2];
  size_t room_size;
  size_t direction;

  grid->data.nb_constraints = 0;

  for (size_t i = 0; i < grid->data.dim; ++i)
  {
    for (size_t j = 0; j < grid->data.dim; ++j)
    {
      grid->data.rooms[i * grid->data.dim + j] = EMPTY;
    }
  }

  bool grid_is_full = get_empty_cell(*grid, &cell);

  while (!grid_is_full)
  {
    grid->data.nb_constraints++;
    grid->data.rooms[cell[ROW] * grid->data.dim + cell[COL]] =
      grid->data.nb_constraints;

    room_size = gen_options.min_room_sz +
      (rand() % (gen_options.max_room_sz - gen_options.min_room_sz + 1));

    direction = RIGHT;

    for (size_t i = FIRST_CELL; (i < room_size) && (direction != NO_DIR); ++i)
    {
      direction = get_direction(*grid, cell);
      fill_cell_cstr(grid, &cell, direction);
    }

    grid_is_full = get_empty_cell(*grid, &cell);
  }

  return result;
}
