#include "lkm.h"
#include "../../lkm/api.h"
#include <fcntl.h>
#include <unistd.h>

int driver_fd = -1;

extern int __real_ioctl(int fd, int cmd, void* arg);

void mach_driver_init(void)
{
	int new_driver_fd;

	if (driver_fd != -1)
		close(driver_fd);

	new_driver_fd = open("/dev/mach", O_RDWR | O_CLOEXEC);
	if (new_driver_fd == -1)
	{
		const char* msg = "Cannot open /dev/mach. Aborting.\nMake sure you have loaded the darling-mach kernel module.\n";

		write(2, msg, strlen(msg));
		abort();
	}

	// Ensure we get the same fd number, even if lower ones are available
	if (driver_fd != -1 && driver_fd != new_driver_fd)
	{
		dup2(new_driver_fd, driver_fd);
		close(new_driver_fd);
	}
	else
		driver_fd = new_driver_fd;

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
