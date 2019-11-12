#include "z3_solver.h"

/* Returns binomial coefficient */
static size_t binom_coeff(size_t n, size_t k)
{
  return ((n == k || k == 0) ?
      1: (binom_coeff(n - 1, k - 1) + binom_coeff(n - 1, k)));
}

/* Fills combinaison buffer with the n-th combinaison with repetition */
static void get_rep_comb(size_t * combinaison,
    size_t size,
    size_t n,
    size_t base)
{
  size_t i;
  size_t j;
  bool restart = false;

  for (i = 0; i < size; ++i)
  {
    combinaison[i] = 1;
  }

  i = 0;
  j = 0;

  while (j < n)
  {
    if (combinaison[i] < base)
    {
      ++combinaison[i];
      if (restart)
      {
        for (size_t k = 0; k < i; ++k)
        {
          combinaison[k] = combinaison[i];
        }
        restart = false;
        i = 0;
      }
      ++j;
    }
    else
    {
      ++i;
      restart = true;
    }
  }
}

/* Finds possible values for cells in the current constraint to linearize
   the multiplication constraint into a addition constraint.
   Returns the sum of this possible values if the values' product is equal
   to the target constraint. Otherwise, it returns 0. */
static size_t product_to_sum(size_t i,
    grid_t grid,
    size_t nb_cells,
    size_t constraint_id)
{
  size_t sum = 0;
  size_t product = 1;

  size_t * combinaison = malloc(sizeof(size_t) * nb_cells);
  if (combinaison == NULL)
  {
    printf("Error: file: linear_inequalities_system.c,"
        " function: product_to_sum().\ncombinaison can't be allocated.\n");
    exit(1);
  }

  get_rep_comb(combinaison, nb_cells, i, grid.dim);

  for (size_t j = 0; j < nb_cells; ++j)
  {
    sum += combinaison[j];
    product *= combinaison[j];
  }

  if (grid.constraints[constraint_id].target != product)
  {
    sum = 0;
  }

  free(combinaison);
  return sum;
}

/* Finds possible values for cells in the current constraint to linearize
   the division constraint into a addition constraint.
   Returns the sum of this possible values if the values' quotient is equal
   to the target constraint. Otherwise, it returns 0. */
static size_t quotient_to_sum(size_t i,
    grid_t grid,
    size_t nb_cells,
    size_t constraint_id)
{
  size_t * combinaison = malloc(sizeof(size_t) * nb_cells);
  if (combinaison == NULL)
  {
    printf("Error: file: linear_inequalities_system.c,"
        " function: quotient_to_sum().\ncombinaison can't be allocated.\n");
    exit(1);
  }

  get_rep_comb(combinaison, nb_cells, i, grid.dim);

  size_t sum = combinaison[0];
  size_t quotient = combinaison[0];
  bool is_int_div = true;

  for (size_t j = 1; (j < nb_cells) && is_int_div; ++j)
  {
    sum += combinaison[j];
    is_int_div = (quotient % combinaison[j] == 0);
    quotient /= combinaison[j];
  }

  if ((!is_int_div) || (grid.constraints[constraint_id].target != quotient))
  {
    sum = 0;
  }

  free(combinaison);
  return sum;
}

static void linear_rooms_constraints(grid_t grid, sys_constraints_t * sys)
{
  size_t size_constraint;
  size_t sum;
  size_t nb_comb;
  size_t cpt;
  /* A room can potentially have the same size than the grid */
  int * data_constraint =
    malloc(sizeof(int) * ((grid.dim * grid.dim + 1) * CELL_SZ));
  if (data_constraint == NULL)
  {
    printf("Error: file: linear_inequalities_system.c,"
        " function: linear_room_constraints().\n"
        " data_constraint can't be allocated.\n");
    exit(1);
  }

  for (size_t i = 0; i < grid.nb_constraints; ++i)
  {

    for (size_t j = 0; j < ((grid.dim * grid.dim + 1) * CELL_SZ); ++j)
    {
      data_constraint[j] = EMPTY_DATA;
    }

    size_constraint = get_constraint_pos(grid, &data_constraint, i + 1);
    /* fills the arguments of the first term of the constraint with cells
       inside */
    for (size_t j = 0; j < size_constraint; ++j)
    {
      sys->args[j] =
        sys->vars[data_constraint[CELL_SZ * j + ROW_INDEX]
        * grid.dim + data_constraint[CELL_SZ * j + COL_INDEX]];
    }
    /* Depending of the operator, the constraint will be written differently */
    switch ((grid.constraints[i]).op)
    {
      case PLUS:
        sys->terms[0] = Z3_mk_add(sys->ctx, size_constraint, sys->args);
        sys->terms[1] = sys->constants[i];
        /* Make an addition constraint */
        sys->constraints[sys->nb_constraints] =
          Z3_mk_eq(sys->ctx, sys->terms[0], sys->terms[1]);
        Z3_solver_assert(sys->ctx, sys->slv,
            sys->constraints[sys->nb_constraints]);
        ++sys->nb_constraints;
        break;

      case MINUS:
        sys->terms[grid.dim * grid.dim] = sys->constants[i];
        /* This loop is changing the variables' place in the first term
           of the constraint. */
        for (size_t k = 0; k < size_constraint; ++k)
        {
          /* the (grid.dim * grid.dim)th variable in sys.arg is a tmp
             variable */
          sys->args[grid.dim * grid.dim] = sys->args[0];
          sys->args[0] = sys->args[k];
          sys->args[k] = sys->args[grid.dim * grid.dim];
          /* Make one term of the substraction constraint */
          sys->terms[k] = Z3_mk_sub(sys->ctx, size_constraint, sys->args);
          sys->terms[k] =
            Z3_mk_eq(sys->ctx, sys->terms[k], sys->terms[grid.dim * grid.dim]);
        }
        /* A substraction constraint is an OR of several substractions
           because we don't know before the resolution which cell is the
           greatest */
        sys->constraints[sys->nb_constraints] =
          Z3_mk_or(sys->ctx, size_constraint, sys->terms);
        Z3_solver_assert(sys->ctx, sys->slv,
            sys->constraints[sys->nb_constraints]);
        ++sys->nb_constraints;
        break;

      case TIMES:

        /* Linearizes a multiplication constraint to an addition constraint */
        sys->args[0] = Z3_mk_add(sys->ctx, size_constraint, sys->args);
        nb_comb = REP_COMBINAISON(grid.dim, size_constraint);
        /* This variable counts how many terms will be in the constraints */
        cpt = 0;
        /* This variable forbids terms' repetition in a same constraint for
           a product target */
        size_t min = nb_comb;
        /* This loop builds each terms of the new OR constraint */
        for (size_t k = 0; k < nb_comb; ++k)
        {
          sum = product_to_sum(k, grid, size_constraint, i);
          if ((sum < min) && (sum > 0))
          {
            min = sum;
            sys->args[1] = mk_int(sys, sum);
            sys->terms[cpt] = Z3_mk_eq(sys->ctx, sys->args[0], sys->args[1]);
            ++cpt;
          }
        }
        if (cpt > 1)
        {
          sys->constraints[sys->nb_constraints] =
            Z3_mk_or(sys->ctx, cpt, sys->terms);
        }
        /* If there are a lonely term in the constraint the algorithm doesn't
           apply OR binary operation between terms. */
        else
        {
          sys->constraints[sys->nb_constraints] = sys->terms[0];
        }
        Z3_solver_assert(sys->ctx, sys->slv,
            sys->constraints[sys->nb_constraints]);
        ++sys->nb_constraints;
        break;

      case DIV:

        /* Linearizes a division constraint to an addition constraint */
        sys->args[0] = Z3_mk_add(sys->ctx, size_constraint, sys->args);
        nb_comb = REP_COMBINAISON(grid.dim, size_constraint);
        /* This variable counts how many terms will be in the constraints */
        cpt = 0;
        /* This loop builds each terms of the new OR constraint */
        for (size_t k = 0; k < nb_comb; ++k)
        {
          sum = quotient_to_sum(k, grid, size_constraint, i);
          if (sum > 0)
          {
            sys->args[1] = mk_int(sys, sum);
            sys->terms[cpt] = Z3_mk_eq(sys->ctx, sys->args[0], sys->args[1]);
            ++cpt;
          }
        }
        if (cpt > 1)
        {
          sys->constraints[sys->nb_constraints] =
            Z3_mk_or(sys->ctx, cpt, sys->terms);
        }
        /* If there are a lonely term in the constraint the algorithm doesn't
           apply OR binary operation between terms. */
        else
        {
          sys->constraints[sys->nb_constraints] = sys->terms[0];
        }
        Z3_solver_assert(sys->ctx, sys->slv,
            sys->constraints[sys->nb_constraints]);
        ++sys->nb_constraints;
        break;

      case EQ:
        /* Make a constraint which contains an unique cell */
        sys->constraints[sys->nb_constraints] =
          Z3_mk_eq(sys->ctx, sys->args[0], sys->constants[i]);
        Z3_solver_assert(sys->ctx, sys->slv,
            sys->constraints[sys->nb_constraints]);
        ++sys->nb_constraints;
        break;
      default:
        break;
    }
  }

  free(data_constraint);
}

/* Checks if the found grid is a solution */
static bool is_solution(grid_t uncompleted_grid,
    sys_constraints_t * sys,
    solve_opt_t solve_options)
{
  bool is_solution = false;

  completed_grid_t grid =
  {
    .board = NULL,
    .data = uncompleted_grid
  };

  grid.board =
    malloc(sizeof(size_t) * grid.data.dim * grid.data.dim);
  if (grid.board == NULL)
  {
    printf("Error: linear_inequalities_system.c,"
        " function: check_solution().\n"
        "memory for grid.board can't be allocated\n");
    exit(1);
  }

  int * data_constraint = malloc(sizeof(int)
      * (grid.data.dim * grid.data.dim + 1));
  if (data_constraint == NULL)
  {
    printf("Error: linear_inequalities_system.c,"
        " function: check_solution().\n"
        "memory for data constraint can't be allocated\n");
    exit(1);
  }

  while (!is_solution && (Z3_solver_check(sys->ctx, sys->slv) == Z3_L_TRUE))
  {
    fill_sol(&grid, sys);

    is_solution = true;

    for (size_t i = 1; (i <= grid.data.nb_constraints) && is_solution; ++i)
    {
      memset(data_constraint, LAST_ELMT,
          (grid.data.dim * grid.data.dim + 1) * sizeof(int));

      get_constraint_val(grid, &data_constraint, i);

      switch (grid.data.constraints[i - 1].op)
      {
        case DIV:
          is_solution &= (div_constr(data_constraint) ==
              (int) grid.data.constraints[i - 1].target);
          break;

        case TIMES:
          is_solution &= (prod_constr(data_constraint) ==
              (int) grid.data.constraints[i - 1].target);
          break;

        case MINUS:
          is_solution &= (diff_constr(data_constraint) ==
              (int) grid.data.constraints[i - 1].target);
          break;

        case PLUS:
          is_solution &= (sum_constr(data_constraint) ==
              grid.data.constraints[i - 1].target);
          break;

        case EQ:
          is_solution &= (data_constraint[0] ==
              (int) grid.data.constraints[i - 1].target);
          break;

        default:
          break;
      }
    }

    if (!is_solution)
    {
      found_grid_constraint(grid, sys);
      if (solve_options.verbose_mod)
      {
        printf("\nnew constraint added :\n%s\n\n", Z3_ast_to_string(sys->ctx,
              sys->constraints[sys->nb_constraints - 1]));
      }
    }
  }

  free(grid.board);
  free(data_constraint);

  return is_solution;
}

/* To linearize multiplication and division constraints, the algorithm
   turns it into a weaker constraint. The algorithm have to check if
   the found grid follows grid's constraints */
static void check_and_display(grid_t uncompleted_grid,
    solve_opt_t solve_options,
    sys_constraints_t * sys)
{
  int stdout_fd;

  completed_grid_t grid =
  {
    .board = NULL,
    .data = uncompleted_grid
  };

  grid.board = malloc(sizeof(size_t) * grid.data.dim * grid.data.dim);
  if (grid.board == NULL)
  {
    printf("Error: z3_solvers.c, function: display().\n"
        "memory for grid.board can't be allocated\n");
    exit(1);
  }

  if (solve_options.filename != NULL)
  {
    redirect_stdout(solve_options.filename, &stdout_fd);
  }

  if (solve_options.verbose_mod)
  {
    display_sys(sys);
  }

  if (solve_options.all_sols)
  {
    while (is_solution(uncompleted_grid, sys, solve_options))
    {
      fill_sol(&grid, sys);
      display_sol(grid);
      found_grid_constraint(grid, sys);

      if (solve_options.verbose_mod)
      {
        printf("\nnew constraint added :\n%s\n\n", Z3_ast_to_string(sys->ctx,
              sys->constraints[sys->nb_constraints - 1]));
      }
    }
  }
  else
  {
    if (is_solution(uncompleted_grid, sys, solve_options))
    {
      fill_sol(&grid, sys);
      display_sol(grid);
    }
  }

  if (solve_options.filename != NULL)
  {
    flush(&stdout_fd);
  }

  free(grid.board);
  del_solver(sys);
}

bool linear_inequalities_system(grid_t grid, solve_opt_t solve_options)
{
  bool result = true;

  sys_constraints_t sys =
  {
    .args = NULL,
    .terms = NULL,
    .vars = NULL,
    .constants = NULL,
    .constraints = NULL,
    .nb_constraints = 0
  };

  init_context(&sys);
  init_solver(&sys);

  /* Can contains each cell of the grid + 1 tmp variable */
  sys.args = malloc(sizeof(Z3_ast) * (grid.dim * grid.dim + 1));
  if (sys.args == NULL)
  {
    printf("Error: file: linear_inequalities_system.c,"
        " function: linear_inequalities_system().\n"
        " sys.args can't be allocated.\n");
    exit(1);
  }

  /* Can contains a grid solution + 1 tmp variable */
  sys.terms = malloc(sizeof(Z3_ast) * (grid.dim * grid.dim + 1));
  if (sys.terms == NULL)
  {
    printf("Error: file: linear_inequalities_system.c,"
        " function: linear_inequalities_system().\n"
        " sys.terms can't be allocated.\n");
    exit(1);
  }

  /* Can contains each cell of the grid */
  sys.vars = malloc(sizeof(Z3_ast) * grid.dim * grid.dim);
  if (sys.vars == NULL)
  {
    printf("Error: file: linear_inequalities_system.c,"
        " function: linear_inequalities_system().\n"
        " sys.vars can't be allocated.\n");
    exit(1);
  }

  /* Can contains rooms' constants + latin square sum constant + cell's less
     than constant (=0) + cell's greater than constant + constants for each
     possibility in a cell */
  sys.constants = malloc(sizeof(Z3_ast) * (grid.nb_constraints + grid.dim + 3));
  if (sys.constants == NULL)
  {
    printf("Error: file: linear_inequalities_system.c,"
        " function: linear_inequalities_system().\n"
        " sys.constants can't be allocated.\n");
    exit(1);
  }

  /* Can contains room's constraints + latin square sum constraints +
     distinct cells' constraints + greater than and less than cells'
     constraints */
  sys.constraints =
    malloc(sizeof(Z3_ast) * (grid.dim * grid.dim * (grid.dim - 1)
          + grid.dim * 2 + grid.nb_constraints + 2 * grid.dim * grid.dim));
  if (sys.constraints == NULL)
  {
    printf("Error: file: linear_inequalities_system.c,"
        " function: linear_inequalities_system().\n"
        "sys.constraints can't be allocated.\n");
    exit(1);
  }

  init_vars(grid, &sys);

  init_constants(grid, &sys);
  /* greater than constant */
  sys.constants[grid.nb_constraints + 1] = mk_int(&sys, 0);
  /* less than constant */
  sys.constants[grid.nb_constraints + 2] = mk_int(&sys, grid.dim + 1);
  /* constants used for rooms' constraints */
  for (size_t i = 1; i < grid.dim + 1; ++i)
  {
    sys.constants[grid.nb_constraints + 2 + i] = mk_int(&sys, i);
  }

  gtlt_ls_constraints(grid, &sys);

  dist_ls_constraints(grid, &sys);

  linear_rooms_constraints(grid, &sys);

  check_and_display(grid, solve_options, &sys);

  return result;
}
