#define __SYSCALL_32BIT_ARG_BYTES 0
#include "SYS.h"

#ifndef SYS_workq_open
#error "SYS_workq_open not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___workq_open)
SYSCALL_NONAME(workq_open, 0, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___workq_open, workq_open, 0, cerror_nocancel)
#endif

#endif

