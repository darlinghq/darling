#include "config.h"
#include "misc.h"
#include "trace.h"
#include <langinfo.h>
#include <cstdlib>
#include <cstring>

const char* locale_charset()
{
	return nl_langinfo(CODESET);
}

int __darwin_sigsetjmp(sigjmp_buf b, int savesigs)
{
	return sigsetjmp(b, savesigs);
}

extern "C" void* __darwin_realloc(void* p, size_t s)
{
	TRACE2(p, s);
	//if (p < (void*)0xffff)
	//	p = 0;
	return realloc(p,s);
}

int __darwin_atexit(void (*function)(void))
{
	return atexit(function);
}

void* __darwin_malloc(size_t size)
{
	void* p = malloc(size);
	memset(p, 0, size);
	return p;
}
