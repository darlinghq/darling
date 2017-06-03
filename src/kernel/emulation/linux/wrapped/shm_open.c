#include "shm_open.h"
#include "../base.h"
#include "../errno.h"
#include "../fcntl/open.h"
#include <sys/errno.h>
#include <linux-syscalls/linux.h>
#include <elfcalls.h>

extern struct elf_calls* _elfcalls;

long sys_shm_open(const char* name, int oflag, int mode)
{
#ifndef VARIANT_DYLD
	int ret;

	ret = _elfcalls->shm_open(name, oflags_bsd_to_linux(oflag), mode);
	if (ret == -1)
		ret = -errno_linux_to_bsd(_elfcalls->get_errno());

	return ret;
#else
	return -ENOSYS;
#endif
}

