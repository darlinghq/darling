#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include "elfcalls.h"
#include "threads.h"

extern uint8_t exe_uuid[16];

static void* dlopen_simple(const char* name)
{
	return dlopen(name, RTLD_LAZY);
}

static void* dlopen_fatal(const char* name)
{
	void* rv = dlopen_simple(name);
	if (!rv)
	{
		fprintf(stderr, "Cannot load %s (ELF): %s\n", name, dlerror());
		abort();
	}
	return rv;
}

static void* dlsym_fatal(void* handle, const char* sym)
{
	void* addr = dlsym(handle, sym);
	if (!addr)
	{
		fprintf(stderr, "Failed to lookup symbol %s (ELF): %s\n", sym, dlerror());
		abort();
	}
	return addr;
}

static int dlclose_fatal(void* handle)
{
	if (dlclose(handle) != 0)
	{
		fprintf(stderr, "Cannot dlclose library (ELF): %s\n", dlerror());
		abort();
	}
	return 0;
}

static const uint8_t* get_exe_uuid(void)
{
	return exe_uuid;
}

char* elfcalls_make(void)
{
	static char param[32];
	static struct elf_calls calls;

	calls.dlopen = dlopen_simple;
	calls.dlclose = dlclose;
	calls.dlsym = dlsym;
	calls.dlerror = dlerror;

	calls.dlopen_fatal = dlopen_fatal;
	calls.dlsym_fatal = dlsym_fatal;
	calls.dlclose_fatal = dlclose_fatal;

	calls.darling_thread_create = __darling_thread_create;
	calls.darling_thread_terminate = __darling_thread_terminate;
	calls.darling_thread_get_stack = __darling_thread_get_stack;

	calls.exe_uuid = get_exe_uuid;

	sprintf(param, "elf_calls=%p", &calls);
	return param;
}

