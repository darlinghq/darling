#include <stdio.h>
#include <stdlib.h>


__attribute__((constructor))
void init()
{
    printf("[FAIL]  dlopen-rpath-prev-override\n");
    exit(0);
}
