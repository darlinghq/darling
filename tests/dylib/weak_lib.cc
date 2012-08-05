#include <stdio.h>

#include "weak.h"

__attribute__((constructor)) void init_weak() {
  printf("init_weak before: %d %p\n", C<int>::weak, &C<int>::weak);
  C<int>::weak = 42;
  printf("init_weak after: %d %p\n", C<int>::weak, &C<int>::weak);
}
