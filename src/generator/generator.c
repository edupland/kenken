#include "generator.h"

static void display_board(completed_grid_t grid)
{
  printf("\n# Solution\n");
  for (size_t i = 0; i < grid.data.dim; ++i)
  {
    printf("#");
    for (size_t j = 0; j < grid.data.dim; ++j)
    {
      for (size_t k = 2;
          k < NB_DIGITS(grid.data.dim)
          - NB_DIGITS(grid.board[i * grid.data.dim + j]) + 1;
          ++k)
      {
        printf(" ");
      }
      printf("%zd", grid.board[i * grid.data.dim + j]);
    }
    printf("\n");
  }
}

static void display_rooms(completed_grid_t grid)
{
  printf("\n# Rooms\n");
  for (size_t i = 0; i < grid.data.dim; ++i)
  {
    for (size_t j = 0; j < grid.data.dim; ++j)
    {
      for (size_t k = 1;
          k < NB_DIGITS(grid.data.nb_constraints)
          - NB_DIGITS(grid.data.rooms[i * grid.data.dim + j]);
          ++k)
      {
        printf(" ");
      }
      printf("%zd", grid.data.rooms[i * grid.data.dim + j]);
    }
    printf("\n");
  }
}

static void display_constraints(completed_grid_t grid)
{
  printf("\n# Constraints\n");
  for (size_t i = 0; i < grid.data.nb_constraints; ++i)
  {
    for (size_t k = 1; k <
        NB_DIGITS(grid.data.nb_constraints) - NB_DIGITS(i + 1); ++k)
    {
      printf(" ");
    }
    printf("%zd: %zd", i + 1, grid.data.constraints[i].target);
    switch (grid.data.constraints[i].op)
    {
      case PLUS:
        printf("+\n");
        break;

      case MINUS:
        printf("-\n");
        break;

      case TIMES:
        printf("x\n");
        break;

      case DIV:
        printf("/\n");
        break;

      case EQ:
        printf("\n");
        break;

      default:
        break;
    }
  }
}

static void display(completed_grid_t grid, gen_opt_t gen_options)
{
  int stdout_fd;
  if (gen_options.filename != NULL)
  {
    redirect_stdout(gen_options.filename, &stdout_fd);
  }

  display_board(grid);

  display_rooms(grid);

  display_constraints(grid);

  if (gen_options.filename != NULL)
  {
    flush(&stdout_fd);
  }
}

static void exceed_limits(completed_grid_t grid)
{

  printf("Error: file: generator.c, function: generate().\n"
      "You have to change one or multiple of those generating options:\n"
      " - minimum room size\n - maximum room size\n - allowed operators\n"
      "Rooms created with your parameters can exceed one or several of these"
      " limits:\n"
      " - For a room with a addition constraint, the sum of each cell "
      "is greater than the C integer limit.\n"
      " - For a room with a substraction constraint, the higher cell's value "
      "is less than the sum of others --> create a room with a negative "
      "target\n"
      " - For a room with a multiplication constraint, the product of each"
      " cell is greater than the C integer limit.\n"
      " - For a room with a division constraint, the quotient of cells is"
      " not an integer.\n");
  free(grid.board);
  free(grid.data.rooms);
  free(grid.data.constraints);
}

bool generate(gen_opt_t gen_options)
{
  struct timespec seed;
  timespec_get(&seed, TIME_UTC);
  srand(seed.tv_sec * 1000000000 + seed.tv_nsec);

  bool result = false;
  completed_grid_t grid =
  {
    .board = NULL
  };

  grid.data.dim = gen_options.size;

  grid.board = malloc(sizeof(size_t) * grid.data.dim * grid.data.dim);
  if (grid.board == NULL)
  {
    printf("Error: file : generator.c, function: generate().\n"
        "memory for latin square grids can't be allocated\n");
    exit(1);
  }

  grid.data.rooms = malloc(sizeof(size_t) * grid.data.dim * grid.data.dim);
  if (grid.data.rooms == NULL)
  {
    printf("Error: file : generator.c, function: generate().\n"
        "memory for rooms grids can't be allocated\n");
    exit(1);
  }
  bool success;
  do
  {
    result = generate_board(&grid) ? true : false;
    result = generate_rooms(&grid, gen_options) ? true : false;

    grid.data.constraints = malloc(sizeof(constraint_t)
        * grid.data.nb_constraints * CONSTRAINT_SIZE);
    if (grid.data.constraints == NULL)
    {
      printf("Error: file: generator.c function: generate().\n"
          "memory for constraints grid can't be allocated\n");
      exit(1);
    }


    result = generate_constraints(&grid, gen_options) ? true : false;

    if (!result)
    {
      exceed_limits(grid);
      exit(1);
    }

    if (gen_options.unique)
    {
      success = check_uniqueness(grid.data);

      if (!success)
      {
        free(grid.data.constraints);
      }
    }

  } while (gen_options.unique && !success);

  display(grid, gen_options);

  free(grid.board);
  free(grid.data.rooms);
  free(grid.data.constraints);

  return result;
}
