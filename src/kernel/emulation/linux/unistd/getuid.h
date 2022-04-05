#ifndef LINUX_GETUID_H
#define LINUX_GETUID_H

#include <stdint.h>

long sys_getuid(void);

void __getuidgid(int32_t* uid, int32_t* gid);
long __setuidgid(int32_t uid, int32_t gid);

#endif

