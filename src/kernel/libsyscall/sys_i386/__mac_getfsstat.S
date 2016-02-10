#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS___mac_getfsstat
#error "SYS___mac_getfsstat not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__mac_getfsstat, __mac_getfsstat, 5, cerror_nocancel)
#endif

