#include "solver.h"

size_t nb_sol;

/* back_tracking_container_t */
static size_t *initialize_board(grid_t grid)
{

  size_t *board = malloc(sizeof(size_t) * grid.dim * grid.dim);

  if (board == NULL)
  {
    printf("Error: file: back_tracking.c, function: initialize_board().\n"
        "Memory for board can't be allocated\n");
    exit(1);
  }

  for (size_t i = 0; i < grid.dim * grid.dim; ++i)
  {
    board[i] = 0;
  }

  return board;
}


static void initialize_btc(grid_t grid,
    back_tracking_container_t *btc,
    solve_opt_t solve_options)
{
  btc->grid = grid;
  btc->board = initialize_board(grid);
  btc->rooms = initialize_rooms(grid);
  btc->options = solve_options;
}

static void print_board(grid_t grid, size_t *board)
{
  for (size_t i = 0; i < grid.dim; ++i)
  {
    for (size_t j = 0; j < grid.dim - 1; ++j)
    {
      printf("%zu ", board[i * grid.dim + j]);
    }
    printf("%zu\n", board[i * grid.dim + grid.dim - 1]);
  }
  printf("-------\n");
}

static void free_btc(back_tracking_container_t btc)
{

  for (size_t i = 0; i < btc.grid.nb_constraints; ++i)
  {
    cells_list_t *prec = btc.rooms[i];
    cells_list_t *tmp = btc.rooms[i];
    while (tmp)
    {
      tmp = tmp->next;
      free(prec);
      prec = tmp;
    }
  }

  free(btc.board);
  free(btc.rooms);
}

static bool check_plus(size_t i,
    size_t j,
    size_t n,
    back_tracking_container_t btc)
{

  constraint_t c_id =
    btc.grid.constraints[btc.grid.rooms[i * btc.grid.dim + j] - 1];
  cells_list_t *tmp = NULL;
  size_t total = n;

  /* Size of the room */
  size_t size = 0;

  /* Number of cells that contain numbers in that room.
   * Starts at 1 to take the current cell size_to account.
   * */
  size_t filled_size = 1;

  tmp = btc.rooms[btc.grid.rooms[i * btc.grid.dim + j] - 1];

  while (tmp)
  {
    size_t cell_value = btc.board[tmp->i * btc.grid.dim + tmp->j];

    if (cell_value != 0)
    {
      filled_size++;
      total += cell_value;
    }

    size++;
    tmp = tmp->next;
  }

  /* Does the room still have free cells and the total does not exceed
   * the target, or is it filled and the total is equal to the target ?
   * */
  return (size > filled_size && total < c_id.target) ||
    (total == c_id.target && size == filled_size);
}

static bool check_times(size_t i,
    size_t j,
    size_t n,
    back_tracking_container_t btc)
{

  constraint_t c_id =
    btc.grid.constraints[btc.grid.rooms[i * btc.grid.dim + j] - 1];
  cells_list_t *tmp = NULL;
  size_t total = n;
  size_t size = 0;
  size_t filled_size = 1;

  tmp = btc.rooms[btc.grid.rooms[i * btc.grid.dim + j] - 1];

  while (tmp)
  {
    size_t cell_value = btc.board[tmp->i * btc.grid.dim + tmp->j];

    if (cell_value != 0)
    {
      filled_size++;
      total *= cell_value;
    }

    size++;
    tmp = tmp->next;
  }

  return (size > filled_size && total <= c_id.target) ||
    (total == c_id.target && size == filled_size);
}

static bool check_minus(size_t i,
    size_t j,
    size_t n,
    back_tracking_container_t btc)
{

  constraint_t c_id =
    btc.grid.constraints[btc.grid.rooms[i * btc.grid.dim + j] - 1];
  cells_list_t *tmp = NULL;
  int total = -n;
  size_t size = 0;
  size_t filled_size = 1;
  int max = n;

  tmp = btc.rooms[btc.grid.rooms[i * btc.grid.dim + j] - 1];

  while (tmp)
  {
    int cell_value = btc.board[tmp->i * btc.grid.dim + tmp->j];

    if (cell_value != 0)
    {
      filled_size++;
      total -= cell_value;
    }

    if (cell_value > max)
    {
      max = cell_value;
    }

    size++;
    tmp = tmp->next;
  }

  /* Adding max two times so that every other number in the room
   * will be substracted to it
   * */
  total += 2 * max;
  int target = c_id.target;

  return (size > filled_size) ||
    (total == target && size == filled_size);
}

static bool check_div(size_t i,
    size_t j,
    size_t n,
    back_tracking_container_t btc)
{
  constraint_t c_id =
    btc.grid.constraints[btc.grid.rooms[i * btc.grid.dim + j] - 1];
  cells_list_t *tmp = NULL;
  double total_d = 1 / (double)n;
  size_t size = 0;
  size_t filled_size = 1;
  double max_d = n;

  tmp = btc.rooms[btc.grid.rooms[i * btc.grid.dim + j] - 1];

  while (tmp)
  {
    double cell_value_d = btc.board[tmp->i * btc.grid.dim + tmp->j];

    if (cell_value_d != 0)
    {
      filled_size++;
      total_d /= cell_value_d;
    }

    if (cell_value_d > max_d)
    {
      max_d = cell_value_d;
    }

    size++;
    tmp = tmp->next;
  }

  total_d *= max_d * max_d;
  double target_d = c_id.target;

  return (size > filled_size) ||
    (total_d == target_d && size == filled_size);
}

static bool check_eq(size_t i,
    size_t j,
    size_t n,
    back_tracking_container_t btc)
{

  return (n ==
      btc.grid.constraints[btc.grid.rooms[i * btc.grid.dim + j] - 1].target);
}

static bool check_cage(size_t i,
    size_t j,
    size_t n,
    back_tracking_container_t btc)
{

  bool res = false;

  switch (btc.grid.constraints[btc.grid.rooms[i * btc.grid.dim + j] - 1].op)
  {
    case (PLUS) :
      res = check_plus(i, j, n, btc);
      break;

    case (TIMES) :
      res = check_times(i, j, n, btc);
      break;

    case (MINUS) :
      res = check_minus(i, j, n, btc);
      break;

    case (DIV) :
      res = check_div(i, j, n, btc);
      break;

    case (EQ) :
      res = check_eq(i, j, n, btc);
      break;

    default :
      break;
  }

  return res;
}

static bool check_row(size_t row, size_t n, grid_t grid, size_t *board)
{
  for (size_t j = 0; j < grid.dim; ++j)
  {
    if (board[row * grid.dim + j] == n)
    {
      return false;
    }
  }
  return true;
}

static bool check_col(size_t col, size_t n, grid_t grid, size_t *board)
{
  for (size_t i = 0; i < grid.dim; ++i)
  {
    if (board[i * grid.dim + col] == n)
    {
      return false;
    }
  }
  return true;
}

static bool is_valid(size_t i,
    size_t j,
    size_t n,
    back_tracking_container_t btc)
{
  return check_row(i, n, btc.grid, btc.board)
    && check_col(j, n, btc.grid, btc.board)
    && check_cage(i, j, n, btc);
}

/* Finds the first empty cell of the board
 * If none is found, the grid is solved
 */
static bool find_empty(size_t *i, size_t *j, back_tracking_container_t btc)
{
  for (size_t r = 0; r < btc.grid.dim; ++r)
  {
    for (size_t c = 0; c < btc.grid.dim; ++c)
    {
      if (btc.board[r * btc.grid.dim + c] == 0)
      {
        *i = r;
        *j = c;
        return true;
      }
    }
  }

  return false;
}

static bool solve_rec(back_tracking_container_t btc)
{
  size_t i;
  size_t j;
  /* Find an empty cell */
  if (find_empty(&i, &j, btc))
  {
    /* Test all possible values for that cell */
    for (size_t n = 1; n <= btc.grid.dim; ++n)
    {
      /* Is adding the number n at cell (i, j) is a valid move ? */
      if (is_valid(i, j, n, btc))
      {
        btc.board[i * btc.grid.dim + j] = n;

        if (solve_rec(btc))
        {
          return true;
        }

        /* Reset the cell if the number doesn't fit */
        btc.board[i * btc.grid.dim + j] = 0;
      }
    }
    return false;
  }

  print_board(btc.grid, btc.board);

  if (btc.options.all_sols)
  {
    nb_sol++;
    return false;
  }
  else
  {
    return true;
  }
}

bool back_tracking(grid_t grid, solve_opt_t solve_options)
{

  back_tracking_container_t btc;
  initialize_btc(grid, &btc, solve_options);
  int stdout_fd;
  bool ret = true;

  if (solve_options.filename != NULL)
  {
    redirect_stdout(solve_options.filename, &stdout_fd);
  }

  if (btc.options.all_sols)
  {
    nb_sol = 0;
  }

  if (!solve_rec(btc))
  {
    if (btc.options.all_sols && nb_sol)
    {
      printf("%zu solution%c found\n", nb_sol, nb_sol == 1 ? '\0' : 's');
    }
    else
    {
      printf("no solution found\n");
      ret = false;
    }
  }

  if (solve_options.filename != NULL)
  {
    flush(&stdout_fd);
  }

  free_btc(btc);

  return ret;
}
