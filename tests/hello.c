#include <stdio.h>
int main() {
#ifdef __x86_64__
  printf("Hello, 64bit world!\n");
#else
  printf("Hello, 32bit world!\n");
#endif
  return 0;
}
