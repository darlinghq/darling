
// TEST-OPTIONS: -arch i386 
// TEST-OPTIONS: -arch i386 -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch x86_64 
// TEST-OPTIONS: -arch x86_64  -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch ppc 


int foo() { throw 10; }


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

	if ( state == 4 )
		return 0;
	else
		return 1;
}

