#define __SYSCALL_32BIT_ARG_BYTES 44
#include "SYS.h"

#ifndef SYS_psynch_cvwait
#error "SYS_psynch_cvwait not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___psynch_cvwait, psynch_cvwait, 8, cerror_nocancel)
#endif

