
// 
// TEST-OPTIONS:  backtrace2.c -arch i386 -fexceptions
// TEST-OPTIONS:  backtrace2.c -arch i386 -fexceptions -fomit-frame-pointer
// TEST-OPTIONS:  backtrace2.c -arch i386 -fexceptions -Wl,-no_compact_unwind
// TEST-OPTIONS:  backtrace2.c -arch x86_64
// TEST-OPTIONS:  backtrace2.c -arch x86_64 -fomit-frame-pointer
// TEST-OPTIONS:  backtrace2.c -arch x86_64 -Wl,-no_compact_unwind
// TEST-OPTIONS:  backtrace2.c -arch ppc -fexceptions
//
// Basic backtrace test
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "libunwind.h"

static const char* other_expected[] = { "bar", "foo", "work", "_pthread_start", NULL, NULL };
static const char* main_expected[] = { "bar", "foo", "main", NULL, NULL };


// put intermediate function into another file so compiler does not optimze away 
extern int foo(const char** list);


int bar(const char** list)
{
	char functionName[64];
	unw_cursor_t cursor; 
	unw_context_t uc;
	unw_word_t	offset;
	
	int index = 0;
	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);
	do {
		unw_get_proc_name(&cursor, functionName, 64, &offset);
		//fprintf(stderr, "in function: %s\n", functionName);
		if ( (list[index] != NULL) && (strcmp(functionName, list[index]) != 0) ) {
			//fprintf(stderr, "unexpected function: %s\n", functionName);
			exit(1);
		}
		++index;
	} while (unw_step(&cursor) > 0);
	return 0;
}

static void* work(void* arg)
{
	foo((const char**)arg);
	return NULL;
}


int main()
{
	// test back traces on another thread
	pthread_t worker1;
	if ( pthread_create(&worker1, NULL, work, other_expected) != 0 ) {
		exit(0);
	}
	void* result;
	pthread_join(worker1, &result);

	// test back traces on main thread
	return foo(main_expected);
}

