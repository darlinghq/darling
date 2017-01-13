#include "mremap.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

VISIBLE
int __linux_mremap(void* old_addr, unsigned long old_size, unsigned long new_size, int flags, void* new_address)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_mremap, old_addr, old_size, new_size, flags, new_address);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

