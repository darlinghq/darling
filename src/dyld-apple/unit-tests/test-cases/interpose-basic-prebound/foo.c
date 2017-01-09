
#include <stdbool.h>
#include <string.h>

bool check_dylib_interposing()
{
	const char* x = strdup("123");
	const char* y = strdup("456");
  
	return ( (strcmp(x, "hello") == 0) && (strcmp(y, "hello") == 0) );
}

