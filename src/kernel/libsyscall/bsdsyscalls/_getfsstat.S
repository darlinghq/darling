#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_getfsstat
#error "SYS_getfsstat not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__)
__SYSCALL2(_getfsstat, getfsstat, 3, cerror_nocancel)
#else
__SYSCALL2(___getfsstat, getfsstat, 3, cerror_nocancel)
#endif

