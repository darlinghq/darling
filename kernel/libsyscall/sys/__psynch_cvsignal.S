#define __SYSCALL_32BIT_ARG_BYTES 44
#include "SYS.h"

#ifndef SYS_psynch_cvsignal
#error "SYS_psynch_cvsignal not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__psynch_cvsignal, psynch_cvsignal, 8, cerror_nocancel)
#endif

