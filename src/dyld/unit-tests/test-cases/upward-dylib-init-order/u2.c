#include <stdio.h>
#include "common.h"


static __attribute__((constructor)) void myInit() 
{
	setU2();
	//fprintf(stderr, "init u2\n");
}

