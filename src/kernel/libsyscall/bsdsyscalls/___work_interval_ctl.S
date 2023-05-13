#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_work_interval_ctl
#error "SYS_work_interval_ctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___work_interval_ctl)
ZERO_EXTEND(3)
SYSCALL_NONAME(work_interval_ctl, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___work_interval_ctl, work_interval_ctl, 4, cerror_nocancel)
#endif

#endif

