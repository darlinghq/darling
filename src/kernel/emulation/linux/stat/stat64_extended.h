#ifndef LINUX_STAT64_EXTENDED_H
#define LINUX_STAT64_EXTENDED_H

struct stat64;

long sys_stat64_extended(const char* path, struct stat64* stat, void* xsec, unsigned long* xsec_size);

#endif

