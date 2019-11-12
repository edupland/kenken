#include "kenken.h"

static bool solve(grid_t grid, solve_opt_t solve_options)
{
  bool result = false;

  switch (solve_options.solver)
  {
    case BACK_TRACKING :
      result = back_tracking(grid, solve_options);
      break;

    case EQ_SYSTEM :
      result = equations_system(grid, solve_options);
      break;

    case INEQ_SYSTEM :
      result = inequalities_system(grid, solve_options);
      break;

    case LINEAR_INEQ_SYSTEM :
      result = linear_inequalities_system(grid, solve_options);
      break;

    case LOGIC :
      result = logic(grid, solve_options);
      break;
  }

  return result;
}

static void display_help()
{
  printf("Usage: kenken [-a|-o FILE|-v|-V|-h|-s[SOLVER]] FILE...\n"
      "       kenken -g[SIZE] [-u|-o FILE|-V|-h|-m[MIN]|-M[MAX]|-O[OP]]\n"
      "Solve or generate kenken grids of variable size\n\n"
      " -g[N],--generate[=N]   generate a grid of size NxN (default=6x6)\n"
      " -a,--all               search for all possible solutions\n"
      " -u,--unique            generate a grid with unique solution\n"
      " -o FILE,--output FILE  write result to FILE\n"
      " -v,--verbose           verbose output\n"
      " -V,--version           display version and exit\n"
      " -h,--help              display this help\n"
      " -m[N],--minimum[=N]    generation option to set the minimum number\n"
      "                        of cells per room. The algorithm always\n"
      "                        tries as mush as possible to fill the grid\n"
      "                        with rooms which follow this setup.\n"
      "                        (default:2)\n"
      " -M[N],--maximum[=N]    generation option to set the maximum number\n"
      "                        of cells per room (default:2)\n"
      " -s[S],--solver[=S]     use the specified algorithm to solve\n"
      "                        the grid:\n"
      "                        * 'logic' or 'l' (default),\n"
      "                        * 'back_tracking' or 'bt',\n"
      "                        * 'equations_system' or 'es',\n"
      "                        * 'inequalities_system' or 'is'\n"
      "                        * 'linear_inequalities_system' or 'lis'\n"
      " -O[S],--operators[=S]  generation option to set the allowed\n"
      "                        operators. S is a string which constrains\n"
      "                        the allowed operators. (default:+s*/)\n"
      "                        Possible operators: +s*/\n"
      "                        Error if only / operator is allowed\n"
      );
}

static void check_gen_opt(gen_opt_t * gen_options)
{
  if (gen_options->max_room_sz == DEFAULT_MAX)
  {
    gen_options->max_room_sz = gen_options->size * gen_options->size;
  }
  else if (gen_options->max_room_sz > gen_options->size * gen_options->size)
  {
    printf("Error : grid size less than maximum room size.\n");
    exit(1);
  }

  if (gen_options->min_room_sz > gen_options->max_room_sz)
  {
    printf("Error : minimum room size greater than maximum room size.\n");
    exit(1);
  }
}

static void solving_options(int option,
    bool * marker,
    solve_opt_t * solve_options)
{
  switch (option)
  {
    case 'a':
      if (!marker[ALL])
      {
        marker[ALL] = true;
        solve_options->all_sols = true;
      }
      break;

    case 'o':
      if (!marker[OUTPUT])
      {
        marker[OUTPUT] = true;
        solve_options->filename = malloc(sizeof(char) * MAX_SZ_NAME);
        if (solve_options->filename == NULL)
        {
          printf("Error: file: kenken.c function: solving_options().\n"
              "solve_options.filename can't be allocated");
          exit(1);
        }

        snprintf(solve_options->filename, MAX_SZ_NAME, "output/%s", optarg);
        printf("output file : %s\n", optarg);
      }
      else
      {
        printf("Error: Another output file is already used to treat"
            " your request. The file %s won't be used\n", optarg);
      }
      break;

    case 's':
      if (!marker[SOLVER])
      {
        marker[SOLVER] = true;
        if ((strcmp(optarg, "es") == 0)
            || (strcmp(optarg, "equations_system") == 0))
        {
          solve_options->solver = EQ_SYSTEM;
        }
        else if ((strcmp(optarg, "is") == 0)
            || (strcmp(optarg, "inequalities_system") == 0))
        {
          solve_options->solver = INEQ_SYSTEM;
        }
        else if ((strcmp(optarg, "lis") == 0)
            || (strcmp(optarg, "linear_inequalities_system") == 0))
        {
          solve_options->solver = LINEAR_INEQ_SYSTEM;
        }
        else if ((strcmp(optarg, "bt") == 0)
            || (strcmp(optarg, "back_tracking") == 0))
        {
          solve_options->solver = BACK_TRACKING;
        }
        else if ((strcmp(optarg, "l") == 0) || (strcmp(optarg, "logic") == 0))
        {
          solve_options->solver = LOGIC;
        }
        else
        {
          printf("Error: %s is an unknown algorithm.\n", optarg);
          exit(1);
        }
      }
      break;

    case 'v':
      if (!marker[VERBOSE])
      {
        marker[VERBOSE] = true;
        solve_options->verbose_mod = true;
      }
      break;

    case 'V':
      if (!marker[VERSION])
      {
        marker[VERSION] = true;
        printf("Version : 2.0\n");
      }
      break;

    case -1:
    case 0:
    default:
      break;
  }
}

static void generating_options(int option,
    bool * marker,
    gen_opt_t * gen_options)
{
  switch(option)
  {
    case 'm':
      if (!marker[MIN])
      {
        marker[MIN] = true;
        gen_options->min_room_sz = strtoul(optarg, NULL, DECIMAL_SYSTEM);
        if (gen_options->min_room_sz > gen_options->max_room_sz)
        {
          gen_options->max_room_sz = gen_options->min_room_sz;
        }
      }
      break;

    case 'M':
      if (!marker[MAX])
      {
        gen_options->max_room_sz = strtoul(optarg, NULL, DECIMAL_SYSTEM);
        marker[MAX] = true;
      }
      break;

    case 'o':
      if (!marker[OUTPUT])
      {
        marker[OUTPUT] = true;
        gen_options->filename = malloc(sizeof(char) * MAX_SZ_NAME);
        if (gen_options->filename == NULL)
        {
          printf("Error: file: kenken.c function: generating_options().\n"
              "gen_options.filename can't be allocated");
          exit(1);
        }

        snprintf(gen_options->filename, MAX_SZ_NAME, "output/%s", optarg);
        printf("output file : %s\n", optarg);
      }
      else
      {
        printf("Error: Another output file is already used to treat"
            " your request. The file %s won't be used\n", optarg);
      }
      break;

    case 'O':
      if (!marker[OPERATORS])
      {
        marker[OPERATORS] = true;
        for (size_t i = 0; i < NB_OPERATORS; ++i)
        {
          gen_options->operators[i] = false;
        }
        for (size_t i = 0; i < strlen(optarg); ++i)
        {
          switch (optarg[i])
          {
            case '+':
              gen_options->operators[PLUS] = true;
              break;
            case 's':
              gen_options->operators[MINUS] = true;
              break;
            case '*':
              gen_options->operators[TIMES] = true;
              break;
            case '/':
              gen_options->operators[DIV] = true;
              break;
            default:
              printf("Error: file: kenken.c function: generating_options().\n"
                  "%c is an unknown operator", optarg[i]);
              exit(1);
              break;
          }
        }
      }
      break;

    case 'u':
      if (!marker[UNIQUE])
      {
        marker[UNIQUE] = true;
        gen_options->unique = true;
      }
      break;

    case 'V':
      if (!marker[VERSION])
      {
        marker[VERSION] = true;
        printf("Version : 2.0\n");
      }
      break;

    case 0:
    case -1:
    case 'g':
    default:
      break;
  }
}


int main(int argc, char *argv[])
{
  int option = 0;
  int cpt = 1;
  bool is_help_usage = false;
  bool is_generation_usage = false;
  bool marker[] = {false, false, false, false, false, false};
  FILE * file = NULL;

  bool ret = false;

  gen_opt_t gen_options =
  {
    .filename = NULL,
    .max_room_sz = DEFAULT_MAX,
    .min_room_sz = DEFAULT_MIN,
    .size = DEFAULT_SIZE,
    .unique = false,
    .operators = {true, true, true, true}
  };

  solve_opt_t solve_options =
  {
    .all_sols = false,
    .verbose_mod = false,
    .unique = false,
    .solver = LOGIC,
    .filename = NULL
  };

  const char short_opt[] = "ag::hm:M:o:O:s:uvV";
  struct option long_opt[] =
  {
    {"all"        , no_argument      , NULL, 'a'},
    {"generate"   , optional_argument, NULL, 'g'},
    {"help"       , no_argument      , NULL, 'h'},
    {"minimum"    , required_argument, NULL, 'm'},
    {"maximum"    , required_argument, NULL, 'M'},
    {"output"     , required_argument, NULL, 'o'},
    {"operators"  , required_argument, NULL, 'O'},
    {"solver"     , required_argument, NULL, 's'},
    {"unique"     , no_argument      , NULL, 'u'},
    {"verbose"    , no_argument      , NULL, 'v'},
    {"version"    , no_argument      , NULL, 'V'},
    {NULL         , no_argument      , NULL,  0 }
  };

  const char gen_short_opt[] = "g::m:M:o:O:uV";
  struct option gen_long_opt[] =
  {
    {"generate"   , optional_argument, NULL, 'g'},
    {"minimum"    , required_argument, NULL, 'm'},
    {"maximum"    , required_argument, NULL, 'M'},
    {"output"     , required_argument, NULL, 'o'},
    {"operators"  , required_argument, NULL, 'O'},
    {"unique"     , no_argument      , NULL, 'u'},
    {"version"    , no_argument      , NULL, 'V'},
    {NULL         , no_argument      , NULL,  0 }
  };

  const char solve_short_opt[] = "as:o:vV";
  struct option solve_long_opt[] =
  {
    {"all"     , no_argument      , NULL, 'a'},
    {"output"  , required_argument, NULL, 'o'},
    {"solver"  , required_argument, NULL, 's'},
    {"verbose" , no_argument      , NULL, 'v'},
    {"version" , no_argument      , NULL, 'V'},
    {NULL      , no_argument      , NULL,  0 }
  };

  if (argc < 2)
  {
    printf("\"./kenken --help\" to see how to use the kenken command.\n");
  }
  else
  {
    while (!is_help_usage && (cpt < argc))
    {
      option = getopt_long(argc, argv, short_opt, long_opt, NULL);
      if (option == 'h')
      {
        is_help_usage = true;
      }
      else
      {
        if (option == -1)
        {
          cpt++;
        }
      }
    }

    optind = RESTART;
    cpt = 0;

    if (is_help_usage)
    {
      display_help();
    }
    else
    {
      while (!is_generation_usage && (cpt < argc))
      {
        option = getopt_long(argc, argv, short_opt, long_opt, NULL);
        if (option == 'g')
        {
          is_generation_usage = true;

          if (optarg)
          {
            gen_options.size = strtoul(optarg, NULL, DECIMAL_SYSTEM);
          }
        }
        else
        {
          if (option == -1)
          {
            cpt++;
          }
        }
      }

      optind = RESTART;

      if (is_generation_usage)
      {
        do
        {
          generating_options(option, marker, &gen_options);
          option = getopt_long(argc, argv, gen_short_opt, gen_long_opt, NULL);
        } while (option > 0);

        if (optind < argc)
        {
          printf("kenken: error: unrecognized flag %s\n", argv[optind]);
          exit(1);
        }

        check_gen_opt(&gen_options);
        ret = generate(gen_options);

        free(gen_options.filename);
      }

      else
      {
        do
        {
          solving_options(option, marker, &solve_options);
          option = getopt_long(argc,
              argv,
              solve_short_opt,
              solve_long_opt,
              NULL);
        } while (option > 0);

        if (optind < argc)
        {

          file = fopen(argv[optind], "r");
          if (!file)
          {
            printf("Error: file: kenken.c function: main().\n"
                "kenken: error: can't open file \"%s\"\n", argv[optind]);
            exit(1);
          }

          grid_t grid;

          if (parse(file, &grid) == false)
          {
            fclose(file);
            exit(1);
          }

          ret = solve(grid, solve_options);

          free(grid.rooms);
          free(grid.constraints);

          free(solve_options.filename);

          optind++;
          fclose(file);
        }
      }
    }
  }

  if (!ret)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
