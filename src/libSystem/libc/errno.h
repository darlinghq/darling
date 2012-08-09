#ifndef DARWIN_ERRNO_H
#define DARWIN_ERRNO_H
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

int *__error();
int cthread_errno(void);

char* __darwin_strerror(int errnum);
int __darwin_strerror_r(int errnum, char *strerrbuf, size_t buflen);

#ifdef __cplusplus
}
#endif

int errnoDarwinToLinux(int err);
int errnoLinuxToDarwin(int err);
void errnoOut() { errno = errnoLinuxToDarwin(errno); }
void errnoIn() { errno = errnoDarwinToLinux(errno); }

#endif
