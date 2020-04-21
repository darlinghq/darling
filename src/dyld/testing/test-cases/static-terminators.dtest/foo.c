#include <stddef.h>
#include <stdio.h>


extern void libDynamicTerminated();


static __attribute__((destructor))
void myTerm()
{
    //fprintf(stderr, "foo static terminator\n");
    libDynamicTerminated();
}

