#ifndef LINUX_SIGSUSPEND_H
#define LINUX_SIGSUSPEND_H
#include "duct_signals.h"

long sys_sigsuspend(sigset_t set);
long sys_sigsuspend_nocancel(sigset_t set);

#endif

