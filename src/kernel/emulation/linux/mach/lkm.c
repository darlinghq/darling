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


static int driver_fd = -1;

VISIBLE
struct elf_calls* _elfcalls;

void mach_driver_init(const char** applep)
{
	// DARLINGSERVER/MLDR TESTING
	__simple_printf("We're being initialized...\n");
	__builtin_unreachable();

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
#endif

	// If mach_driver_init() is being called in the fork child, the LKM will now
	// swap out driver_fd for a new one.
	if (__real_ioctl(driver_fd, NR_get_api_version, 0) != DARLING_MACH_API_VERSION)
	{
		const char* msg = "Darling Mach kernel module reports different API level. Aborting.\n";
		sys_write(2, msg, strlen(msg));
		sys_kill(0, 6);
	}

	// mach_driver_init() gets called at several points in application's lifetime:
	// 1) When dyld loads
	// 2) When libc initializes
	// 3) After forking
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
}

__attribute__((visibility("default")))
int lkm_call(int call_nr, void* arg)
{
       return __real_ioctl(driver_fd, call_nr, arg);
}

__attribute__((visibility("default")))
int lkm_call_raw(int call_nr, void* arg)
{
       return __real_ioctl_raw(driver_fd, call_nr, arg);
}

__attribute__((visibility("default")))
int mach_driver_get_fd(void)
{
	return driver_fd;
}

