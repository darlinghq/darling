
#include <stdio.h>

int foo() 
{ 
    printf("foo\n");
    return 1; 
}
int bar() { return 1; }


static __attribute__((constructor)) void foo_init()
{
    printf("foo_init\n");
}

static __attribute__((destructor)) void foo_term()
{
    printf("foo_term\n");
}

