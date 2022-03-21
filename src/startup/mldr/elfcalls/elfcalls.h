#ifndef _ELFCALLS_H_
#define _ELFCALLS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct darling_thread_create_callbacks {
	int (*thread_self_trap)(void);
	void (*thread_set_tsd_base)(void*, int);
	void (*rpc_guard)(int);
	void (*rpc_unguard)(int);
};

typedef const struct darling_thread_create_callbacks* darling_thread_create_callbacks_t;

struct elf_calls
{
	// ELF dynamic loader access
	void* (*dlopen)(const char* name);
	int (*dlclose)(void* lib);
	void* (*dlsym)(void* lib, const char* name);
	char* (*dlerror)(void);

	// pthread wrapping
	void* (*darling_thread_create)(unsigned long stack_size, unsigned long pthobj_size,
                void* entry_point, uintptr_t arg3,
                uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
                darling_thread_create_callbacks_t callbacks, void* dthread);
	int (*darling_thread_terminate)(void* stackaddr,
				unsigned long freesize, unsigned long pthobj_size);
	void* (*darling_thread_get_stack)(void);

	// The same as above, except they abort() in case of failure
	void* (*dlopen_fatal)(const char* name);
	int (*dlclose_fatal)(void* lib);
	void* (*dlsym_fatal)(void* lib, const char* name);

	// POSIX semaphore APIs
	int (*get_errno)(void);
	int* (*sem_open)(const char* name, int oflag, unsigned short mode, unsigned int value);
	int (*sem_wait)(int* sem);
	int (*sem_trywait)(int* sem);
	int (*sem_post)(int* sem);
	int (*sem_close)(int* sem);
	int (*sem_unlink)(const char* name);

	// POSIX SHM APIs
	int (*shm_open)(const char* name, int oflag, unsigned short mode);
	int (*shm_unlink)(const char* name);

	void (*exit)(int ec);

	// POSIX sysconf
	long (*sysconf)(int name);

	// darlingserver RPC info
	const void* (*dserver_socket_address)(void);
	int (*dserver_per_thread_socket)(void);
	void (*dserver_per_thread_socket_refresh)(void);
};

#endif

