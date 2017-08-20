#include "sem_wait.h"
#include "../base.h"
#include "../errno.h"
#include <sys/errno.h>
#include <linux-syscalls/linux.h>
#include <elfcalls.h>
#include "../bsdthread/cancelable.h"

extern struct elf_calls* _elfcalls;

long sys_sem_wait(int* sem)
{
	CANCELATION_POINT();
	return sys_sem_wait_nocancel(sem);
}

long sys_sem_wait_nocancel(int* sem)
{
#ifndef VARIANT_DYLD
	int ret;

	ret = _elfcalls->sem_wait(sem);
	if (ret == -1)
		ret = -errno_linux_to_bsd(_elfcalls->get_errno());

	return ret;
#else
	return -ENOSYS;
#endif
}

