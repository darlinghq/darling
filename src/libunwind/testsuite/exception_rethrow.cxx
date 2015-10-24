
// TEST-OPTIONS: -arch i386 
// TEST-OPTIONS: -arch i386 -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch x86_64 
// TEST-OPTIONS: -arch x86_64  -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch ppc 

#include <stdio.h>

int global = 0;

int bar() 
{ 
	global = 1;
	throw 10; 
	
}


void foo() 
{ 
	try {
		bar();
	}
	catch(int x) {
		global = 2;
		throw;
	}
}



int main()
{
	int state = 1;
	try {
		state = 2;
		foo();
		state = 3;
	}
	catch (int x) {
		if ( state != 2 )
			return 1;
		if ( x != 10 )
			return 1;
		state = 4;
	}

	if ( (state == 4) && (global == 2) )
		return 0;
	else 
		return 1;
}

