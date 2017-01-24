#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_audit_session_join
#error "SYS_audit_session_join not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_audit_session_join, audit_session_join, 1, cerror_nocancel)
#endif

