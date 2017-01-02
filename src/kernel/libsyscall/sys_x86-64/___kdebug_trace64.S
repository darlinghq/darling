#define __SYSCALL_32BIT_ARG_BYTES 36
#include "SYS.h"

#ifndef SYS_kdebug_trace64
#error "SYS_kdebug_trace64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___kdebug_trace64, kdebug_trace64, 5, cerror_nocancel)
#endif

