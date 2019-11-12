#include "z3_solver.h"

bool inequalities_system(grid_t grid, solve_opt_t solve_options)
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
    printf("Error: file: inequalities_system.c,"
        " function: inequalities_system().\nsys.args can't be allocated.\n");
    exit(1);
  }

  /* Can contains a grid solution + 1 tmp variable */
  sys.terms = malloc(sizeof(Z3_ast) * (grid.dim * grid.dim + 1));
  if (sys.terms == NULL)
  {
    printf("Error: file: inequalities_system.c,"
        " function: inequalities_system().\nsys.terms can't be allocated.\n");
    exit(1);
  }

  /* Can contains each cell of the grid */
  sys.vars = malloc(sizeof(Z3_ast) * grid.dim * grid.dim);
  if (sys.vars == NULL)
  {
    printf("Error: file: inequalities_system.c,"
        " function: inequalities_system().\nsys.vars can't be allocated.\n");
    exit(1);
  }

  /* Can contains rooms' constants + latin square sum constant + cell's less
     than constant (=0) + cell's greater than constant */
  sys.constants = malloc(sizeof(Z3_ast) * (grid.nb_constraints + 3));
  if (sys.constants == NULL)
  {
    printf("Error: file: inequalities_system.c,"
        " function: inequalities_system().\n"
        "sys.constants can't be allocated.\n");
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
    printf("Error: file: inequalities_system.c,"
        " function: inequalities_system().\n"
        "sys.constraints can't be allocated.\n");
    exit(1);
  }

  init_vars(grid, &sys);

  init_constants(grid, &sys);
  /* greater than constant */
  sys.constants[grid.nb_constraints + 1] = mk_int(&sys, 0);
  /* less than constant */
  sys.constants[grid.nb_constraints + 2] = mk_int(&sys, grid.dim + 1);

  gtlt_ls_constraints(grid, &sys);

  dist_ls_constraints(grid, &sys);

  rooms_constraints(grid, &sys);

  display(grid, solve_options, &sys);

  return result;
}
