#ifndef LINUX_FSTAT64_EXTENDED_H
#define LINUX_FSTAT64_EXTENDED_H

struct stat64;

long sys_fstat64_extended(int fd, struct stat64* stat, void* xsec, unsigned long* xsec_size);

#endif

