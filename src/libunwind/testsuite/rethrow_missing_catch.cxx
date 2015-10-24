

// TEST-OPTIONS: -arch i386 
// TEST-OPTIONS: -arch i386 -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch x86_64  
// TEST-OPTIONS: -arch x86_64  -Wl,-no_compact_unwind


#include <stdlib.h>

#include <exception>


int foo() 
{ 
	try {
		throw 10; 
	} 
	catch (...) {
		throw;
	}
}

static void term()
{
	// terminate called, as we want
	exit(0);
}


int main()
{
	std::set_terminate(term);
	
	foo();

	// should not get here
	exit(1);
}

