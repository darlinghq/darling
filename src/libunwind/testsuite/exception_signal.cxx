
// TEST-OPTIONS: -arch i386 
// TEST-OPTIONS: -arch x86_64 
// TEST-OPTIONS: -arch ppc 

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <new>

void segfault(int sig)
{
	::signal(SIGBUS, SIG_DFL);
	::signal(SIGSEGV, SIG_DFL);

	//fprintf(stderr, "bus error handler for pid=%d\n", getpid());
	//sleep(30);
	throw "seg fault";
}


__attribute__((noinline)) void boom(int* p)
{
	// do allocation, so compiler won't optimize away catch clause in main()
	int* x = new int[100];
	*x = *p;
}

int main()
{
	::signal(SIGBUS, segfault);
	::signal(SIGSEGV, segfault);

	int state = 1;
	try {
		state = 2;
		boom(NULL);
		state = 3;
	}
	catch (const char* msg) {
		//fprintf(stderr, "caught %s\n", msg);
		if ( state != 2 )
			exit(1);
		state = 4;
	}
	catch(...) {
		//fprintf(stderr, "caught ...\n");
		exit(1);
	}

	if ( state == 4 )
		return 0;
	else
		return 1;
}

