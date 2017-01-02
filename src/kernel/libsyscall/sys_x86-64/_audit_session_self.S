#define __SYSCALL_32BIT_ARG_BYTES 0
#include "SYS.h"

#ifndef SYS_audit_session_self
#error "SYS_audit_session_self not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_audit_session_self, audit_session_self, 0, cerror_nocancel)
#endif

