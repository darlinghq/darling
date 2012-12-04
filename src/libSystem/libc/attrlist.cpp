#include "attrlist.h"
#include "darwin_errno_codes.h"
#include <errno.h>

int getattrlist(const char* path, struct attrlist* attrs, void* buf, size_t bufSize, unsigned long options)
{
	errno = DARWIN_ENOTSUP;
	return -1;
}

int setattrlist(const char* path, struct attrlist* attrs, void* buf, size_t bufSize, unsigned long options)
{
	errno = DARWIN_ENOTSUP;
	return -1;
}

