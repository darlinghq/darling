#include <stdio.h>
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <stdbool.h>
#include <string.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

// returns false on success
extern bool test1();

const char* str = "hello";


int main()
{
	if ( test1() )
		FAIL("branch-islands: test1");
	
	if ( strncmp(str, "he", 2) != 0 )
		FAIL("branch-islands: strncmp stub");

	
	PASS("branch-islands");
	return EXIT_SUCCESS;
}

