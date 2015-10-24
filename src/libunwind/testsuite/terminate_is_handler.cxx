
// TEST-OPTIONS: -arch i386 
// TEST-OPTIONS: -arch i386 -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch x86_64 
// TEST-OPTIONS: -arch x86_64 -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch ppc 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exception>


void my_terminate() {
    try {
        throw;
    } catch (char *s) {
		if ( (s == NULL) || (strcmp(s, "my exception") != 0) )
			exit(1);
        //printf("caught rethrown %p %s\n", s, s);
		exit(0);
    }
}


int main()
{
    std::set_terminate(&my_terminate);
    throw "my exception";
}

