#ifndef LINUX_SETATTRLIST_H
#define LINUX_SETATTRLIST_H
#include "getattrlist.h"

long sys_setattrlist(const char* path, struct attrlist* alist, void *attributeBuffer, __SIZE_TYPE__ bufferSize, unsigned long options);

#endif

