#include <locale.h>
#include <stdio.h>

int main() {
  char* p = setlocale(2, "");
  printf("%p\n", p);
  printf("%s\n", p);
  return 0;
}
