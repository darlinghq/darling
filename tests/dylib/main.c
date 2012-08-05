#include "lib.h"

#include <stdlib.h>

int main() {
  if (hello("world") != 42)
    abort();
  return 0;
}
