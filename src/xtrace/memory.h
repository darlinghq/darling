#ifndef XTRACE_MEMORY
#define XTRACE_MEMORY

#include "base.h"
#include <stddef.h>

XTRACE_DECLARATIONS_C_BEGIN

void* xtrace_malloc(size_t size);
void xtrace_free(void *ptr);
void* xtrace_realloc(void *ptr, size_t size);

XTRACE_DECLARATIONS_C_END

#endif // XTRACE_MEMORY