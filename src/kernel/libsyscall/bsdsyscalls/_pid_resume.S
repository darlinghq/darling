#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_pid_resume
#error "SYS_pid_resume not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_pid_resume)
SYSCALL_NONAME(pid_resume, 1, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_pid_resume, pid_resume, 1, cerror_nocancel)
#endif

#endif

