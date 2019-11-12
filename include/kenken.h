#ifndef KENKEN_H
#define KENKEN_H

#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "generator.h"
#include "parser.h"
#include "solver.h"

#define BINARY_SYSTEM 2
#define DECIMAL_SYSTEM 10
#define DEFAULT_SIZE 6
#define DEFAULT_MIN 1
#define DEFAULT_MAX 0
#define RESTART 1

typedef enum
{
  VERSION = 0,
  OUTPUT = 1,
  ALL = 2,
  UNIQUE = 2,
  VERBOSE = 3,
  MIN = 3,
  SOLVER = 4,
  MAX = 4,
  OPERATORS = 5
} index_flag;

#endif
