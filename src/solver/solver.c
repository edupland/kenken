#include "solver.h"

/* Initialize the array of cell lists. The i_th list of the array
 * represents the i_th constraint of the grid.
 * */
cells_list_t **initialize_rooms(grid_t grid)
{

  cells_list_t **rooms = malloc(sizeof(cells_list_t*) * grid.nb_constraints);

  if (rooms == NULL)
  {
    printf("Error: file: solver.c, function: initialize_rooms().\n"
        "Memory for rooms can't be allocated\n");
    exit(1);
  }

  for (size_t k = 0; k < grid.nb_constraints; ++k)
  {
    rooms[k] = NULL;
  }

  cells_list_t *tmp = NULL;

  for (size_t i = 0; i < grid.dim; ++i)
  {
    for (size_t j = 0; j < grid.dim; ++j)
    {
      tmp = rooms[grid.rooms[i * grid.dim + j] - 1];

      if (!tmp)
      {
        rooms[grid.rooms[i * grid.dim + j] - 1] = malloc(sizeof(cells_list_t));

        if (rooms[grid.rooms[i * grid.dim + j] - 1] == NULL)
        {
          printf("Error: file: solver.c, "
              "function: initialize_rooms().\n"
              "Memory for that room can't be allocated\n");
          exit(1);
        }

        rooms[grid.rooms[i * grid.dim + j] - 1]->i = i;
        rooms[grid.rooms[i * grid.dim + j] - 1]->j = j;
        rooms[grid.rooms[i * grid.dim + j] - 1]->next = NULL;
      }
      else
      {
        while (tmp->next)
        {
          tmp = tmp->next;
        }

        tmp->next = malloc(sizeof(cells_list_t));

        if (tmp->next == NULL)
        {
          printf("Error: file: solver.c, "
              "function: initialize_rooms().\n"
              "Memory for tmp->next can't be allocated\n");
          exit(1);
        }

        tmp->next->i = i;
        tmp->next->j = j;
        tmp->next->next = NULL;
      }
    }
  }

  return rooms;
}

room_t *initialize_rooms_logic(grid_t grid)
{
  /* First we get rooms' sizes */
  size_t *sizes = calloc(grid.nb_constraints, sizeof(size_t));

  for (size_t i = 0; i < grid.dim; ++i)
  {
    for (size_t j = 0; j < grid.dim; ++j)
    {
      ++sizes[grid.rooms[i * grid.dim + j] - 1];
    }
  }

  /* Initialize the rooms according to their size */
  room_t *ret = malloc(grid.nb_constraints * sizeof(room_t));

  for (size_t c_id = 0; c_id < grid.nb_constraints; ++c_id)
  {
    ret[c_id].size = sizes[c_id];
    ret[c_id].constraint = grid.constraints[c_id];
    ret[c_id].cell_list = malloc(sizes[c_id] * sizeof(cell_t));
  }

  /* Store the rooms' cells location */
  size_t *cpt_arr = calloc(grid.nb_constraints, sizeof(size_t));

  for (size_t i = 0; i < grid.dim; ++i)
  {
    for (size_t j = 0; j < grid.dim; ++j)
    {
      size_t c_id = grid.rooms[i * grid.dim + j] - 1;
      ret[c_id].cell_list[cpt_arr[c_id]].i = i;
      ret[c_id].cell_list[cpt_arr[c_id]].j = j;
      ++cpt_arr[c_id];
    }
  }

  free(cpt_arr);
  free(sizes);

  return ret;
}
