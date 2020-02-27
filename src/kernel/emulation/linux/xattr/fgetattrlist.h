#ifndef LINUX_FGETATTRLIST_H
#define LINUX_FGETATTRLIST_H
#include "getattrlist.h"

long sys_fgetattrlist(int fd, struct attrlist* alist, void *attributeBuffer, __SIZE_TYPE__ bufferSize, unsigned long options);

#endif

