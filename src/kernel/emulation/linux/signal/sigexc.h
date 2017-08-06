#ifndef _SIGEXC_H
#define _SIGEXC_H
#include <stdbool.h>
#include "rtsig.h"
#include "sigaction.h"

#define SIGNAL_SIGEXC_TOGGLE	LINUX_SIGRTMIN
#define SIGNAL_SIGEXC_THUPDATE	(LINUX_SIGRTMIN + 1)

#define SIGRT_MAGIC_ENABLE_SIGEXC	0xdebdeb01
#define SIGRT_MAGIC_DISABLE_SIGEXC	0xdebdeb00
#define SIGRT_

// Initializes this module
void sigexc_setup(void);

// Is this process currently traced by a debugger?
bool darling_am_i_ptraced(void);

// for PT_SIGEXC to handle this operation synchronously
void darling_sigexc_self(void);
void sigexc_handler(int linux_signum, struct linux_siginfo* info, void* ctxt);

#endif

