#ifndef _ASM_X86_UNISTD_H
#define _ASM_X86_UNISTD_H

# ifdef __i386__
#  include "linux-x86.h"
# elif defined(__x86_64__)
#  include "linux-x86_64.h"
# else
#  error Missing Linux sc numbers for this arch
# endif

#endif /* _ASM_X86_UNISTD_H */
