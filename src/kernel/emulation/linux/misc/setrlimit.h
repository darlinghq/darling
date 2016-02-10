#ifndef LINUX_SETRLIMIT_H
#define LINUX_SETRLIMIT_H
#include "getrlimit.h"

long sys_setrlimit(unsigned int which, const struct rlimit* rlp);

#endif

