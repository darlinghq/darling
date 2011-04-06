#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  const char* err;

#ifdef __APPLE__
  void* h = dlopen("mach/dylib/lib.dylib", RTLD_NOW);
#else
  void* h = dlopen("mach/dylib/lib.so", RTLD_NOW);
#endif
  if (!h) {
    fprintf(stderr, "dlopen failed: %s\n", dlerror());
    abort();
  }

  if ((err = dlerror())) {
    fprintf(stderr, "dlopen failed (dlerror): %s\n", err);
    abort();
  }

  int (*hello)(const char*) = (int(*)(const char*))dlsym(h, "hello");
  if (!hello) {
    fprintf(stderr, "dlsym failed: %s\n", dlerror());
    abort();
  }

  if ((err = dlerror())) {
    fprintf(stderr, "dlsym failed (dlerror): %s\n", err);
    abort();
  }

  if (hello("world") != 42) {
    fprintf(stderr, "hello failed\n");
    abort();
  }

  int (*hell)(const char*) = (int(*)(const char*))dlsym(h, "hell");
  if (hell) {
    fprintf(stderr, "dlsym unexcepctedly succeeded: %s\n", dlerror());
    abort();
  }

  if (!(err = dlerror())) {
    fprintf(stderr, "dlsym unexcepctedly succeeded (dlerror)\n");
    abort();
  }

  printf("The error message for failing dlsym: %s\n", err);

  if (dlerror()) {
    fprintf(stderr, "dlerror has still been set\n");
    abort();
  }

  int* check_value = (int*)dlsym(h, "check_value");
  if (!check_value) {
    fprintf(stderr, "dlsym failed: %s\n", dlerror());
    abort();
  }

  if (*check_value != 999) {
    fprintf(stderr, "initializer function didn't run?\n");
    abort();
  }

  return 0;
}
