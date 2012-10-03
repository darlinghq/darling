#include <cstring>
#include <cstdlib>

extern "C" bool g_loggingEnabled = false;

__attribute__((constructor)) static void initLogging()
{
	const char* v = getenv("DYLD_DEBUG");
	if (v && atoi(v))
		g_loggingEnabled = true;
}

