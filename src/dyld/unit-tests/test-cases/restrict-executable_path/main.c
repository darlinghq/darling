#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <string.h> // strcmp(), strncmp()
#include <unistd.h> // issetugid
#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


extern int foo();

int main(int argc, const char* argv[])
{
	foo();
	return EXIT_SUCCESS;
}
