#include <stdio.h>
#include "common.h"



static __attribute__((constructor)) void myInit() 
{
	setB();
	//fprintf(stderr, "init b\n");
}
