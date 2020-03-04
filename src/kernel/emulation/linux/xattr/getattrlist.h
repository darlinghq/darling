#ifndef LINUX_GETATTRLIST_H
#define LINUX_GETATTRLIST_H
#include <stdint.h>

typedef uint32_t attrgroup_t;

struct attrlist {
	uint16_t     bitmapcount;
	uint16_t   reserved;
	attrgroup_t commonattr;
	attrgroup_t volattr;
	attrgroup_t dirattr;
	attrgroup_t fileattr;
	attrgroup_t forkattr;
};

long sys_getattrlist(const char* path, struct attrlist* alist, void *attributeBuffer, __SIZE_TYPE__ bufferSize, unsigned long options);

#endif

