

// TEST-OPTIONS: -x c exception_missing_eh2.c -arch i386 
// TEST-OPTIONS: -x c exception_missing_eh2.c -arch i386 -Wl,-no_compact_unwind
// TEST-OPTIONS: -x c exception_missing_eh2.c -arch ppc 
// TEST-OPTIONS: exception_missing_eh2_x86_64.s -arch x86_64  
// TEST-OPTIONS: exception_missing_eh2_x86_64.s -arch x86_64  -Wl,-no_compact_unwind


#include <stdlib.h>

#include <exception>

extern "C" {
	extern int bar();
	extern int foo();
}


int foo() { throw 10; }

static void term()
{
	// terminate called, as we want
	exit(0);
}


int main()
{
	std::set_terminate(term);
	
	int state = 1;
	try {
		state = 2;
		// bar() calls foo() which throws
		// but, bar is missing eh info, so terminate() is called
		bar();
		state = 3;
	}
	catch (int x) {
		if ( state != 2 )
			return 1;
		if ( x != 10 )
			return 1;
		state = 4;
	}

	// should not get here
	exit(1);
}

