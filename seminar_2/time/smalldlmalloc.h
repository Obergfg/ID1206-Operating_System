#ifndef SMALLDLMALLOC
#define SMALLDLMALLOC

#include <stddef.h>

void *dalloc(size_t request);
void dfree(void *memory);
void init();

#endif