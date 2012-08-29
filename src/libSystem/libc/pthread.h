#ifndef LIBC_PTHREAD_H
#define LIBC_PTHREAD_H
#include <pthread.h>
#include <stdint.h>

#define __DARWIN_PTHREAD_MUTEX_NORMAL 0
#define __DARWIN_PTHREAD_MUTEX_ERRORCHECK 1
#define __DARWIN_PTHREAD_MUTEX_RECURSIVE 2
#define __DARWIN_PTHREAD_PROCESS_SHARED 1
#define __DARWIN_PTHREAD_PROCESS_PRIVATE 2

struct __darwin_pthread_rwlock_t
{
	enum { SIGNATURE_MACRO_INITIALIZED = 0x2da8b3b4, SIGNATURE_NATIVE_INITIALIZED = 0x12345678 };
	uint32_t signature;
	pthread_rwlock_t native; // this will fit on Linux
};

extern "C"
{

int __darwin_pthread_mutexattr_settype(pthread_mutexattr_t* attr, int kind);
int __darwin_pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared);

// pthread_rwlock
int __darwin_pthread_rwlockattr_setpshared(pthread_rwlockattr_t* attr, int pshared);
int __darwin_pthread_rwlock_init(__darwin_pthread_rwlock_t* rwlock, const pthread_rwlockattr_t* attr);
int __darwin_pthread_rwlock_destroy(__darwin_pthread_rwlock_t *rwlock);
int __darwin_pthread_rwlock_rdlock(__darwin_pthread_rwlock_t *rwlock);
int __darwin_pthread_rwlock_tryrdlock(__darwin_pthread_rwlock_t *rwlock);
int __darwin_pthread_rwlock_trywrlock(__darwin_pthread_rwlock_t *rwlock);
int __darwin_pthread_rwlock_wrlock(__darwin_pthread_rwlock_t *rwlock);
int __darwin_pthread_rwlock_timedrdlock(__darwin_pthread_rwlock_t* rwlock, const struct timespec* abs_timeout);
int __darwin_pthread_rwlock_unlock(__darwin_pthread_rwlock_t *rwlock);

// TODO: add other pthread functions for errno translation

}

#endif
