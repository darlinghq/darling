#include <stdio.h>
#include <string.h>
#include <mach-o/dyld-interposing.h>
#include "base.h"
#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


int (*p2)() = &base2;

__attribute__((constructor))
void myinit()
{
	if ( (*p2)() == 20 )
		PASS("interpose-multiple");
	else
		FAIL("interpose-multiple");
}


int mybase1()
{
	return 10;
}


DYLD_INTERPOSE(mybase1, base1)
