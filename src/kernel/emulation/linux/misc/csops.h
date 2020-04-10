#ifndef LINUX_CSOPS_H
#define LINUX_CSOPS_H

#include <stddef.h>

long sys_csops(int pid, unsigned int ops, void* useraddr, size_t usersize);

#endif // LINUX_CSOPS_H
