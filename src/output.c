#include "output.h"

/* Redirects the stdout to a file. */
void redirect_stdout(char *filename, int * stdout_fd)
{
  FILE * file = fopen(filename, "w");
  if (!file)
  {
    exit(1);
  }
  fclose(file);

  int fd = open(filename, O_WRONLY);
  if (fd < 0)
  {
    exit(1);
  }
  *stdout_fd = dup(STDOUT_FILENO);
  if (*stdout_fd < 0)
  {
    exit(1);
  }
  if (dup2(fd, STDOUT_FILENO) < 0)
  {
    exit(1);
  }
  close(fd);
}

/* */
void flush(int * stdout_fd)
{
  fflush(stdout);
  if (dup2(*stdout_fd, STDOUT_FILENO) < 0)
  {
    exit(1);
  }
  close(*stdout_fd);
}
