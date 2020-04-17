#include <stdio.h>

#include "base.h"

int							coal1 = 2;  // note: this is not weak and therefore should win
int __attribute__((weak))	coal2 = 2;

static __attribute__((constructor))
void myinit()
{
	//fprintf(stderr, "myinit() in foo1.c\n");
	baseVerifyCoal1("in foo2", &coal1);
	baseVerifyCoal2("in foo2", &coal2);
}
