#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS___sigwait_nocancel
#error "SYS___sigwait_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(____sigwait_nocancel, __sigwait_nocancel, 2, cerror_nocancel)
#endif

