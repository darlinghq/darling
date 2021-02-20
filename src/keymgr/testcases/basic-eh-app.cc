#include <stdlib.h>

/* This is the most basic use of EH you can imagine.  It just throws and
   catches an exception.  It tests a significant fraction of
   keymgr.  */

void throw_an_exception ()
{
  throw 1;
}

int main()
{
  try {
    throw_an_exception();
  } catch (int x) {
    return 0;
  }
  abort ();
}
