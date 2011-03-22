#include <string.h>

int main(int argc, char* argv[]) {
  puts("run");
  if (strcmp(argv[argc-1], "a.out")) {
    puts("yes");
  } else {
    puts("no");
  }
  puts("done");
  return 0;
}
