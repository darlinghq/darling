
#include <mach-o/dyld-interposing.h>

extern void foo();

void my_foo() { }

DYLD_INTERPOSE(my_foo, foo);
