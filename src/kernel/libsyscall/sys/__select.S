#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_select
#error "SYS_select not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__select, select, 5, cerror)
#endif

#if defined(__x86_64__)
	.globl	select$DARWIN_EXTSN
		select$DARWIN_EXTSN = __select
#endif

