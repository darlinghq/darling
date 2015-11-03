#ifndef LINUX_LSTAT64_EXTENDED_H
#define LINUX_LSTAT64_EXTENDED_H

struct stat64;

long sys_lstat64_extended(const char* path, struct stat64* stat, void* xsec, unsigned long* xsec_size);

#endif

