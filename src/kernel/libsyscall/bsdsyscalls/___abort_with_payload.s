#define __SYSCALL_32BIT_ARG_BYTES 32
#include "SYS.h"

#ifndef SYS_abort_with_payload
#error "SYS_abort_with_payload not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___abort_with_payload)
SYSCALL_NONAME(abort_with_payload, 6, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___abort_with_payload, abort_with_payload, 6, cerror_nocancel)
#endif

#endif

