#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_fgetattrlist
#error "SYS_fgetattrlist not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_fgetattrlist)
ZERO_EXTEND(3)
ZERO_EXTEND(4)
SYSCALL_NONAME(fgetattrlist, 5, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_fgetattrlist, fgetattrlist, 5, cerror_nocancel)
#endif

#endif

