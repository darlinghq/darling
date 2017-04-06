
#include <mach-o/dyld-interposing.h>

extern int foo2();
extern int foo4() __attribute__((weak_import));



int myfoo2() { return 12; }
int myfoo4() { return 14; }



DYLD_INTERPOSE(myfoo2, foo2)
DYLD_INTERPOSE(myfoo4, foo4)
