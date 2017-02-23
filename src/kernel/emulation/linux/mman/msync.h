#ifndef LINUX_MSYNC_H
#define LINUX_MSYNC_H

long sys_msync(void* addr, unsigned long len, int flags);
long sys_msync_nocancel(void* addr, unsigned long len, int flags);

#define LINUX_MS_ASYNC	1
#define LINUX_MS_INVALIDATE	2
#define LINUX_MS_SYNC	4

#define BSD_MS_ASYNC	1
#define BSD_MS_INVALIDATE	2
#define BSD_MS_SYNC		0x10

#endif

