#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_kqueue_workloop_ctl
#error "SYS_kqueue_workloop_ctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___kqueue_workloop_ctl, kqueue_workloop_ctl, 4, cerror_nocancel)
#endif

