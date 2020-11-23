#ifndef DLMALLOC
#define DLMALLOC

#include <stddef.h>

void *dalloc(size_t request);
void dfree(void *memory);
void init();
int countFlist();
double countAverageBlockSize();
void printFlist();

#endif