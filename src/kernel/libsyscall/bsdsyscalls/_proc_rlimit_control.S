#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_proc_rlimit_control
#error "SYS_proc_rlimit_control not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_proc_rlimit_control)
SYSCALL_NONAME(proc_rlimit_control, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_proc_rlimit_control, proc_rlimit_control, 3, cerror_nocancel)
#endif

#endif

