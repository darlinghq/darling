#ifndef _SIGEXC_H
#define _SIGEXC_H
#include <stdbool.h>
#include "rtsig.h"
#include "sigaction.h"

// Uses one of the below magic values to toggle the debugging state
#define SIGNAL_SIGEXC_TOGGLE	LINUX_SIGRTMIN

// A BSD signal number is passed as value
#define SIGNAL_SIGEXC_THUPDATE	(LINUX_SIGRTMIN + 1)

#define SIGRT_MAGIC_ENABLE_SIGEXC	0xdebdeb01
#define SIGRT_MAGIC_DISABLE_SIGEXC	0xdebdeb00

// Initializes this module (before opening LKM)
void sigexc_setup1(void);
// Finish initialization (after opening LKM)
void sigexc_setup2(void);

// Is this process currently traced by a debugger?
bool darling_am_i_ptraced(void);

// for PT_SIGEXC to handle this operation synchronously
void darling_sigexc_self(void);
void sigexc_handler(int linux_signum, struct linux_siginfo* info, struct linux_ucontext* ctxt);

int linux_sigqueue(int pid, int rtsig, int value);
int linux_sigqueue_thread(int pid, int tid, int rtsig, int value);

#endif

