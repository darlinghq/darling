#ifndef _ELFCALLS_H_
#define _ELFCALLS_H_
#include <stdint.h>

struct elf_calls
{
	void* (*dlopen)(const char* name);
	int (*dlclose)(void* lib);
	void* (*dlsym)(void* lib, const char* name);
	char* (*dlerror)(void);
	void* (*darling_thread_create)(unsigned long stack_size, unsigned long pthobj_size,
                void* entry_point, uintptr_t arg3,
                uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
                int (*thread_self_trap)());
	int (*darling_thread_terminate)(void* stackaddr,
				unsigned long freesize, unsigned long pthobj_size);
	void* (*darling_thread_get_stack)(void);
};

#endif

