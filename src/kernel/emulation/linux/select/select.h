#ifndef LINUX_SELECT_H
#define LINUX_SELECT_H
#include "../time/gettimeofday.h"

long sys_select(int nfds, void* rfds, void* wfds, void* efds, struct bsd_timeval* timeout);
long sys_select_nocancel(int nfds, void* rfds, void* wfds, void* efds, struct bsd_timeval* timeout);

#endif

