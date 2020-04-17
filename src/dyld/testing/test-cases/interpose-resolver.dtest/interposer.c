#include <stdlib.h>
#include <string.h>
#include <mach-o/dyld-interposing.h>

extern int foo();

int myfoo()
{
    return 11;
}


DYLD_INTERPOSE(myfoo, foo)
