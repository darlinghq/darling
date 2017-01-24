#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_csops_audittoken
#error "SYS_csops_audittoken not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_csops_audittoken, csops_audittoken, 5, cerror_nocancel)
#endif

