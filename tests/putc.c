// It's very unfortunate that apple's putc_unlocked is a macro and depends on
// the layout of FILE.
#include <stdio.h>

int main() {
  putc_unlocked('h', stdout);
  putc_unlocked('o', stdout);
  putc_unlocked('g', stdout);
  putc_unlocked('e', stdout);
  putc_unlocked('\n', stdout);
  return 0;
}
