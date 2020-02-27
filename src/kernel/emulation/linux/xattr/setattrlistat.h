#ifndef LINUX_SETATTRLISTAT_H
#define LINUX_SETATTRLISTAT_H
#include "getattrlist.h"

long sys_setattrlistat(int fd, const char* path, struct attrlist* alist, void *attributeBuffer, __SIZE_TYPE__ bufferSize, unsigned long options);

#endif

