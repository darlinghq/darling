#include <stdlib.h>
#include <mach-o/dyld-interposing.h>

void* mymalloc(size_t s)
{
	return malloc(s);
}

DYLD_INTERPOSE(mymalloc, malloc)
