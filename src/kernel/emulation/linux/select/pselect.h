#ifndef LINUX_PSELECT_H
#define LINUX_PSELECT_H
#include "../time/gettimeofday.h"
#include "../signal/duct_signals.h"

long sys_pselect(int nfds, void* rfds, void* wfds, void* efds, struct bsd_timeval* timeout, const sigset_t* mask);
long sys_pselect_nocancel(int nfds, void* rfds, void* wfds, void* efds, struct bsd_timeval* timeout, const sigset_t* mask);

#endif

