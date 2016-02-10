
#include <stdio.h>
void bar()
{
	throw "wow";
}

extern "C" void foo();

void foo()
{
	try {
		bar();
	}
	catch(...) {
		//fprintf(stderr, "caught\n");
	}


}
