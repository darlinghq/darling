#include <stdio.h>



static __attribute__((constructor)) void myInit() 
{
	//fprintf(stderr, "init c\n");
}
