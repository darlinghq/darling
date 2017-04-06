
#include <stdlib.h> 

#include "foo.h"

test* maketestsub()
{
	return new testsub();
}

bool istestsub(test* t)
{
	return (dynamic_cast<testsub*>(t) != NULL);
}

