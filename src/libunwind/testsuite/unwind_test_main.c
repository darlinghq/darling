
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "libunwind.h"

extern int unwind_tester(void*);
extern void* unwind_tester_list[];

int main()
{
	// loop over all function pointers in unwind_tester_list
	// and call unwind_tester() on each one.  If it returns
	// non-zero, then that test failed.
	void** p;
	for(p=unwind_tester_list; *p != NULL; ++p) {
		//fprintf(stderr, "unwind_tester(%p)\n", *p);
		if ( unwind_tester(*p) )
			return 1;
	}
	return 0;
}

// called by test function
// we unwind through the test function
// and resume at caller (unwind_tester)
void uwind_to_main()
{
	unw_cursor_t cursor; 
	unw_context_t uc;
	unw_word_t	offset;
	
	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);
	if ( unw_step(&cursor) > 0 ) {
		// now in test function
		if ( unw_step(&cursor) > 0 ) {
			// now in unwind_tester
			unw_resume(&cursor);
		}
	}
	// error if we got here
	exit(1);
}

