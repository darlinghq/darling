#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


#include "common.h"

int main()
{
	if ( ok() )
		PASS("upward-dylib-init-order");
	else
		FAIL("upward-dylib-init-order");
        
	return EXIT_SUCCESS;
}


