#include "solver.h"

size_t nb_sol;

/* logic_container_t */
static unsigned long long* initialize_logic_board(size_t dim)
{
  unsigned long long *board = malloc(sizeof(unsigned long long) * dim * dim);

  if (!board)
  {
    printf("Unsuccessful malloc\n");
    return false;
  }

  for (size_t i = 0; i < dim * dim; ++i)
  {
    board[i] = 1;
    for (size_t j = 0; j < dim - 1; ++j)
    {
      board[i] <<= 1;
      board[i] += 1;
    }
  }

  return board;
}

static void reset_board(logic_container_t lc1, logic_container_t lc2)
{
  for (size_t i = 0; i < lc1.dim * lc1.dim; ++i)
  {
    lc1.board[i] = lc2.board[i];
  }
}

static unsigned long long* copy_board(logic_container_t lc)
{
  unsigned long long *board =
    malloc(sizeof(unsigned long long) * lc.dim * lc.dim);

  if (!board)
  {
    printf("Unsuccessful malloc\n");
    return false;
  }

  for (size_t i = 0; i < lc.dim * lc.dim; ++i)
  {
    board[i] = lc.board[i];
  }

  return board;
}

static void initialize_logic_container(logic_container_t *lc, grid_t grid)
{
  lc->dim = grid.dim;
  lc->nb_rooms = grid.nb_constraints;
  lc->board = initialize_logic_board(lc->dim);
  lc->rooms = initialize_rooms_logic(grid);
}

static void free_logic_container(logic_container_t lc)
{

  for (size_t i = 0; i < lc.nb_rooms; ++i)
  {
    free(lc.rooms[i].cell_list);
  }

  free(lc.board);
  free(lc.rooms);
}

/* Reads the i-th bit, starting from the right */
static unsigned long long read_bit(unsigned long long b, size_t i)
{
  if (i > MAX_BIT)
    return 0;

  b >>= i;

  return b & 1;
}

static void print_board_no_options(logic_container_t lc)
{
  printf("Grid solved :\n");
  for (size_t i = 0; i < lc.dim; ++i)
  {
    for (size_t j = 0; j < lc.dim - 1; ++j)
    {
      printf("%d ", (int)log2l(lc.board[i * lc.dim + j]) + 1);
    }

    printf("%d\n", (int)log2l(lc.board[(i + 1) * lc.dim - 1]) + 1);
  }
}

static void print_board_all(logic_container_t lc)
{

  for (size_t i = 0; i < lc.dim; ++i)
  {
    for (size_t j = 0; j < lc.dim - 1; ++j)
    {
      printf("%d ", (int)log2l(lc.board[i * lc.dim + j]) + 1);
    }

    printf("%d\n", (int)log2l(lc.board[(i + 1) * lc.dim - 1]) + 1);
  }

  printf("---\n");
}

/* Sets the i-th bit to 1, starting from the right */
static void set_bit(unsigned long long *b, size_t i)
{
  unsigned long long mask = 1;

  mask <<= (i);

  *b |= mask;
}

/* Sets the i-th bit to 0, starting from the right */
static void remove_bit(unsigned long long *b, size_t i)
{
  unsigned long long mask = 1;

  mask <<= (i);

  *b &= ~mask;
}

static bool is_determined(unsigned long long b)
{
  return (b & (b - 1)) == 0;
}

static bool update_equal(logic_container_t lc, size_t c)
{
  cell_t *tmp = lc.rooms[c].cell_list;

  if (is_determined(lc.board[tmp->i * lc.dim + tmp->j]))
  {
    return false;
  }

  for (size_t k = 0; k < lc.dim; ++k)
  {

    /* Set all the bits of the cell to 0 except the target-th bit */
    if (k + 1 != lc.rooms[c].constraint.target)
    {
      remove_bit(&lc.board[tmp->i * lc.dim + tmp->j], k);
    }
  }

  return true;
}

static bool update_rows_cols(logic_container_t lc)
{

  bool ret = false;
  unsigned long long prev;

  if (lc.options.verbose_mod)
  {
    fprintf(stderr, "Updating rows and columns : ");
  }

  for (size_t i = 0; i < lc.dim; ++i)
  {
    for (size_t j = 0; j < lc.dim; ++j)
    {
      if (is_determined(lc.board[i * lc.dim + j]))
      {
        for (size_t k = 0; k < lc.dim; ++k)
        {
          /* Set the room's row's k-th cell's target-th bit to 0 */
          if (k != j)
          {
            prev = lc.board[i * lc.dim + k];
            lc.board[i * lc.dim + k] &= ~(lc.board[i * lc.dim + j]);
            ret = ret || (prev != lc.board[i * lc.dim + k]);
          }

          /* Set the room's column's k-th cell's target-th bit to 0 */
          if (k != i)
          {
            prev = lc.board[k * lc.dim + j];
            lc.board[k * lc.dim + j] &= ~(lc.board[i * lc.dim + j]);
            ret = ret || (prev != lc.board[k * lc.dim + j]);
          }
        }
      }
    }
  }

  if (lc.options.verbose_mod)
  {
    if (ret)
    {
      fprintf(stderr, "Board updated\n");
    }
    else
    {
      fprintf(stderr, "No changes\n");
    }
  }

  return ret;
}

static bool incr_loop_cpt(unsigned long long *loop_cpt,
    size_t index,
    size_t dim,
    size_t s)
{
  if (index == s)
  {
    return false;
  }

  if (loop_cpt[index] == dim)
  {
    loop_cpt[index] = 1;
    return incr_loop_cpt(loop_cpt, index + 1, dim, s);
  }
  else
  {
    loop_cpt[index]++;
    return true;
  }
}

static unsigned long long compute_cpt(unsigned long long *loop_cpt,
    size_t s,
    operation_t o)
{

  unsigned long long res = 0;
  unsigned long long max = 0;

  switch (o)
  {
    case (PLUS) :
      res = 0;
      for (size_t i = 0; i < s; ++i)
      {
        res += loop_cpt[i];
      }
      break;

    case (MINUS) :
      res = 0;
      max = 1;

      for (size_t i = 0; i < s; ++i)
      {
        if (loop_cpt[i] > max)
        {
          max = loop_cpt[i];
        }

        res += loop_cpt[i];
      }

      if (2 * max > res)
      {
        res = 2 * max - res;
      }
      else
      {
        res = 0;
      }

      break;

    case (TIMES) :
      res = 1;

      for (size_t i = 0; i < s; ++i)
      {

        res *= loop_cpt[i];
      }


      break;

    case (DIV) :
      res = 1;
      max = 1;

      for (size_t i = 0; i < s; ++i)
      {
        if (loop_cpt[i] > max)
        {
          max = loop_cpt[i];
        }
        res *= loop_cpt[i];
      }

      unsigned long long tmp = res / max;
      res /= tmp;

      if (max * max > res && res % tmp == 0)
      {
        res = res / tmp;
      }
      else
      {
        res = 0;
      }

      break;

    default :
      break;
  }

  return res;
}

static void init_masks(unsigned long long *masks, size_t s)
{
  for (size_t i = 0; i < s; ++i)
  {
    masks[i] = 0;
  }
}

static void update_masks(unsigned long long *masks,
    unsigned long long *loop_cpt,
    size_t s)
{
  for (size_t i = 0; i < s; ++i)
  {
    set_bit(masks + i, loop_cpt[i] - 1);
  }
}

static bool apply_masks(logic_container_t lc,
    unsigned long long *masks,
    size_t c)
{

  cell_t *tmp = lc.rooms[c].cell_list;
  unsigned long long prev;
  bool ret = false;

  size_t tmp_i;
  size_t tmp_j;

  for (size_t k = 0; k < lc.rooms[c].size; ++k)
  {
    tmp_i = tmp[k].i;
    tmp_j = tmp[k].j;

    prev = lc.board[tmp_i * lc.dim + tmp_j];
    lc.board[tmp_i * lc.dim + tmp_j] &= masks[k];

    if (prev != lc.board[tmp_i * lc.dim + tmp_j])
    {
      ret = true;
    }
  }

  return ret;
}

static bool valid_combination(logic_container_t lc,
    unsigned long long *loop_cpt,
    size_t s,
    size_t c)
{

  cell_t *tmp = lc.rooms[c].cell_list;

  bool ret = true;

  for (size_t i = 0; i < s; ++i)
  {
    if (!read_bit(lc.board[tmp[i].i * lc.dim + tmp[i].j], loop_cpt[i] - 1))
    {
      ret = false;
      break;
    }
  }

  return ret;
}

static bool update_room(logic_container_t lc, size_t c, operation_t o)
{

  size_t size = lc.rooms[c].size;
  unsigned long long *loop_cpt =
    malloc(sizeof(unsigned long long) * size);

  if (!loop_cpt)
  {
    printf("Unsuccessful malloc\n");
    return false;
  }

  for (size_t i = 0; i < size; ++i)
  {
    loop_cpt[i] = 1;
  }

  unsigned long long *masks = malloc(sizeof(unsigned long long) * size);
  if (!masks)
  {
    printf("Unsuccessful malloc\n");
    return false;
  }
  init_masks(masks, size);

  do
  {
    if (!valid_combination(lc, loop_cpt, size, c))
    {
      continue;
    }

    if (lc.rooms[c].constraint.target == compute_cpt(loop_cpt, size, o))
    {
      update_masks(masks, loop_cpt, size);
    }
  } while (incr_loop_cpt(loop_cpt, 0, lc.dim, size));

  bool ret = apply_masks(lc, masks, c);
  
  free(masks);
  free(loop_cpt);
  
  return ret;
}

static bool update_rooms(logic_container_t lc)
{

  bool ret = false;
  
  if (lc.options.verbose_mod)
  {
    fprintf(stderr, "Updating rooms : ");
  }

  for (size_t c = 0; c < lc.nb_rooms; c++)
  {
    if (lc.rooms[c].constraint.op == EQ)
    {
      ret = update_equal(lc , c) || ret;
    }
    else
    {
      ret = update_room(lc, c, lc.rooms[c].constraint.op) || ret;
    }
  }

  if (lc.options.verbose_mod)
  {
    if (ret)
    {
      fprintf(stderr, "Board updated\n");
    }
    else
    {
      fprintf(stderr, "No Changes done\n");
    }
  }

  return ret;
}

static bool is_solved(logic_container_t lc)
{
  unsigned long long b;

  unsigned long long filter = 0;
  for (size_t i = 0; i < lc.dim; ++i)
  {
    filter <<= 1;
    filter += 1;
  }

  for (size_t i = 0; i < lc.dim; ++i)
  {
    b = 0;

    for (size_t j = 0; j < lc.dim; ++j)
    {
      b += lc.board[i * lc.dim + j];
    }

    if (b != filter)
      return false;
  }

  return true;
}

static size_t nb_set_bits(unsigned long long b)
{
  if (b == 0)
  {
    return 0;
  }
  else
  {
    return 1 + nb_set_bits(b & (b - 1));
  }

}

static bool find_undetermined(size_t *i, size_t *j, logic_container_t lc)
{
  size_t size_min = lc.dim;
  size_t size;

  for (size_t r = 0; r < lc.dim; ++r)
  {
    for (size_t c = 0; c < lc.dim; ++c)
    {
      if (!is_determined(lc.board[r * lc.dim + c]))
      {
        /* find the cell with the smallest number of possibilities */
        size = nb_set_bits(lc.board[r * lc.dim + c]);

        if (size_min > size)
        {
          *i = r;
          *j = c;
          size_min = size;
        }
      }
    }
  }

  return size_min != lc.dim;
}

static bool check_zeros(logic_container_t lc)
{
  bool ret = false;

  for (size_t i = 0; i < lc.dim * lc.dim; ++i)
  {
    if (lc.board[i] == 0)
    {
      ret = true;
      break;
    }
  }

  return ret;
}

static bool naked_subset(logic_container_t lc, bool *stop)
{
  bool *row = malloc(sizeof(bool) * lc.dim);

  if (!row)
  {
    printf("Unsuccessful malloc\n");
    return false;
  }

  bool *col = malloc(sizeof(bool) * lc.dim);

  if (!col)
  {
    printf("Unsuccessful malloc\n");
    return false;
  }

  size_t cpt_row;
  size_t cpt_col;
  size_t k;
  size_t nb;
  unsigned long long subset;
  unsigned long long prec;
  bool ret = false;

  if (lc.options.verbose_mod)
  {
    fprintf(stderr, "Applying naked subset method : ");
  }

  for (size_t i = 0; i < lc.dim; ++i)
  {
    for (size_t j = 0; j < lc.dim; ++j)
    {
      subset = lc.board[i * lc.dim + j];
      memset(col, 0, lc.dim * sizeof(bool));
      memset(row, 0, lc.dim * sizeof(bool));
      cpt_row = 0;
      cpt_col = 0;

      for (k = 0; k < lc.dim; ++k)
      {
        if ((~subset & lc.board[i * lc.dim + k]) == 0)
        {
          row[k] = true;
          cpt_row++;
        }

        if ((~subset & lc.board[k * lc.dim + j]) == 0)
        {
          col[k] = true;
          cpt_col++;
        }
      }

      nb = nb_set_bits(subset);

      if (cpt_row > nb || cpt_col > nb)
      {
        if (lc.options.verbose_mod)
        {
          fprintf(stderr, "Stopping naked subset\n");
        }

        free(row);
        free(col);

        *stop = true;
        return false;
      }

      if (cpt_row == nb)
      {
        for (k = 0; k < lc.dim; ++k)
        {
          if (!row[k])
          {
            prec = lc.board[i * lc.dim + k];
            lc.board[i * lc.dim + k] &= ~subset;
            ret = ret || (prec != lc.board[i * lc.dim + k]);
          }
        }
      }

      if (cpt_col == nb)
      {
        for (k = 0; k < lc.dim; ++k)
        {
          if (!col[k])
          {
            prec = lc.board[k * lc.dim + j];
            lc.board[k * lc.dim + j] &= ~subset;
            ret = ret || (prec != lc.board[k * lc.dim + j]);
          }
        }
      }
    }
  }

  if (lc.options.verbose_mod)
  {
    if (ret)
    {
      fprintf(stderr, "Board updated\n");
    }
    else
    {
      fprintf(stderr, "No Changes done\n");
    }
  }

  free(row);
  free(col);

  return ret;
}

/* Seeks all two-sized hidden subsets */
static bool hidden_subset(logic_container_t lc)
{
  bool *row = malloc(sizeof(bool) * lc.dim);

  if (!row)
  {
    printf("Unsuccessful malloc\n");
    return false;
  }

  bool *col = malloc(sizeof(bool) * lc.dim);

  if (!col)
  {
    printf("Unsuccessful malloc\n");
    return false;
  }

  unsigned long long hidden_subset;
  unsigned long long prec;
  bool ret = false;
  size_t cpt_row;
  size_t cpt_col;

  if (lc.options.verbose_mod)
  {
    fprintf(stderr, "Applying hidden subset method : ");
  }

  for (size_t e = 0; e + 1 < lc.dim; e++)
  {
    for (size_t f = e + 1; f < lc.dim; f++)
    {
      hidden_subset = 1;
      hidden_subset <<= f - e;
      hidden_subset += 1;
      hidden_subset <<= e;

      for (size_t i = 0; i < lc.dim; ++i)
      {
        memset(col, 0, lc.dim * sizeof(bool));
        memset(row, 0, lc.dim * sizeof(bool));
      
        cpt_row = 0;
        cpt_col = 0;
        
        for (size_t j = 0; j < lc.dim; ++j)
        {
          if ((lc.board[i * lc.dim + j] & hidden_subset) != 0)
          {
            row[j] = true;
            cpt_row++;
          }

          if ((lc.board[j * lc.dim + i] & hidden_subset) != 0)
          {
            col[j] = true;
            cpt_col++;
          }
        }

        if (cpt_row == 2)
        {
          for (size_t k = 0; k < lc.dim; ++k)
          {
            if (row[k])
            {
              prec = lc.board[i * lc.dim + k];
              lc.board[i * lc.dim + k] &= hidden_subset;
              ret = ret || (lc.board[i * lc.dim + k] != prec);
            }
          }
        }

        if (cpt_col == 2)
        {
          for (size_t k = 0; k < lc.dim; ++k)
          {
            if (col[k])
            {
              prec = lc.board[k * lc.dim + i];
              lc.board[k * lc.dim + i] &= hidden_subset;
              ret = ret || (lc.board[k * lc.dim + i] != prec);
            }
          }
        }
      }
    }
  }

  if (lc.options.verbose_mod)
  {
    if (ret)
    {
      fprintf(stderr, "Board updated\n");
    }
    else
    {
      fprintf(stderr, "No Changes done\n");
    }
  }

  free(row);
  free(col);

  return ret;
}

static bool logic_rec(logic_container_t lc)
{

  if (lc.options.unique && nb_sol > 1)
  {
    return false;
  }

  bool modif;
  bool stop = false;

  do
  {
    modif = false;

    /* Update possibility : Rooms constraints */
    modif = update_rooms(lc);

    /* Naked subset */
    modif = naked_subset(lc, &stop) || modif;

    if (stop)
      return false;

    /* Hidden subset */
    modif = hidden_subset(lc) || modif;

    /* Update possibility : Rows and cols constraints */
    modif = update_rows_cols(lc) || modif;

    if (check_zeros(lc))
    {
      return false;
    }

  } while(modif);

  if (!is_solved(lc))
  {
    size_t i = 0;
    size_t j = 0;

    if (!find_undetermined(&i, &j, lc))
    {
      return false;
    }

    logic_container_t lc_tmp;
    lc_tmp.dim = lc.dim;
    lc_tmp.nb_rooms = lc.nb_rooms;
    lc_tmp.rooms = lc.rooms;
    lc_tmp.options = lc.options;
    lc_tmp.board = copy_board(lc);

    for (size_t n = 1; n <= lc.dim; ++n)
    {
      if (lc.options.verbose_mod)
      {
        fprintf(stderr, "Testing %zu in cell %zu, %zu\n", n, i, j);
      }

      if (read_bit(lc_tmp.board[i * lc_tmp.dim + j], n - 1))
      {
        for (size_t k = 0; k < lc_tmp.dim; ++k)
        {

          if (k + 1 != n)
          {
            remove_bit(&lc_tmp.board[i * lc_tmp.dim + j], k);
          }
        }

        if (logic_rec(lc_tmp))
        {
          reset_board(lc, lc_tmp);
          free(lc_tmp.board);

          return true;
        }

        /* Reset */
        reset_board(lc_tmp, lc);
      }
    }

    free(lc_tmp.board);
    return false;
  }

  if (lc.options.unique)
  {
    nb_sol++;
    return false;
  }


  if (lc.options.all_sols)
  {
    print_board_all(lc);
    nb_sol++;
    return false;
  }
  else
  {
    return true;
  }
}

bool logic(grid_t grid, solve_opt_t solve_options)
{
  logic_container_t lc;
  lc.options = solve_options;

  initialize_logic_container(&lc, grid);

  if (lc.options.all_sols)
  {
    nb_sol = 0;
  }

  if (!logic_rec(lc))
  {
    if (lc.options.all_sols && nb_sol)
    {
      printf("%zu solution%c found\n", nb_sol, nb_sol == 1 ? '\0' : 's');
    }
  }
  else
  {
    print_board_no_options(lc);
  }

  free_logic_container(lc);

  return true;
}

bool check_uniqueness(grid_t grid)
{
  bool ret = true;
  
  logic_container_t lc;

  solve_opt_t so =
  {
    .all_sols = false,
    .verbose_mod = false,
    .unique = true,
    .solver = 0,
    .filename = NULL
  };
  lc.options = so;
  
  nb_sol = 0;

  initialize_logic_container(&lc, grid);

  logic_rec(lc);
  
  if (nb_sol != 1)
  {
    ret = false;
  }

  free_logic_container(lc);

  return ret;
 
}
