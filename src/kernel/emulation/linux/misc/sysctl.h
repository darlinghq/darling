#ifndef LINUX_SYSCTL_H
#define LINUX_SYSCTL_H

long sys_sysctl(int* name, unsigned int nlen, void* old, unsigned long* oldlen, void* _new, unsigned long newlen);
long sys_sysctlbyname(const char* name, unsigned long namelen, void* old, unsigned long* oldlen, void* _new, unsigned long newlen);

#endif

