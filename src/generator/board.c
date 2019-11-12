#include "generator.h"

/* Fills the grid with a preselected latin square */
static void fill_grid(completed_grid_t * grid)
{
  for (size_t i = 0; i < grid->data.dim; ++i)
  {
    for (size_t j = 0; j < grid->data.dim; ++j)
    {
      grid->board[i * grid->data.dim + ((j + i) % grid->data.dim)] =
        j + 1;
    }
  }
}

/* Picks 2 different random numbers */
static void pick_rd_nbs(size_t * rd_nb1, size_t * rd_nb2, size_t dim)
{
  *rd_nb1 = rand() % dim;
  do
  {
    *rd_nb2 = rand() % dim;
  } while (*rd_nb1 == *rd_nb2);
}

/* Swapping functions */
static void swap_rows(completed_grid_t * grid)
{
  size_t rd_nb1;
  size_t rd_nb2;
  size_t tmp;

  pick_rd_nbs(&rd_nb1, &rd_nb2, grid->data.dim);

  for (size_t j = 0; j < grid->data.dim; ++j)
  {
    tmp = grid->board[rd_nb1 * grid->data.dim + j];
    grid->board[rd_nb1 * grid->data.dim + j] =
      grid->board[rd_nb2 * grid->data.dim + j];
    grid->board[rd_nb2 * grid->data.dim + j] = tmp;
  }
}

static void swap_cols(completed_grid_t * grid)
{
  size_t rd_nb1;
  size_t rd_nb2;
  size_t tmp;

  pick_rd_nbs(&rd_nb1, &rd_nb2, grid->data.dim);

  for (size_t j = 0; j < grid->data.dim; ++j)
  {
    tmp = grid->board[grid->data.dim * j + rd_nb1];
    grid->board[grid->data.dim * j + rd_nb1] =
      grid->board[grid->data.dim * j + rd_nb2];
    grid->board[grid->data.dim * j + rd_nb2] = tmp;
  }
}

bool generate_board(completed_grid_t * grid)
{
  bool result = true;

  fill_grid(grid);

  bool rd_swap;

  /* Swaps rows or columns 2 * grid's dimension times */
  for (size_t i = 0; i < 2 * grid->data.dim; ++i)
  {
    /* 0 = Swaps columns, 1 = Swaps rows */
    rd_swap = rand() % 2;

    if (rd_swap)
    {
      swap_rows(grid);
    }
    else
    {
      swap_cols(grid);
    }
  }

  return result;
}
