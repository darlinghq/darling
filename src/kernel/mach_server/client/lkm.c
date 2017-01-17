#include "lkm.h"
#include "../../lkm/api.h"
#include <fcntl.h>
#include <unistd.h>
#include "../../libsyscall/wrappers/_libkernel_init.h"

int driver_fd = -1;

extern int __real_ioctl(int fd, int cmd, void* arg);
extern _libkernel_functions_t _libkernel_functions;

void mach_driver_init(void)
{
	if (driver_fd != -1)
		close(driver_fd);
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

	driver_fd = open("/dev/mach", O_RDWR | O_CLOEXEC);
	if (driver_fd == -1)
	{
		const char* msg = "Cannot open /dev/mach. Aborting.\nMake sure you have loaded the darling-mach kernel module.\n";

		write(2, msg, strlen(msg));
		abort();
	}

	if (__real_ioctl(driver_fd, NR_get_api_version, 0) != DARLING_MACH_API_VERSION)
	{
		const char* msg = "Darling Mach kernel module reports different API level. Aborting.\n";

		write(2, msg, strlen(msg));
		abort();
	}
}

int lkm_call(int call_nr, void* arg)
{
	return __real_ioctl(driver_fd, call_nr, arg);
}

int mach_driver_get_fd(void)
{
	return driver_fd;
}
