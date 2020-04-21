
#include <stdlib.h>

extern int foo();

// test that calls to resolver based function in same dylib work
int fooPlusOne()
{
	return foo() + 1;
}

