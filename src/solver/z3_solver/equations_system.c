#include "z3_solver.h"

void sum_ls_constraints(grid_t grid, sys_constraints_t * sys)
{
  /* A row in a kenken grid can be written like a sum of its cells */
  for (size_t i = 0; i < grid.dim; ++i)
  {
    /* Get cells variables */
    for (size_t j = 0; j < grid.dim; ++j)
    {
      sys->args[j] = sys->vars[i * grid.dim + j];
    }

    /* Addition between cells variables */
    sys->terms[0] = Z3_mk_add(sys->ctx, grid.dim, sys->args);
    /* Get the Pascal Triangle number which is the other term of this
       constraint */
    sys->terms[1] = sys->constants[grid.nb_constraints];
    /* Make the constraint */
    sys->constraints[sys->nb_constraints] =
      Z3_mk_eq(sys->ctx, sys->terms[0], sys->terms[1]);
    Z3_solver_assert(sys->ctx, sys->slv, sys->constraints[sys->nb_constraints]);
    ++sys->nb_constraints;
  }

  /* Same thing for columns in a kenken grid */
  for (size_t i = 0; i < grid.dim; ++i)
  {
    for (size_t j = 0; j < grid.dim; ++j)
    {
      sys->args[j] = sys->vars[j * grid.dim + i];
    }

    sys->terms[0] = Z3_mk_add(sys->ctx, grid.dim, sys->args);
    sys->terms[1] = sys->constants[grid.nb_constraints];
    sys->constraints[sys->nb_constraints] =
      Z3_mk_eq(sys->ctx, sys->terms[0], sys->terms[1]);
    Z3_solver_assert(sys->ctx, sys->slv, sys->constraints[sys->nb_constraints]);
    ++sys->nb_constraints;
  }
}

static void product_ls_constraints(grid_t grid, sys_constraints_t * sys)
{
  /* A row in a kenken grid, coupled to a sum constraint, can be written like
     a product of its cells. */
  for (size_t i = 0; i < grid.dim; ++i)
  {
    /* Collects cells variables to make a product */
    for (size_t j = 0; j < grid.dim; ++j)
    {
      sys->args[j] = sys->vars[i * grid.dim + j];
    }

    sys->terms[0] = Z3_mk_mul(sys->ctx, grid.dim, sys->args);

    /* If the grid size is larger than 20, the result of this constraint
       pass throught the limit of a C integer. The solution is to give a
       product to Z3 to not have to manage this problem */
    for (size_t j = 0; j < grid.dim; ++j)
    {
      sys->args[j] = sys->constants[j + grid.nb_constraints + 1];
    }

    /* Factorial of the grid size is the second term of the constraint */
    sys->terms[1] = Z3_mk_mul(sys->ctx, grid.dim, sys->args);

    sys->constraints[sys->nb_constraints] =
      Z3_mk_eq(sys->ctx, sys->terms[0], sys->terms[1]);
    Z3_solver_assert(sys->ctx, sys->slv, sys->constraints[sys->nb_constraints]);
    ++sys->nb_constraints;
  }

  /* Same thing for columns in a kenken grid */
  for (size_t i = 0; i < grid.dim; ++i)
  {
    for (size_t j = 0; j < grid.dim; ++j)
    {
      sys->args[j] = sys->vars[j * grid.dim + i];
    }

    sys->terms[0] = Z3_mk_mul(sys->ctx, grid.dim, sys->args);

    for (size_t j = 0; j < grid.dim; ++j)
    {
      sys->args[j] = sys->constants[j + grid.nb_constraints + 1];
    }

    sys->terms[1] = Z3_mk_mul(sys->ctx, grid.dim, sys->args);
    sys->constraints[sys->nb_constraints] =
      Z3_mk_eq(sys->ctx, sys->terms[0], sys->terms[1]);
    Z3_solver_assert(sys->ctx, sys->slv, sys->constraints[sys->nb_constraints]);
    ++sys->nb_constraints;
  }
}

bool equations_system(grid_t grid, solve_opt_t solve_options)
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
    printf("Error: file: equations_system.c, function: equations_system() -->"
        " sys.args can't be allocated.\n");
    exit(1);
  }

  /* Can contains a grid solution + 1 tmp variable */
  sys.terms = malloc(sizeof(Z3_ast) * (grid.dim * grid.dim + 1));
  if (sys.terms == NULL)
  {
    printf("Error: file: equations_system.c, function: equations_system() -->"
        " sys.terms can't be allocated.\n");
    exit(1);
  }

  /* Can contains each cell of the grid */
  sys.vars = malloc(sizeof(Z3_ast) * grid.dim * grid.dim);
  if (sys.vars == NULL)
  {
    printf("Error: file: equations_system.c, function: equations_system() -->"
        " sys.vars can't be allocated.\n");
    exit(1);
  }

  /* Can contains each rooms' result of the grid + each constant used in a
     factorial number + latin square sum constant */
  sys.constants = malloc(sizeof(Z3_ast) * (grid.nb_constraints + grid.dim + 1));
  if (sys.constants == NULL)
  {
    printf("Error: file: equations_system.c, function: equations_system() -->"
        " sys.constants can't be allocated.\n");
    exit(1);
  }

  /* Can contains latin square sum constaints + latin square product
     constraints + romms constraints */
  sys.constraints =
    malloc(sizeof(Z3_ast) * (grid.dim * 4 + grid.nb_constraints));
  if (sys.constraints == NULL)
  {
    printf("Error: file: equations_system.c, function: equations_system() -->"
        " sys.constraints can't be allocated.\n");
    exit(1);
  }

  init_vars(grid, &sys);

  init_constants(grid, &sys);
  /* factorial constants used for factorial number */
  for (size_t i = 1; i < grid.dim + 1; ++i)
  {
    sys.constants[grid.nb_constraints + i] = mk_int(&sys, i);
  }

  sum_ls_constraints(grid, &sys);

  product_ls_constraints(grid, &sys);

  rooms_constraints(grid, &sys);

  display(grid, solve_options, &sys);

  return result;
}
