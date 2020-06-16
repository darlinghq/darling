typedef void (*thrower)();

extern "C" void
do_call_thrower (thrower do_throw)
{
  do_throw();
}
