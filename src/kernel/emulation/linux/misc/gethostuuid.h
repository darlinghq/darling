#ifndef LINUX_GETHOSTUUID_H
#define LINUX_GETHOSTUUID_H

long sys_gethostuuid(unsigned char* uuid_buf, void* timeout);

#endif

