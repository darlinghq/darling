#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


#include "up.h"
#include "down.h"

int main()
{
	if ( whatsup() )
		PASS("upward-dylib");
	else
		FAIL("upward-dylib");
        
	return EXIT_SUCCESS;
}


