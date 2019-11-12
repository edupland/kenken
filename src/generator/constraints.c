#include "generator.h"

static bool test_addition(completed_grid_t * grid,
    int * data_constraint,
    size_t index)
{
  bool result = false;
  size_t sum = sum_constr(data_constraint);

  if (sum < 1)
  {
    result = true;
  }
  else
  {
    grid->data.constraints[index].target = sum;
    grid->data.constraints[index].op = PLUS;
  }

  return result;
}

static bool test_multiplication(completed_grid_t * grid,
    int * data_constraint,
    size_t index)
{
  bool result = false;
  int product = prod_constr(data_constraint);

  if (product < 1)
  {
    result = true;
  }
  else
  {
    grid->data.constraints[index].target = product;
    grid->data.constraints[index].op = TIMES;
  }

  return result;
}

static bool test_substraction(completed_grid_t * grid,
    int * data_constraint,
    size_t index)
{
  bool result = false;
  int difference = diff_constr(data_constraint);

  if (difference < 0)
  {
    result = true;
  }
  else
  {
    grid->data.constraints[index].target = difference;
    grid->data.constraints[index].op = MINUS;
  }

  return result;
}

static bool test_division(completed_grid_t * grid,
    int * data_constraint,
    size_t index)
{
  bool result = false;
  double quotient = div_constr(data_constraint);

  if (rint(quotient) != quotient)
  {
    result = true;
  }
  else
  {
    grid->data.constraints[index].target = (size_t)rint(quotient);
    grid->data.constraints[index].op = DIV;
  }

  return result;
}

static bool add_constraint(completed_grid_t * grid,
    int * data_constraint,
    size_t index,
    gen_opt_t gen_options)
{
  bool is_possible = true;
  size_t rd_operator;
  bool restart[NB_OPERATORS];

  for (size_t i = 0; i < NB_OPERATORS; ++i)
  {
    restart[i] = !gen_options.operators[i];
  }

  do
  {
    rd_operator = rand() % NB_OPERATORS;
    if (!restart[rd_operator])
    {
      switch (rd_operator)
      {
        case DIV:
          restart[DIV] = test_division(grid, data_constraint, index);
          break;

        case MINUS:
          restart[MINUS] = test_substraction(grid, data_constraint, index);
          break;

        case PLUS:
          restart[PLUS] = test_addition(grid, data_constraint, index);
          break;

        case TIMES:
          restart[TIMES] = test_multiplication(grid, data_constraint, index);
          break;

        default:
          break;
      }
    }
    for (size_t i = 0; i < NB_OPERATORS; ++i)
    {
      is_possible &= restart[i];
    }
    is_possible = !is_possible;
  } while (is_possible && restart[rd_operator]);

  return is_possible;
}

bool generate_constraints(completed_grid_t * grid, gen_opt_t gen_options)
{
  bool result = true;
  size_t nb_cells_in_room = 0;
  int * data_constraint = NULL;

  data_constraint = malloc(sizeof(int) * (gen_options.max_room_sz + 1));
  if (data_constraint == NULL)
  {
    printf("Error: file constraints.c function: generate_constraints().\n"
        "memory for data constraint can't be allocated\n");
    exit(1);
  }

  for (size_t i = 0;
      result && (i < grid->data.nb_constraints);
      ++i)
  {
    memset(data_constraint, LAST_ELMT,
        (gen_options.max_room_sz + 1) * sizeof(int));

    nb_cells_in_room =
      get_constraint_val(*grid, &data_constraint, i + 1);

    if (nb_cells_in_room == 1)
    {
      grid->data.constraints[i].target = data_constraint[0];
      grid->data.constraints[i].op = EQ;
    }
    else
    {
      result = add_constraint(grid, data_constraint, i, gen_options);
    }
  }

  free(data_constraint);

  return result;
}
