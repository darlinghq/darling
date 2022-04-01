#ifndef _SIGEXC_H
#define _SIGEXC_H
#include <stdbool.h>
#include "rtsig.h"
#include "sigaction.h"

// NOTE: Keep these definitions up to date with lkm/darling/binfmt.c!
// Uses one of the below magic values to toggle the debugging state
#define SIGNAL_SIGEXC_SUSPEND	LINUX_SIGRTMIN
#define SIGNAL_S2C (LINUX_SIGRTMIN + 1)

void sigexc_setup(void);

// for PT_SIGEXC to handle this operation synchronously
void darling_sigexc_self(void);
void sigexc_handler(int linux_signum, struct linux_siginfo* info, struct linux_ucontext* ctxt);

void sigexc_thread_setup(void);
void sigexc_thread_exit(void);

#endif

