#define __SYSCALL_32BIT_ARG_BYTES 28
#include "SYS.h"

#ifndef SYS___semwait_signal
#error "SYS___semwait_signal not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___semwait_signal, __semwait_signal, 6, cerror)
#endif

