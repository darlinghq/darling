#ifndef LIBC_MOUNT_H
#define LIBC_MOUNT_H
#include "kernel-bsd/fs.h"

extern "C"
{

int __darwin_getmntinfo(struct __darwin_statfs** mntbufp, int flags);
int __darwin_getmntinfo64(struct __darwin_statfs64** mntbufp, int flags) asm("__darwin_getmntinfo$INODE64");

}

#endif
