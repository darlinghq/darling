#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_audit_session_port
#error "SYS_audit_session_port not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(audit_session_port, audit_session_port, 2, cerror_nocancel)
#endif

