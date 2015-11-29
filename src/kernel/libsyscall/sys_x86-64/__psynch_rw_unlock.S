#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_psynch_rw_unlock
#error "SYS_psynch_rw_unlock not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__psynch_rw_unlock, psynch_rw_unlock, 5, cerror_nocancel)
#endif

