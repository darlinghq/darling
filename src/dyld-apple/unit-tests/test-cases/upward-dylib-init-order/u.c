#include <stdio.h>
#include "common.h"


static __attribute__((constructor)) void myInit() 
{
	setU();
	//fprintf(stderr, "init u\n");
}

