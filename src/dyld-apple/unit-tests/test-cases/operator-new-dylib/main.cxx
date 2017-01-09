#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

#include <stdlib.h>
#include <new>


extern char* foo();

static char all[] = "hello";

void* operator new[](std::size_t) throw (std::bad_alloc)
{
	return all;
}


int main()
{
	char* newArray = new char[24];
	char* fromFoo = foo();
	
	if ( fromFoo == newArray )
		PASS("operator-new-dylib");
	else
		FAIL("operator-new-dylib");
	return EXIT_SUCCESS;
}

