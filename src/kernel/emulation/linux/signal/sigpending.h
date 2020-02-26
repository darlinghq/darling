#ifndef LINUX_SIGPENDING_H
#define LINUX_SIGPENDING_H
#include "duct_signals.h"

long sys_sigpending(sigset_t set);

#endif

