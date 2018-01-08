#include "elfcalls_wrapper.h"
#include <elfcalls.h>
#include <dlfcn.h>

static struct elf_calls* _elfcalls;

struct elf_calls* elfcalls(void)
{
	if (!_elfcalls)
	{
		void* module = dlopen("/usr/lib/libelfloader.dylib", RTLD_NOW);
		// if (!module)
		//	__simple_printf("Load error: %s\n", dlerror());
		
		// struct elf_calls** ptr = (struct elf_calls**) dlsym(module, "_elfcalls");
		// __simple_printf("_elfcalls is at %p\n", ptr);
		// __simple_printf("*_elfcalls = %p\n", *ptr);
		_elfcalls = *(struct elf_calls**) dlsym(module, "_elfcalls");
	}
	return _elfcalls;
}

void native_exit(int ec)
{
	if (_elfcalls)
		_elfcalls->exit(ec);
}

void* __darling_thread_create(unsigned long stack_size, unsigned long pthobj_size,
			void* entry_point, uintptr_t arg3,
			uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
			int (*thread_self_trap)())
{
	return elfcalls()->darling_thread_create(stack_size, pthobj_size, entry_point,
			arg3, arg4, arg5, arg6, thread_self_trap);
}

int __darling_thread_terminate(void* stackaddr,
			unsigned long freesize, unsigned long pthobj_size)
{
	return elfcalls()->darling_thread_terminate(stackaddr, freesize, pthobj_size);
}

void* __darling_thread_get_stack(void)
{
	return elfcalls()->darling_thread_get_stack();
}

void* native_dlopen(const char* path)
{
	return elfcalls()->dlopen(path);
}

char* native_dlerror(void)
{
	return elfcalls()->dlerror();
}

void* native_dlsym(void* module, const char* name)
{
	return elfcalls()->dlsym(module, name);
}

int native_dlclose(void* module)
{
	return elfcalls()->dlclose(module);
}

