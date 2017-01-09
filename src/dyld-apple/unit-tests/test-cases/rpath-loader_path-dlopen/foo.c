#include <stdbool.h>
#include <stdlib.h>
#include <dlfcn.h>

bool foo()
{
	return (dlopen("./libbar.dylib", RTLD_NOW) != NULL);
}
