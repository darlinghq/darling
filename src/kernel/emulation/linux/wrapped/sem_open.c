#include "sem_open.h"
#include "../base.h"
#include "../errno.h"
#include "../fcntl/open.h"
#include <sys/errno.h>
#include <linux-syscalls/linux.h>
#include <elfcalls.h>

extern struct elf_calls* elfcalls(void);

long sys_sem_open(const char* name, int oflag, int mode, int value)
{
#ifndef VARIANT_DYLD
	int ret;
	int* ptr;
	
	// __simple_printf("sem_open %s, %d, %d, %d\n", name, oflag, mode, value);
	
	ptr = elfcalls()->sem_open(name, oflags_bsd_to_linux(oflag), mode, value);
	//__simple_printf("sem_open -> %p\n", ptr);
	
	if (!ptr)
	{
		// __simple_printf("errno: %d\n", elfcalls()->get_errno());
		return -errno_linux_to_bsd(elfcalls()->get_errno());
	}

	return (long) ptr;
#else
	return -ENOSYS;
#endif
}

