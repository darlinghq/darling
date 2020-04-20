#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_bsdthread_ctl
#error "SYS_bsdthread_ctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___bsdthread_ctl, bsdthread_ctl, 4, cerror_nocancel)
#endif

