#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_change_fdguard_np
#error "SYS_change_fdguard_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_change_fdguard_np, change_fdguard_np, 6, cerror_nocancel)
#endif

