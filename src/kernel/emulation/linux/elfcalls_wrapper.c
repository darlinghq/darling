#include "elfcalls_wrapper.h"
#include <elfcalls.h>

extern struct elf_calls* _elfcalls;

void* __darling_thread_create(unsigned long stack_size, unsigned long pthobj_size,
			void* entry_point, uintptr_t arg3,
			uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
			int (*thread_self_trap)())
{
	return _elfcalls->darling_thread_create(stack_size, pthobj_size, entry_point,
			arg3, arg4, arg5, arg6, thread_self_trap);
}

int __darling_thread_terminate(void* stackaddr,
			unsigned long freesize, unsigned long pthobj_size)
{
	return _elfcalls->darling_thread_terminate(stackaddr, freesize, pthobj_size);
}

void* __darling_thread_get_stack(void)
{
	return _elfcalls->darling_thread_get_stack();
}

void* native_dlopen(const char* path)
{
	return _elfcalls->dlopen(path);
}

char* native_dlerror(void)
{
	return _elfcalls->dlerror();
}

void* native_dlsym(void* module, const char* name)
{
	return _elfcalls->dlsym(module, name);
}

int native_dlclose(void* module)
{
	return _elfcalls->dlclose(module);
}

const uint8_t* get_exe_uuid(void)
{
	return _elfcalls->exe_uuid();
}

