#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_select_nocancel
#error "SYS_select_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___select_nocancel)
SYSCALL_NONAME(select_nocancel, 5, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___select_nocancel, select_nocancel, 5, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_select$DARWIN_EXTSN$NOCANCEL
	.set	_select$DARWIN_EXTSN$NOCANCEL, ___select_nocancel
#endif

#if defined(__x86_64__)
	.globl	_select$DARWIN_EXTSN$NOCANCEL
	.set	_select$DARWIN_EXTSN$NOCANCEL, ___select_nocancel
#endif

#if defined(__ppc__)
	.globl	_select$DARWIN_EXTSN$NOCANCEL
	.set	_select$DARWIN_EXTSN$NOCANCEL, ___select_nocancel
#endif

#if defined(__arm__)
	.globl	_select$DARWIN_EXTSN$NOCANCEL
	.set	_select$DARWIN_EXTSN$NOCANCEL, ___select_nocancel
#endif

#if defined(__arm64__)
	.globl	_select$DARWIN_EXTSN$NOCANCEL
	.set	_select$DARWIN_EXTSN$NOCANCEL, ___select_nocancel
#endif

