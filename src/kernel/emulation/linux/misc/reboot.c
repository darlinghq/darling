#include "reboot.h"
#include "../base.h"
#include "../simple.h"
#include "../errno.h"
#include "../unistd/exit.h"
#include <sys/errno.h>
#include <stddef.h>

long sys_reboot(int opt, const char* cmd)
{
	__simple_printf("ALERT: The process has asked for system reboot with cmd '%s' - terminating\n", cmd);
	sys_exit(1);
	return -ENOTSUP;
}

