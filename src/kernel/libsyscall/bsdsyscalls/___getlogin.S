#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_getlogin
#error "SYS_getlogin not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___getlogin)
SYSCALL_NONAME(getlogin, 2, cerror)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___getlogin, getlogin, 2, cerror)
#endif

#endif

