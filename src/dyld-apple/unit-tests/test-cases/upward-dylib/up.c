#include <stdio.h>
#include "down.h"

static int state = 0;

#ifndef STUB
// should run second because up.dylib is higher than down.dylib
static __attribute__((constructor)) void myInit1() 
{
	//fprintf(stderr, "myInit1()\n");
	if ( getdown() ) {
		state = 1;
	}
}
#endif

int whatsup()
{
	return state;
}

