#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_pselect
#error "SYS_pselect not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___pselect)
SYSCALL_NONAME(pselect, 6, cerror)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___pselect, pselect, 6, cerror)
#endif

#endif

