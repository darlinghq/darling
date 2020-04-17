
#include <stdlib.h>
#include <mach-o/dyld-interposing.h>

extern int interposableFoo();
int (*pFoo)() = &interposableFoo;

int callFoo()
{
    return (*pFoo)();
}

extern int interposableBar();

int myBar() {
  return 2;
}

int (*pBar)() = &myBar;

int callBar()
{
  return (*pBar)();
}

DYLD_INTERPOSE(myBar, interposableBar)
