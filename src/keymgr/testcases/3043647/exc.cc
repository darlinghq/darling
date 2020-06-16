#include <stdio.h>

extern "C" int do_test(int n)
{
  printf ("Entering do_test\n");
  printf ("Normal return from do_test.\n");
  return 0;
}
