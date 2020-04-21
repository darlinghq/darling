
#include <stdlib.h>
#include <string.h>

char buffer[100000];
char* p = buffer;

void* malloc(size_t size)
{
    // bump ptr allocate and fill second half with '#'
    char* result = p;
    p += size;
    memset(result, '#', size);
    p = (char*)(((long)p + 15) & (-16)); // 16-byte align next malloc
    return result;
}

void free(void* p)
{
}

