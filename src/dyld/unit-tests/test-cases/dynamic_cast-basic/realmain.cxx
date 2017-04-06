

#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

#include "foo.h"


void realmain()
{
	test* t1 = maketestsub();
	test* t2 = new testsub();
	test* t3 = new testsubother();
	testsub* t1a = dynamic_cast<testsub*>(t1);
	testsubother* t3a = dynamic_cast<testsubother*>(t3);
	if ( (t1a == NULL) || (t3a == NULL) || !istestsub(t2) )
		FAIL("dynamic_cast-basic");
	else
		PASS("dynamic_cast-basic");
}


