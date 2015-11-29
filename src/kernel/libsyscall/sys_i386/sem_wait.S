#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#if defined(__i386__)
	.globl	sem_wait$UNIX2003
		sem_wait$UNIX2003 = __sem_wait
#endif

