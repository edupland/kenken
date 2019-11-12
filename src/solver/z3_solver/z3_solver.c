#include "z3_solver.h"

void init_solver(sys_constraints_t * sys)
{
  sys->slv = Z3_mk_solver(sys->ctx);
  Z3_solver_inc_ref(sys->ctx, sys->slv);
}

void init_context(sys_constraints_t * sys)
{
  Z3_config cfg;
  cfg = Z3_mk_config();
  Z3_set_param_value(cfg, "model", "true");
  sys->ctx = Z3_mk_context(cfg);
  Z3_del_config(cfg);
}

void del_solver(sys_constraints_t * sys)
{
  free(sys->args);
  free(sys->terms);
  free(sys->vars);
  free(sys->constants);
  free(sys->constraints);

  Z3_solver_dec_ref(sys->ctx, sys->slv);
  Z3_del_context(sys->ctx);
  Z3_finalize_memory();
}

static Z3_ast mk_var(sys_constraints_t * sys, const char * name, Z3_sort sort)
{
  Z3_symbol symb = Z3_mk_string_symbol(sys->ctx, name);
  return Z3_mk_const(sys->ctx, symb, sort);
}

Z3_ast mk_int_var(sys_constraints_t * sys, const char * name)
{
  Z3_sort sort = Z3_mk_int_sort(sys->ctx);
  return mk_var(sys, name, sort);
}

Z3_ast mk_int(sys_constraints_t * sys, int constant)
{
  Z3_sort sort = Z3_mk_int_sort(sys->ctx);
  return Z3_mk_int(sys->ctx, constant, sort);
}

/* Parses the Z3 solution's output to fill a sys.sol */
void fill_sol(completed_grid_t * grid, sys_constraints_t * sys)
{
  bool is_index = true;
  size_t index, value;
  size_t bi_index = 0;
  size_t bv_index = 0;

  Z3_model model = Z3_solver_get_model(sys->ctx, sys->slv);
  if (model)
  {
    Z3_model_inc_ref(sys->ctx, model);
  }

  size_t len_model = strlen(Z3_model_to_string(sys->ctx, model));
  char * buffer_model = malloc(sizeof(char) * len_model);
  if (buffer_model == NULL)
  {
    printf("Error: file: z3_solvers.c, function: fill_sol() -->"
        " buffer_model can't be allocated.\n");
    exit(1);
  }

  char * buffer_index = malloc(sizeof(char)
      * (NB_DIGITS(grid->data.dim * grid->data.dim) + 1));
  if (buffer_index == NULL)
  {
    printf("Error: file: z3_solvers.c, function: fill_sol() -->"
        " buffer_index can't be allocated.\n");
    exit(1);
  }
  memset(buffer_index, '\0', NB_DIGITS(grid->data.dim * grid->data.dim) + 1);

  char * buffer_value = malloc(sizeof(char) * (NB_DIGITS(grid->data.dim) + 1));
  if (buffer_value == NULL)
  {
    printf("Error: file: z3_solvers.c, function: fill_sol() -->"
        " buffer_value can't be allocated.\n");
    exit(1);
  }
  memset(buffer_value, '\0', NB_DIGITS(grid->data.dim) + 1);

  snprintf(buffer_model, len_model, "%s", Z3_model_to_string(sys->ctx, model));

  for (size_t i = 0; i < len_model; ++i)
  {
    if (is_index)
    {
      if (isdigit(buffer_model[i]))
      {
        buffer_index[bi_index] = buffer_model[i];
        bi_index++;
      }
      else if (buffer_model[i] != 'x')
      {
        is_index = false;
        bi_index = 0;
        i += 3;
      }
    }
    else
    {
      if (isdigit(buffer_model[i]))
      {
        buffer_value[bv_index] = buffer_model[i];
        bv_index++;
      }
      else
      {
        is_index = true;
        index = strtoul(buffer_index, NULL, DECIMAL_SYSTEM);
        value = strtoul(buffer_value, NULL, DECIMAL_SYSTEM);
        memset(buffer_index, '\0', strlen(buffer_index));
        bv_index = 0;
        memset(buffer_value, '\0', strlen(buffer_value));
        grid->board[index] = value;
      }
    }
  }

  free(buffer_model);
  free(buffer_index);
  free(buffer_value);

  if (model)
  {
    Z3_model_dec_ref(sys->ctx, model);
  }
}

void display_sol(completed_grid_t grid)
{
  printf("\nSolution :\n");
  for (size_t i = 0; i < grid.data.dim; ++i)
  {
    for (size_t j = 0; j < grid.data.dim; ++j)
    {
      printf("%zd ", grid.board[i * grid.data.dim + j]);
    }
    printf("\n");
  }
}

void init_vars(grid_t grid, sys_constraints_t * sys)
{
  char * name_var = malloc(sizeof(char) * (NB_DIGITS(grid.dim * grid.dim) + 2));
  if (name_var == NULL)
  {
    printf("Error: file: z3_solvers.c, function: init_vars() -->"
        " name_var can't be allocated.\n");
    exit(1);
  }

  /* Each variable is a cell of the grid */
  for (size_t i = 0; i < grid.dim * grid.dim; ++i)
  {
    snprintf(name_var, NB_DIGITS(i) + 2,"x%zd", i);
    sys->vars[i] = mk_int_var(sys, name_var);
  }
  free(name_var);
}

void init_constants(grid_t grid, sys_constraints_t * sys)
{
  /* Each target in a room is constant */
  for (size_t i = 0; i < grid.nb_constraints; ++i)
  {
    sys->constants[i] = mk_int(sys, grid.constraints[i].target);
  }

  /* Constraints for latin square's rows and columns need a Pascal Triangle
     number as a constant  */
  sys->constants[grid.nb_constraints] =
    mk_int(sys, PASCAL_TRIANGLE(grid.dim));
}

void gtlt_ls_constraints(grid_t grid, sys_constraints_t * sys)
{
  for (size_t i = 0; i < grid.dim * grid.dim; ++i)
  {
    /* A cell in a kenken grid can't be greater than grid size */
    sys->constraints[sys->nb_constraints] =
      Z3_mk_gt(sys->ctx, sys->vars[i], sys->constants[grid.nb_constraints + 1]);
    Z3_solver_assert(sys->ctx, sys->slv, sys->constraints[sys->nb_constraints]);
    ++sys->nb_constraints;

    /* A cell in a kenken grid can't be lower than 1. */
    sys->constraints[sys->nb_constraints] =
      Z3_mk_lt(sys->ctx, sys->vars[i], sys->constants[grid.nb_constraints + 2]);
    Z3_solver_assert(sys->ctx, sys->slv, sys->constraints[sys->nb_constraints]);
    ++sys->nb_constraints;
  }
}

void dist_ls_constraints(grid_t grid, sys_constraints_t * sys)
{
  for (size_t i = 0; i < grid.dim; ++i)
  {
    for (size_t j = 0; j < grid.dim; ++j)
    {
      /* In a Row of a kenken grid, each cell is distinct */
      for (size_t k = j + 1; k < grid.dim; ++k)
      {
        sys->args[0] = sys->vars[i * grid.dim + j];
        sys->args[1] = sys->vars[i * grid.dim + k];
        sys->constraints[sys->nb_constraints] =
          Z3_mk_distinct(sys->ctx, 2, sys->args);
        Z3_solver_assert(sys->ctx, sys->slv,
            sys->constraints[sys->nb_constraints]);
        ++sys->nb_constraints;
      }
      /* In a column of a kenken grid, each cell is distinct */
      for (size_t l = i + 1; l < grid.dim; ++l)
      {
        sys->args[0] = sys->vars[i * grid.dim + j];
        sys->args[1] = sys->vars[l * grid.dim + j];
        sys->constraints[sys->nb_constraints] =
          Z3_mk_distinct(sys->ctx, 2, sys->args);
        Z3_solver_assert(sys->ctx, sys->slv,
            sys->constraints[sys->nb_constraints]);
        ++sys->nb_constraints;
      }
    }
  }
}

void rooms_constraints(grid_t grid, sys_constraints_t * sys)
{
  size_t size_constraint;
  /* A room can potentially have the same size than the grid */
  int * data_constraint =
    malloc(sizeof(int) * ((grid.dim * grid.dim + 1) * CELL_SZ));
  if (data_constraint == NULL)
  {
    printf("Error: file: z3_solvers.c, function: room_constraints() -->"
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
        /* A substraction constraint is a OR of several substractions
           because we don't know before the resolution which cell is the
           greatest */
        sys->constraints[sys->nb_constraints] =
          Z3_mk_or(sys->ctx, size_constraint, sys->terms);
        Z3_solver_assert(sys->ctx, sys->slv,
            sys->constraints[sys->nb_constraints]);
        ++sys->nb_constraints;
        break;

      case TIMES:
        sys->terms[0] = Z3_mk_mul(sys->ctx, size_constraint, sys->args);
        sys->terms[1] = sys->constants[i];
        /* Make a multiplication constraint */
        sys->constraints[sys->nb_constraints] =
          Z3_mk_eq(sys->ctx, sys->terms[0], sys->terms[1]);
        Z3_solver_assert(sys->ctx, sys->slv,
            sys->constraints[sys->nb_constraints]);
        ++sys->nb_constraints;
        break;

      case DIV:
        sys->terms[grid.dim * grid.dim] = sys->constants[i];
        /* This loop is changing the variables' place in the first term
           of the constraint. */
        for (size_t k = 0; k < size_constraint; ++k)
        {
          sys->args[grid.dim * grid.dim] = sys->args[0];
          sys->args[0] = sys->args[k];
          sys->args[k] = sys->args[grid.dim * grid.dim];
          sys->terms[k] = sys->args[0];
          /* A Z3 division can't have more than two variables. With this
             loop, we can make Z3 division with more than 2 variables. */
          for (size_t l = 1; l < size_constraint; ++l)
          {
            sys->terms[k] = Z3_mk_div(sys->ctx, sys->terms[k], sys->args[l]);
          }
          /* Make one term of the division constraint */
          sys->terms[k] =
            Z3_mk_eq(sys->ctx, sys->terms[k], sys->terms[grid.dim * grid.dim]);
        }
        /* A division constraint is a OR of several divisions because
           we don't know before the resolution which cell is the greatest */
        sys->constraints[sys->nb_constraints] =
          Z3_mk_or(sys->ctx, size_constraint, sys->terms);
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

void display_sys(sys_constraints_t * sys)
{
  printf("\nConstraints:\n");
  for (size_t i = 0; i < sys->nb_constraints; ++i)
  {
    printf("%s\n", Z3_ast_to_string(sys->ctx, sys->constraints[i]));
  }
}

void found_grid_constraint(completed_grid_t grid, sys_constraints_t * sys)
{
  /* We don't know how many solution have the grid so we have to use realloc()
     to add a new constraint in the solver each time a new solution is found */
  sys->constraints = realloc(sys->constraints,
      sizeof(Z3_ast) * (sys->nb_constraints + 1));
  if (sys->constraints == NULL)
  {
    printf("Error: file: z3_solvers.c, function: found_grid_constraint() -->"
        " sys.constraints can't be reallocated.\n");
    exit(1);
  }

  /* The found solution will be an AND of EQUAL constraints where each EQUAL
     is a solution for one grid's cell. */
  for (size_t i = 0; i < grid.data.dim * grid.data.dim; ++i)
  {
    Z3_ast sol = mk_int(sys, grid.board[i]);
    sys->args[i] = Z3_mk_eq(sys->ctx, sys->vars[i], sol);
  }
  sys->terms[0] = Z3_mk_and(sys->ctx, grid.data.dim * grid.data.dim, sys->args);
  /* This solution is now considered as a FALSE constraint, the solver can
     search an other solution */
  sys->terms[1] = Z3_mk_false(sys->ctx);

  sys->constraints[sys->nb_constraints] =
    Z3_mk_eq(sys->ctx, sys->terms[0], sys->terms[1]);
  Z3_solver_assert(sys->ctx, sys->slv, sys->constraints[sys->nb_constraints]);
  ++sys->nb_constraints;
}

void display(grid_t uncompleted_grid,
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
    while (Z3_solver_check(sys->ctx, sys->slv) == Z3_L_TRUE)
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
    if (Z3_solver_check(sys->ctx, sys->slv) == Z3_L_TRUE)
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
