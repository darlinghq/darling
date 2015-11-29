#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_psynch_mutexdrop
#error "SYS_psynch_mutexdrop not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__psynch_mutexdrop, psynch_mutexdrop, 5, cerror_nocancel)
#endif

