#include <stdlib.h>


static int foo_ten()
{
	return 10;
}

static int foo_zero()
{
	return 0;
}


// This foo is a "resolver" function that return the actual address of "foo"
void* foo()
{
	__asm__(".symbol_resolver _foo");  // magic until we have compiler support
#if TEN
		return &foo_ten;
#else
		return &foo_zero;
#endif
}


