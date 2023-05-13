#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_kqueue_workloop_ctl
#error "SYS_kqueue_workloop_ctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___kqueue_workloop_ctl)
ZERO_EXTEND(3)
SYSCALL_NONAME(kqueue_workloop_ctl, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___kqueue_workloop_ctl, kqueue_workloop_ctl, 4, cerror_nocancel)
#endif

#endif

