#include <stdlib.h>

void throw_an_exception ()
{
  throw 1;
}

typedef void (*thrower)();
typedef void (*call_thrower)(thrower);

extern "C" void func(call_thrower do_call_thrower)
{
  try {
    (*do_call_thrower) (throw_an_exception);
  } catch (int x) {
    return;
  }
  abort ();
}
