#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void shiftData(FILE* file, off_t src, off_t dst);

#endif // FILEOPERATIONS_H
