#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_reboot
#error "SYS_reboot not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___reboot)
SYSCALL_NONAME(reboot, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___reboot, reboot, 2, cerror_nocancel)
#endif

#endif

