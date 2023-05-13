#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_pselect_nocancel
#error "SYS_pselect_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___pselect_nocancel)
SYSCALL_NONAME(pselect_nocancel, 6, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___pselect_nocancel, pselect_nocancel, 6, cerror_nocancel)
#endif

#endif

