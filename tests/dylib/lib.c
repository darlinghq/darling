#include <stdio.h>

int check_value;

__attribute__((constructor)) static void init() {
  check_value = 999;
}

int hello(const char* arg) {
  printf("Hello, %s!\n", arg);
  return 42;
}
