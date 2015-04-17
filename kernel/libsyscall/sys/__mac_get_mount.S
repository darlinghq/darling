#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS___mac_get_mount
#error "SYS___mac_get_mount not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__mac_get_mount, __mac_get_mount, 2, cerror_nocancel)
#endif

