
#include <stdlib.h>

extern int interposableFoo();

int (*pFoo)() = &interposableFoo;

int callFoo()
{
    return (*pFoo)();
}

extern int interposableBar();

int (*pBar)() = &interposableBar;

int callBar()
{
  return (*pBar)();
}

