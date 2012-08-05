#include <mach-o/dyld.h>
#include <stdio.h>

int main() {
  char buf[4096];
  unsigned int size = 4095;
  int r = _NSGetExecutablePath(buf, &size);
  printf("r=%d %s\n", r, buf);
  return 0;
}
