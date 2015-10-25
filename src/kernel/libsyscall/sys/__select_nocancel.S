#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_select_nocancel
#error "SYS_select_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__select_nocancel, select_nocancel, 5, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	select$DARWIN_EXTSN$NOCANCEL
		select$DARWIN_EXTSN$NOCANCEL = __select_nocancel
#endif

