#include "../base.h"
#include <asm/unistd.h>

void __linux_sched_yield(void)
{
	LINUX_SYSCALL0(__NR_sched_yield);
}

