#define __SYSCALL_32BIT_ARG_BYTES 32
#include "SYS.h"

#ifndef SYS_kevent_qos
#error "SYS_kevent_qos not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_kevent_qos)
SYSCALL_NONAME(kevent_qos, 8, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_kevent_qos, kevent_qos, 8, cerror_nocancel)
#endif

#endif

