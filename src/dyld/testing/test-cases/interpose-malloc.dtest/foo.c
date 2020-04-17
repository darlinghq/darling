
#include <stdlib.h>

void* myalloc1(size_t sz)
{
    return malloc(sz);
}

void* (*pMalloc)(size_t) = &malloc;

void* myalloc2(size_t sz)
{
    return (*pMalloc)(sz);
}

