#ifndef _ASM_GENERAL_UNISTD_H
#define _ASM_GENERAL_UNISTD_H

# ifdef __i386__
//  # include <asm/unistd_32.h>
    # include "linux-x86.h"
# elif defined(__x86_64__)
//  # include <asm/unistd_64.h>
    # include "linux-x86_64.h"
# elif defined(__arm64__)
//  # include <asm/unistd.h>
    # include "linux-arm64.h"
# else
//  look in <asm/unistd.h> if you are unsure
    # error Missing Linux sc numbers for this arch
# endif

#endif /* _ASM_GENERAL_UNISTD_H */
