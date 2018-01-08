#ifndef _ELFCALLS_WRAPPER_H
#define _ELFCALLS_WRAPPER_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Native library loading
void* native_dlopen(const char* path);
char* native_dlerror(void);
void* native_dlsym(void* module, const char* name);
int native_dlclose(void* module);

void native_exit(int ec);

// Native thread wrapping
void* __darling_thread_create(unsigned long stack_size, unsigned long pthobj_size,
			void* entry_point, uintptr_t arg3,
			uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
			int (*thread_self_trap)());

int __darling_thread_terminate(void* stackaddr,
			unsigned long freesize, unsigned long pthobj_size);

void* __darling_thread_get_stack(void);


#ifdef __cplusplus
}
#endif

#endif

