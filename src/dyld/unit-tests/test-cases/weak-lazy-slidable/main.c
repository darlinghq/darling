

#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <stdbool.h> 

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


extern bool foo1();
extern bool foo2();


int main()
{
	if ( foo1() && foo2() ) 
		PASS("weak-lazy-slidable");
	else
		FAIL("weak-lazy-slidable");
			
	return EXIT_SUCCESS;
}


