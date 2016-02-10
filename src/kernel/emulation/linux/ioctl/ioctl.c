#include "ioctl.h"
#include "../base.h"
#include "../../../../../platform-include/sys/errno.h"
#include <stddef.h>
#include <stdbool.h>
#include "termios.h"
#include "filio.h"

#define IOCTL_STEP(x) { int state, retval; state = (x); \
	if (state == IOCTL_HANDLED) return retval; }

// Emulated ioctl implementation
VISIBLE
long ioctl(int fd, int cmd, void* arg)
{
	IOCTL_STEP(handle_filio(fd, cmd, arg, &retval));
	IOCTL_STEP(handle_termios(fd, cmd, arg, &retval));
	
run_ioctl:
	return __real_ioctl(fd, cmd, arg);
}



