#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_sigsuspend_nocancel
#error "SYS_sigsuspend_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__sigsuspend_nocancel, sigsuspend_nocancel, 1, cerror_nocancel)
#endif

