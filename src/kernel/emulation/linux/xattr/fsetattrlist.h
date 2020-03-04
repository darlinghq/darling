#ifndef LINUX_FSETATTRLIST_H
#define LINUX_FSETATTRLIST_H
#include "getattrlist.h"

long sys_fsetattrlist(int fd, struct attrlist* alist, void *attributeBuffer, __SIZE_TYPE__ bufferSize, unsigned long options);

#endif

