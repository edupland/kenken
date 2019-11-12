#ifndef OUTPUT_H
#define OUTPUT_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void redirect_stdout(char *filename, int * stdout_fd);
void flush(int * stdout_fd);

#endif
