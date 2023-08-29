#ifndef _XTRACE_MALLOC_H_
#define _XTRACE_MALLOC_H_

#include <stddef.h>
#include "base.h"

XTRACE_DECLARATIONS_C_BEGIN

void* xtrace_malloc(size_t size);
void xtrace_free(void* pointer);
void* xtrace_realloc(void* old_pointer, size_t new_size);

XTRACE_DECLARATIONS_C_END

#endif // _XTRACE_MALLOC_H_
