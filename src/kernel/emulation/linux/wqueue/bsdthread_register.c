#include "bsdthread_register.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_bsdthread_register(void* thread_start, void* wqthread, int pthsize,
		void* dummy, void* targetconc, unsigned long long dpq_offset)
{
	return 0;
}

