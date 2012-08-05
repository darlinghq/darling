#include <stdio.h>
#include <sys/time.h>

int main() {
  struct timeval tv;
  int r = gettimeofday(&tv, NULL);
  printf("%d %d\n", tv.tv_sec, tv.tv_usec);
  return 0;
}

