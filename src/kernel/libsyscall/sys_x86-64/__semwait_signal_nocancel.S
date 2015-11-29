#define __SYSCALL_32BIT_ARG_BYTES 28
#include "SYS.h"

#ifndef SYS___semwait_signal_nocancel
#error "SYS___semwait_signal_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__semwait_signal_nocancel, __semwait_signal_nocancel, 6, cerror_nocancel)
#endif

