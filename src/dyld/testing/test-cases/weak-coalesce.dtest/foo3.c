#include <stdio.h>

#include "base.h"

int __attribute__((weak))	coal1 = 3;
int __attribute__((weak))	coal2 = 2;

static __attribute__((constructor))
void myinit() 
{
	//fprintf(stderr, "myinit() in foo1.c\n");
	baseVerifyCoal1("in foo3", &coal1);
	baseVerifyCoal2("in foo3", &coal2);
}

