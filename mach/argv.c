#include <stdio.h>

int main(int argc, char* argv[]) {
  printf("argc=%d\n", argc);
  printf("argv[0]=%p\n", argv[0]);
  printf("argv[0]=%s\n", argv[0]);
  printf("argv[1]=%p\n", argv[1]);
  printf("argv[1]=%s\n", argv[1]);
  return 0;
}
