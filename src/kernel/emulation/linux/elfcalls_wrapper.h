#ifndef _ELFCALLS_WRAPPER_H
#define _ELFCALLS_WRAPPER_H

#include <elfcalls.h>

#ifdef __cplusplus
extern "C" {
#endif

// Native library loading
void* native_dlopen(const char* path);
char* native_dlerror(void);
void* native_dlsym(void* module, const char* name);
int native_dlclose(void* module);

void native_exit(int ec);

long native_sysconf(int name);

// Native thread wrapping
void* __darling_thread_create(unsigned long stack_size, unsigned long pthobj_size,
			void* entry_point, uintptr_t arg3,
			uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
			darling_thread_create_callbacks_t callbacks, void* dthread);

int __darling_thread_terminate(void* stackaddr,
			unsigned long freesize, unsigned long pthobj_size);

void* __darling_thread_get_stack(void);

const void* __dserver_socket_address(void);
int __dserver_per_thread_socket(void);
void __dserver_per_thread_socket_refresh(void);
void __dserver_close_socket(int socket);

int __dserver_get_process_lifetime_pipe(void);
int __dserver_process_lifetime_pipe_refresh(void);
void __dserver_close_process_lifetime_pipe(int* fds);

#ifdef __cplusplus
}
#endif

#endif

