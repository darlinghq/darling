#define __SYSCALL_32BIT_ARG_BYTES 36
#include "SYS.h"

#ifndef SYS_terminate_with_payload
#error "SYS_terminate_with_payload not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___terminate_with_payload)
SYSCALL_NONAME(terminate_with_payload, 7, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___terminate_with_payload, terminate_with_payload, 7, cerror_nocancel)
#endif

#endif

