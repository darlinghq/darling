#ifndef _ELFCALLS_H_
#define _ELFCALLS_H_
#include <stdint.h>

struct elf_calls
{
	// ELF synamic loader access
	void* (*dlopen)(const char* name);
	int (*dlclose)(void* lib);
	void* (*dlsym)(void* lib, const char* name);
	char* (*dlerror)(void);

	// pthread wrapping
	void* (*darling_thread_create)(unsigned long stack_size, unsigned long pthobj_size,
                void* entry_point, uintptr_t arg3,
                uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
                int (*thread_self_trap)());
	int (*darling_thread_terminate)(void* stackaddr,
				unsigned long freesize, unsigned long pthobj_size);
	void* (*darling_thread_get_stack)(void);

	// The same as above, except they abort() in case of failure
	void* (*dlopen_fatal)(const char* name);
	int (*dlclose_fatal)(void* lib);
	void* (*dlsym_fatal)(void* lib, const char* name);

	// Get main executable's UUID (16 bytes)
	const uint8_t* (*exe_uuid)(void);

};

#endif

