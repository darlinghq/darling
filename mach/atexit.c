#include <stdio.h>
#include <stdlib.h>

void f() {
  puts("atexit");
}

int main() {
  atexit(f);
  return 0;
}
