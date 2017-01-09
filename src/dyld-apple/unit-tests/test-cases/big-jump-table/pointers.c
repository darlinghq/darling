
#include "foo.h"

long useNonLazy()
{
	long result = 0; 
	result += (long)&foo001;
	result += (long)&foo002;
	result += (long)&foo003;
#if CASE > 0
	result += (long)&foo818;
#endif
#if CASE > 1
	result += (long)&foo817;
#endif
#if CASE > 2
	result += (long)&foo816;
#endif
#if CASE > 3
	result += (long)&foo815;
#endif
	return result;
}
