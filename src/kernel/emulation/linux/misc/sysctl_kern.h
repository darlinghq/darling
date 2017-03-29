#ifndef _SYSCTL_KERN_H
#define _SYSCTL_KERN_H
#include "sysctl.h"

extern const struct known_sysctl sysctls_kern[];

struct linux_utsname* need_uname(void);

#endif

