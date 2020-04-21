
#include <stdio.h>

extern int bar();
extern int bazIsInited();

int foo() {
	if ( bar() != 0 ) {
		return 1;
	}
	if ( bazIsInited() == 0 ) {
		printf("[FAIL]  dlopen-in-init3, didn't init baz\n");
		return 1;
	}
	return 0;
}