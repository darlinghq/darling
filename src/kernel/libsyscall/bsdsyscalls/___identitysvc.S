#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_identitysvc
#error "SYS_identitysvc not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___identitysvc)
SYSCALL_NONAME(identitysvc, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___identitysvc, identitysvc, 2, cerror_nocancel)
#endif

#endif

