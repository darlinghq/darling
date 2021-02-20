#define __SYSCALL_32BIT_ARG_BYTES 44
#include "SYS.h"

#ifndef SYS_psynch_cvwait
#error "SYS_psynch_cvwait not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___psynch_cvwait)
SYSCALL_NONAME(psynch_cvwait, 8, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___psynch_cvwait, psynch_cvwait, 8, cerror_nocancel)
#endif

#endif

