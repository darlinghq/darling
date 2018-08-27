#ifndef LINUX_SIGWAIT_H
#define LINUX_SIGWAIT_H
#include "duct_signals.h"

long sys_sigwait(sigset_t set, int* signal);
long sys_sigwait_nocancel(sigset_t set, int* signal);

#endif

