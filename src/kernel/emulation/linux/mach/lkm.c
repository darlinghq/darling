#include "lkm.h"
#include "../../lkm/api.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include "../../libsyscall/wrappers/_libkernel_init.h"

int driver_fd = -1;

extern int __real_ioctl(int fd, int cmd, void* arg);
extern int sys_open(const char*, int, int);
extern int sys_close(int);
extern int sys_write(int, const void*, int);
extern int sys_kill(int, int);
extern int sys_getrlimit(int, struct rlimit*);
extern int sys_dup2(int, int);
extern int sys_fcntl(int, int, int);
extern _libkernel_functions_t _libkernel_functions;

void mach_driver_init(void)
{
	struct rlimit lim;

	if (driver_fd != -1)
		sys_close(driver_fd);
#ifndef VARIANT_DYLD
	else
	{
		// Ask for fd already set up by dyld
		int (*p)(void);
		_libkernel_functions->dyld_func_lookup("__dyld_get_mach_driver_fd", &p);

		driver_fd = (*p)();

		if (driver_fd != -1)
			return;
	}
#endif

	driver_fd = sys_open("/dev/mach", O_RDWR | O_CLOEXEC, 0);
	if (driver_fd < 0)
	{
		const char* msg = "Cannot open /dev/mach. Aborting.\nMake sure you have loaded the darling-mach kernel module.\n";

		sys_write(2, msg, strlen(msg));
		sys_kill(0, 6);
	}

	if (__real_ioctl(driver_fd, NR_get_api_version, 0) != DARLING_MACH_API_VERSION)
	{
		const char* msg = "Darling Mach kernel module reports different API level. Aborting.\n";

		sys_write(2, msg, strlen(msg));
		sys_kill(0, 6);
	}

	if (sys_getrlimit(RLIMIT_NOFILE, &lim) == 0)
	{
		// sys_getrlimit intentionally reports a limit lower by 1
		// so that our fd remains "hidden" to applications.
		// It also means rlim_cur is not above the limit
		// in the following statement.
		int d = sys_dup2(driver_fd, lim.rlim_cur);
		sys_close(driver_fd);
		
		driver_fd = d;
		sys_fcntl(driver_fd, F_SETFD, FD_CLOEXEC);
	}
}

__attribute__((visibility("default")))
int lkm_call(int call_nr, void* arg)
{
	return __real_ioctl(driver_fd, call_nr, arg);
}

int mach_driver_get_fd(void)
{
	return driver_fd;
}
