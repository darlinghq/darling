#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


extern int var;
extern void func();
extern int myAbs1 __attribute__((weak_import));
extern int myAbs2;


int main()
{
	
	if (&myAbs1 != 0 ) {
		FAIL("absolute-symbol: &myAbs1 != 0");
		return 0;
	}
	
	
	if ((uintptr_t)&myAbs2 != 1 ) {
		FAIL("absolute-symbol: &myAbs2 != 1");
		return 0;
	}
	
	
	PASS("absolute-symbol");
	return 0;
}
