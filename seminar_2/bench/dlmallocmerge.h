#ifndef DLMALLOCMERGE
#define DLMALLOCMERGE

#include <stddef.h>

void *dalloc(size_t request);
void dfree(void *memory);
void init();
int countFlist();
double countAverageBlockSize();

#endif