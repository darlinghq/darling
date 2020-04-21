#include <stdlib.h>
#include <string.h>
#include <mach-o/dyld-interposing.h>



void* mymalloc(size_t size)
{
    // bump ptr allocate twice the size and fills with '#'
    char* result = malloc(size*2);
    memset(result, '#', size*2);
    return result;
}

void myfree(void* p)
{
    free(p);
}

DYLD_INTERPOSE(mymalloc, malloc)
DYLD_INTERPOSE(myfree, free)
