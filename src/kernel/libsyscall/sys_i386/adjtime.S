#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_adjtime
#error "SYS_adjtime not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(adjtime, adjtime, 2, cerror_nocancel)
#endif

