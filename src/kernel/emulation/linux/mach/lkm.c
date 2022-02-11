#include "lkm.h"
#include "../../external/lkm/api.h"
#include "../signal/sigexc.h"
#include "../base.h"
#include "../linux-syscalls/linux.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include "../../../libsyscall/wrappers/_libkernel_init.h"
#include "../simple.h"
#include "../misc/ioctl.h"
#include <elfcalls.h>

extern int sys_open(const char*, int, int);
extern int close_internal(int);
extern int sys_write(int, const void*, int);
extern int sys_kill(int, int);
extern int sys_getrlimit(int, struct rlimit*);
extern int sys_dup2(int, int);
extern int sys_fcntl(int, int, int);
extern _libkernel_functions_t _libkernel_functions;


int driver_fd = -1;

VISIBLE
struct elf_calls* _elfcalls;

static bool use_per_thread_driver_fd = false;

void mach_driver_init(const char** applep)
{
#ifdef VARIANT_DYLD
	if (applep != NULL)
	{
		int i;
		for (i = 0; applep[i] != NULL; i++)
		{
			if (strncmp(applep[i], "kernfd=", 7) == 0)
			{
				driver_fd = __simple_atoi(applep[i] + 7, NULL);
			}
			if (strncmp(applep[i], "elf_calls=", 10) == 0)
			{
				uintptr_t table = (uintptr_t) __simple_atoi16(applep[i] + 10, NULL);
				_elfcalls = (struct elf_calls*) table;
			}
		}
	}
#else
	// Ask for fd already set up by dyld
	int (*p)(void);
	_libkernel_functions->dyld_func_lookup("__dyld_get_mach_driver_fd", (void**) &p);

	driver_fd = (*p)();

	// ask for elfcalls already set up by dyld
	void* (*p2)(void);
	_libkernel_functions->dyld_func_lookup("__dyld_get_elfcalls", (void**)&p2);

	_elfcalls = p2();
#endif

#if 0
	// If mach_driver_init() is being called in the fork child, the LKM will now
	// swap out driver_fd for a new one.
	if (__real_ioctl(driver_fd, NR_get_api_version, 0) != DARLING_MACH_API_VERSION)
	{
		const char* msg = "Darling Mach kernel module reports different API level. Aborting.\n";
		sys_write(2, msg, strlen(msg));
		sys_kill(0, 6);
	}
#endif

	// mach_driver_init() gets called at several points in application's lifetime:
	// 1) When dyld loads
	// 2) When libc initializes
	// 3) After forking

	// this was previously done when the LKM was in-use.
	// now with per-thread dserver FDs, this is more complicated to do.
	// TODO: find a suitable way of doing this.
#if 0
#ifdef VARIANT_DYLD
	struct rlimit lim;
	if (sys_getrlimit(RLIMIT_NOFILE, &lim) == 0 && driver_fd != lim.rlim_cur)
	{
		// sys_getrlimit intentionally reports a limit lower by 1
		// so that our fd remains "hidden" to applications.
		// It also means rlim_cur is not above the limit
		// in the following statement.
		int d = sys_dup2(driver_fd, lim.rlim_cur);
		close_internal(driver_fd);

		driver_fd = d;
	}
#endif
#endif
}

void mach_driver_init_pthread(void) {
	_os_tsd_set_direct(__TSD_DSERVER_RPC_FD, (void*)(intptr_t)driver_fd);
	use_per_thread_driver_fd = true;
};

__attribute__((visibility("default")))
int lkm_call(int call_nr, void* arg)
{
	__simple_printf("Something called the old LKM API (nr = %d)\n", call_nr);
	__builtin_unreachable();
}

__attribute__((visibility("default")))
int lkm_call_raw(int call_nr, void* arg)
{
	__simple_printf("Something called the old LKM API (nr = %d)\n", call_nr);
	__builtin_unreachable();
}

__attribute__((visibility("default")))
int mach_driver_get_dyld_fd(void)
{
	return driver_fd;
}

__attribute__((visibility("default")))
void mach_driver_set_dyld_fd(int fd) {
	driver_fd = fd;
};

VISIBLE
int mach_driver_get_fd(void) {
	if (use_per_thread_driver_fd) {
		return (int)(intptr_t)_os_tsd_get_direct(__TSD_DSERVER_RPC_FD);
	} else {
		return driver_fd;
	}
};

VISIBLE
void* elfcalls_get_pointer(void) {
	return _elfcalls;
};

