#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_kdebug_trace_string
#error "SYS_kdebug_trace_string not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___kdebug_trace_string, kdebug_trace_string, 3, cerror_nocancel)
#endif

