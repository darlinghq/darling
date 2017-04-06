#include <stdio.h>
#include <string.h>
#include <mach-o/dyld-interposing.h>
#include "base.h"
#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

int (*p1)() = &base1;

__attribute__((constructor))
void myinit()
{
	if ( (*p1)() == 10 )
		PASS("interpose-multiple");
	else
		FAIL("interpose-multiple");
}


int mybase2()
{
	return 20;
}


DYLD_INTERPOSE(mybase2, base2)
