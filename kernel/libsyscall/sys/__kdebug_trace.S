#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_kdebug_trace
#error "SYS_kdebug_trace not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__kdebug_trace, kdebug_trace, 6, cerror_nocancel)
#endif

