#include <stdio.h>
#include "up.h"

static int state = 0;


// should run second because down.dylib is lower than up.dylib 
static __attribute__((constructor)) void myInit3() 
{
	//fprintf(stderr, "myInit3()\n");
	state = 1;
}

int getdown()
{
	return state;
}

int getdownsup()
{
	return whatsup();
}