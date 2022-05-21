#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_sysctlbyname
#error "SYS_sysctlbyname not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___sysctlbyname)
ZERO_EXTEND(1)
ZERO_EXTEND(5)
SYSCALL_NONAME(sysctlbyname, 6, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___sysctlbyname, sysctlbyname, 6, cerror_nocancel)
#endif

#endif

