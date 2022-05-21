#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_chmod_extended
#error "SYS_chmod_extended not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___chmod_extended)
SYSCALL_NONAME(chmod_extended, 5, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___chmod_extended, chmod_extended, 5, cerror_nocancel)
#endif

#endif

