#ifndef Z3_SOLVER_H
#define Z3_SOLVER_H

#include <ctype.h>
#include <z3.h>
#include <math.h>
#include <string.h>

#include "solver.h"

#define PASCAL_TRIANGLE(n) (n * (n + 1)) / 2
#define NB_DIGITS(a) floor(log10(a)) + 1
#define REP_COMBINAISON(n, k) binom_coeff(n+k-1, k)

#define DECIMAL_SYSTEM 10
#define EMPTY_DATA -1

typedef struct
{
  Z3_context ctx;
  Z3_solver slv;
  /* contains arguments in a term of the constraint */
  Z3_ast * args;
  /* contains terms of the constraints */
  Z3_ast * terms;
  /* contains variables of the grid */
  Z3_ast * vars;
  /* contains known constants of the grid and its rooms */
  Z3_ast * constants;
  /* contains constraints */
  Z3_ast * constraints;
  size_t nb_constraints;
} sys_constraints_t;

/* Z3 specific functions */
void init_solver(sys_constraints_t * sys);
void init_context(sys_constraints_t * sys);
Z3_ast mk_int_var(sys_constraints_t * sys, const char * name);
Z3_ast mk_int(sys_constraints_t * sys, int constant);
void del_solver(sys_constraints_t * sys);

/* Solution functions */
void fill_sol(completed_grid_t * grid, sys_constraints_t * sys);
void display_sol(completed_grid_t grid);

/* Output functions */
void display_sys(sys_constraints_t * sys);
void display(grid_t uncompleted_grid,
solve_opt_t solve_options,
sys_constraints_t * sys);

/* Initialization members in sys_constraints_t */
void init_vars(grid_t grid, sys_constraints_t * sys);
void init_constants(grid_t grid, sys_constraints_t * sys);

/* Constraints functions */
void sum_ls_constraints(grid_t grid, sys_constraints_t * sys);
void gtlt_ls_constraints(grid_t grid, sys_constraints_t * sys);
void dist_ls_constraints(grid_t grid, sys_constraints_t * sys);
void rooms_constraints(grid_t grid, sys_constraints_t * sys);
void found_grid_constraint(completed_grid_t grid, sys_constraints_t * sys);

#endif
