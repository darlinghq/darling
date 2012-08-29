#include "config.h"
#include "nextstep.h"
#include <cstring>
#include "string.h" // for strlcpy
#include <unistd.h>

extern "C" char* dyld_getDarwinExecutablePath();
extern "C" char* dyld_getLoaderPath();

char* __darwin_executable_path = 0;
char* __loader_path = 0;

__attribute__((constructor)) void getLoaderGlobals()
{
	__darwin_executable_path = dyld_getDarwinExecutablePath();
	__loader_path = dyld_getLoaderPath();
}

int _NSGetExecutablePath(char* buf, unsigned int* size)
{
	strlcpy(buf, __darwin_executable_path, (*size)-1);

	*size = strlen(buf);

	return 0;
}

char*** _NSGetEnviron()
{
	return &environ;
}
