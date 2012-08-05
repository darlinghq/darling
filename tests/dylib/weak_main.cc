#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "weak.h"

int main() {
#ifdef DL
  int dlflags = RTLD_NOW;
# ifdef __APPLE__
  void* h = dlopen("mach/dylib/weak_lib.dylib", dlflags);
# else
  void* h = dlopen("mach/dylib/weak_lib.so", dlflags);
# endif
  if (!h) {
    printf("%s\n", dlerror());
    abort();
  }
#endif
  printf("%d %p\n", C<int>::weak, &C<int>::weak);
  if (C<int>::weak != 42) {
    abort();
  }
}
