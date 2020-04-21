#include <stdlib.h>
#include <string.h>
#include <mach-o/dyld-interposing.h>



int myFoo() {
  return 4;
}

int interposableFoo();

DYLD_INTERPOSE(myFoo, interposableFoo)
