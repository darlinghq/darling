#ifndef LINUX_CSOPS_H
#define LINUX_CSOPS_H

#include <stddef.h>
#include <mach/message.h>

long sys_csops(int pid, unsigned int ops, void* useraddr, size_t usersize);
long sys_csops_audittoken(int pid, unsigned int ops, void* useraddr, size_t usersize, audit_token_t* audit_token);

#endif // LINUX_CSOPS_H
