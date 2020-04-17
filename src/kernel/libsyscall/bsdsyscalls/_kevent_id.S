#define __SYSCALL_32BIT_ARG_BYTES 36
#include "SYS.h"

#ifndef SYS_kevent_id
#error "SYS_kevent_id not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_kevent_id, kevent_id, 8, cerror_nocancel)
#endif

