#ifndef DARWIN_ERRNO_H
#define DARWIN_ERRNO_H

extern "C"
{
// int *__error(); /* generated code */
int cthread_errno();

char* __darwin_strerror(int errnum);
int __darwin_strerror_r(int errnum, char *strerrbuf, size_t buflen);

int errnoDarwinToLinux(int err);
int errnoLinuxToDarwin(int err);

}

#endif
