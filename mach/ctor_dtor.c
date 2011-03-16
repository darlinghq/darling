#include <stdio.h>

__attribute__((constructor)) void ctor() {
  puts("ctor");
}

__attribute__((constructor)) void ctor2() {
  puts("ctor2");
}

__attribute__((destructor)) void dtor() {
  puts("dtor");
}

__attribute__((destructor)) void dtor2() {
  puts("dtor2");
}

int main() {
  puts("main");
  return 0;
}
