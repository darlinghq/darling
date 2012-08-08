#ifndef DARWIN_SYSCTL_H
#define DARWIN_SYSCTL_H

// From /usr/include/sys/sysctl.h

#define CTL_KERN 1
#define CTL_HW 6
#define KERN_OSRELEASE 2

extern "C" int __darwin_sysctl(int* name, u_int namelen, void* oldp, size_t* oldlenp, void* newp, size_t newlen)

#endif
