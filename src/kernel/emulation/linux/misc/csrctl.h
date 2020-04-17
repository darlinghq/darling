#ifndef LINUX_CSRCTL_H
#define LINUX_CSRCTL_H

#include <stdint.h>
#include <stddef.h>

long sys_csrctl(uint32_t op, void* useraddr, size_t usersize);

#endif // LINUX_CSRCTL_H
