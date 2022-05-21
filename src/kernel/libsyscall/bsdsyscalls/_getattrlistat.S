#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_getattrlistat
#error "SYS_getattrlistat not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_getattrlistat)
ZERO_EXTEND(5)
ZERO_EXTEND(4)
SYSCALL_NONAME(getattrlistat, 6, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_getattrlistat, getattrlistat, 6, cerror_nocancel)
#endif

#endif

