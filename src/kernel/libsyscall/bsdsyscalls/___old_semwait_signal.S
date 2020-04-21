#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS___old_semwait_signal
#error "SYS___old_semwait_signal not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___old_semwait_signal, __old_semwait_signal, 5, cerror_nocancel)
#endif

