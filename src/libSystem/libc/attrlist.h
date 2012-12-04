#ifndef LIBC_ATTRLIST_H
#define LIBC_ATTRLIST_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int getattrlist(const char* path, struct attrlist* attrs, void* buf, size_t bufSize, unsigned long options);
int setattrlist(const char* path, struct attrlist* attrs, void* buf, size_t bufSize, unsigned long options);

#ifdef __cplusplus
}
#endif

#endif

