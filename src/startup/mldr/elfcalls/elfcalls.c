#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <locale.h>
#include <unistd.h>
#include "elfcalls.h"
#include "threads.h"
#include <sys/un.h>
#include <sys/socket.h>
#include <fcntl.h>

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

static int get_errno(void)
{
	return errno;
}

extern struct sockaddr_un __dserver_socket_address_data[];

static const void* __dserver_socket_address(void) {
	return &__dserver_socket_address_data;
};

static int __dserver_new_socket(void) {
	int new_rpc_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (new_rpc_fd < 0) {
		return -1;
	}

	// make it close-on-exec
	int fd_flags = fcntl(new_rpc_fd, F_GETFD);
	if (fd_flags < 0) {
		close(new_rpc_fd);
		return -1;
	}
	if (fcntl(new_rpc_fd, F_SETFD, fd_flags | FD_CLOEXEC) < 0) {
		close(new_rpc_fd);
		return -1;
	}

	// auto-bind it
	sa_family_t family = AF_UNIX;
	if (bind(new_rpc_fd, (const struct sockaddr*)&family, sizeof(family)) < 0) {
		close(new_rpc_fd);
		return -1;
	}

	return new_rpc_fd;
};

void elfcalls_make(struct elf_calls* calls)
{
	calls->dlopen = dlopen_simple;
	calls->dlclose = dlclose;
	calls->dlsym = dlsym;
	calls->dlerror = dlerror;

	calls->dlopen_fatal = dlopen_fatal;
	calls->dlsym_fatal = dlsym_fatal;
	calls->dlclose_fatal = dlclose_fatal;

	calls->darling_thread_create = __darling_thread_create;
	calls->darling_thread_terminate = __darling_thread_terminate;
	calls->darling_thread_get_stack = __darling_thread_get_stack;

	calls->get_errno = get_errno;
	calls->exit = exit;

	calls->sysconf = sysconf;

	*((void**)&calls->sem_open) = sem_open;
	*((void**)&calls->sem_wait) = sem_wait;
	*((void**)&calls->sem_trywait) = sem_trywait;
	*((void**)&calls->sem_post) = sem_post;
	*((void**)&calls->sem_close) = sem_close;
	*((void**)&calls->sem_unlink) = sem_unlink;

	*((void**)&calls->shm_open) = shm_open;
	*((void**)&calls->shm_unlink) = shm_unlink;

	calls->dserver_socket_address = __dserver_socket_address;
	calls->dserver_new_socket = __dserver_new_socket;
}
