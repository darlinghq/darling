#include "lkm.h"
#include "../../lkm/api.h"
#include <fcntl.h>
#include <unistd.h>

int driver_fd = -1;

extern int __real_ioctl(int fd, int cmd, void* arg);

void mach_driver_init(void)
{
	if (driver_fd != -1)
		close(driver_fd);

	driver_fd = open("/dev/mach", O_RDWR);
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

// Emulated ioctl implementation
int ioctl(int fd, int cmd, void* arg)
{
	struct bsd_ioctl_args args = {
		.fd = fd,
		.request = cmd,
		.arg = arg
	};

	return __real_ioctl(driver_fd, NR_bsd_ioctl_trap, &args);
}

