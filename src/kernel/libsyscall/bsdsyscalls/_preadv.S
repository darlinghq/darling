#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_preadv
#error "SYS_preadv not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_preadv)
SYSCALL_NONAME(preadv, 4, cerror)
ret
#else
#if defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_preadv, preadv, 4, cerror)
#else
__SYSCALL2(___preadv, preadv, 4, cerror)
#endif

#endif

