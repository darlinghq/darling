#ifndef DARWIN_SYSCTL_H
#define DARWIN_SYSCTL_H
#include <stdint.h>
#include <string.h>

// http://fxr.watson.org/fxr/source/bsd/sys/sysctl.h?v=xnu-1228#L373

#define CTL_KERN 1
#define CTL_HW 6
#define KERN_OSRELEASE 2
#define KERN_OSVERSION 65


extern "C" {

int __darwin_sysctl(int* name, unsigned int namelen, void* oldp, size_t* oldlenp, void* newp, size_t newlen);

int __darwin_sysctlbyname(const char* name, void* oldp, size_t* oldlenp, void* newp, size_t newlen);

}
#endif
