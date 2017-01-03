#include "libdill.h"
#include "dsock.h"

int co(void **ptr, size_t len, void *fn, const char *file, int line, void (*routine)(void *));