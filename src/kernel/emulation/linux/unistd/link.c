#include "link.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "linkat.h"
#include "../bsdthread/per_thread_wd.h"

long sys_link(const char* path, const char* link)
{
	int fd = get_perthread_wd();
	return sys_linkat(fd, path, fd, link, 0);
}

