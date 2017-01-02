#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_mremap_encrypted
#error "SYS_mremap_encrypted not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_mremap_encrypted, mremap_encrypted, 5, cerror_nocancel)
#endif

