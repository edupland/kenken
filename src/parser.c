#include "parser.h"

/* + - x / = */
static int is_operator(char c)
{
  return (c == '+'
      || c == '-'
      || c == 'x'
      || c == '/');
}

/* Valid characters (space, tabulations, line returns, end of file) */
static int is_valid_char(char c)
{
  return (c == ' '
      || c == '\n'
      || c == '\r'
      || c == '\t'
      || c == '\0'
      || c == EOF);
}

static operation_t char_to_operation(char c)
{

  operation_t result = 0;

  switch (c)
  {
    case ('+') :
      result = PLUS;
      break;
    case ('-') :
      result = MINUS;
      break;
    case ('x') :
      result = TIMES;
      break;
    case ('/') :
      result = DIV;
      break;
    default :
      result = EQ;
      break;
  }

  return result;;
}


static void skip_spaces(char *line, size_t *index)
{

  while (line[*index] == ' ' || line[*index] == '\t')
  {
    ++*index;
  }
}

static int read_int(char *line, size_t *index, size_t *line_num)
{

  size_t result = 0;

  if (isdigit(line[*index]))
  {

    while (isdigit(line[*index]))
    {
      result = INT_LEFT_SHIFT(result) + ASCII_TO_INT(line[*index]);
      ++*index;
    }

  }
  else
  {
    printf("kenken: error: line %lu: Unexpected character '%c'\n",
        *line_num,
        line[*index]);

    result = READ_INT_ERROR;
  }

  return result;
}

static int read_operator(char *line,
    size_t *index,
    grid_t *grid,
    size_t constraint_cpt,
    size_t *line_num)
{

  int result = true;

  if (is_operator(line[*index]) || is_valid_char(line[*index]))
  {
    grid->constraints[constraint_cpt].op = char_to_operation(line[*index]);
    ++*index;
  }
  else
  {
    printf("kenken: error: line %lu: Unexpected character '%c'\n",
        *line_num,
        line[*index]);

    result = false;
  }

  return result;
}

static int read_semi_colon(char *line, size_t *index, size_t *line_num)
{

  int result = true;

  if (line[*index] != ':')
  {

    printf("kenken: error: line %zu: '%c' but ':' is expected\n",
        *line_num,
        line[*index]);
    result = false;
  }

  ++*index;

  return result;
}

static int parse_rooms(grid_t *grid, size_t *line_num, FILE *file)
{

  char *line = NULL;
  size_t len = 0;

  /* buffer will contain one row of the grid's rooms */
  size_t buffer[MAX_GRID_SIZE];

  size_t line_cpt = 0;
  bool once = true;
  int result = true;
  int num;

  /* File is read line by line */
  while (getline(&line, &len, file) != GET_LINE_ERROR &&
      (once || line_cpt < grid->dim))
  {

    memset(buffer, 0 , MAX_GRID_SIZE * sizeof(size_t));
    len = strlen(line);
    (*line_num)++;

    size_t buffer_index = 0;

    /* Filling buffer with the numbers of the line*/
    for (size_t i = 0; i < len; ++i)
    {

      skip_spaces(line, &i);

      if (line[i] == '#' || line[i] == '\r' || line[i] == '\n')
      {
        break;
      }

      skip_spaces(line, &i);

      /* Reading one integer, and putting it in buffer */
      if ((num = read_int(line, &i, line_num)) == READ_INT_ERROR)
      {
        result = false;
        break;
      }
      else
      {
        buffer[buffer_index++] = num;
      }

      if (line[i] == '#')
      {
        break;
      }

      /* Verify there is no invalid character */
      if (!is_valid_char(line[i]))
      {
        printf("kenken: error: line %lu: Invalid character '%c'\n",
            *line_num,
            line[i]);
        result = false;
        break;
      }
    }

    if (result == false)
      break;

    /* Did we read some numbers ? */
    if (buffer_index)
    {

      /* Initializes the grid only the first time */
      if (once)
      {
        init_grid(grid, buffer_index);
        once = false;
      }

      fill_grid_line(line_cpt++, buffer, grid);
    }
  }

  free(line);

  return result;
}

static int parse_constraints(grid_t *grid, size_t *line_num, FILE *file)
{
  char *line = NULL;
  size_t len = 0;
  int constraint_cpt = 0;
  int result = true;
  int num;
  size_t i;

  init_constraints(grid);

  /* File is read line by line */
  while (getline(&line, &len, file) != GET_LINE_ERROR)
  {

    len = strlen(line);
    (*line_num)++;
    i = 0;

    skip_spaces(line, &i);

    /* Skip comment or empty line */
    if (line[i] == '#' || line[i] == '\n')
    {
      continue;
    }

    skip_spaces(line, &i);

    /* Expecting the constraint id */
    if ((num = read_int(line, &i, line_num)) == READ_INT_ERROR)
    {
      result = false;
      break;
    }
    else
    {
      if (num != constraint_cpt + 1)
      {
        return false;
      }
    }

    skip_spaces(line, &i);

    if (!read_semi_colon(line, &i, line_num))
    {
      result = false;
      break;
    }

    skip_spaces(line, &i);

    /* Reads the target number */
    if ((num = read_int(line, &i, line_num)) == READ_INT_ERROR)
    {
      result = false;
      break;
    }
    else
    {
      grid->constraints[constraint_cpt].target = num;
    }

    /* Reads the operator */
    if (!read_operator(line, &i, grid, constraint_cpt, line_num))
    {
      result = false;
      break;
    }

    ++constraint_cpt;
  }

  free(line);

  return result;

}

static int do_parse(FILE *file, grid_t *grid)
{
  int result = true;
  size_t line_num = 0;

  result = parse_rooms(grid, &line_num, file);

  result = parse_constraints(grid, &line_num, file) && result;

  return result;
}

bool parse(FILE *file, grid_t *grid)
{

  int result = true;

  result = do_parse(file, grid);

  return result;
}
