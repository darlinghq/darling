#ifndef LIBC_PTHREAD_H
#define LIBC_PTHREAD_H
#include <pthread.h>

#define __DARWIN_PTHREAD_MUTEX_NORMAL 0
#define __DARWIN_PTHREAD_MUTEX_ERRORCHECK 1
#define __DARWIN_PTHREAD_MUTEX_RECURSIVE 2
#define __DARWIN_PTHREAD_PROCESS_SHARED 1
#define __DARWIN_PTHREAD_PROCESS_PRIVATE 2

extern "C"
{

int __darwin_pthread_mutexattr_settype(pthread_mutexattr_t* attr, int kind);
int __darwin_pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared);
int __darwin_pthread_rwlockattr_setpshared(pthread_rwlockattr_t* attr, int pshared);

// TODO: add other pthread functions for errno translation

}

#endif
