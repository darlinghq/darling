#ifndef LIBC_PTHREAD_H
#define LIBC_PTHREAD_H
#include <pthread.h>
#include <stdint.h>

#define __DARWIN_PTHREAD_MUTEX_NORMAL 0
#define __DARWIN_PTHREAD_MUTEX_ERRORCHECK 1
#define __DARWIN_PTHREAD_MUTEX_RECURSIVE 2
#define __DARWIN_PTHREAD_PROCESS_SHARED 1
#define __DARWIN_PTHREAD_PROCESS_PRIVATE 2
#define __DARWIN_PTHREAD_CREATE_JOINABLE 1
#define __DARWIN_PTHREAD_CREATE_DETACHED 2

struct __darwin_pthread_rwlock_t // 200 bytes on Darwin
{
	enum { SIGNATURE_MACRO_INITIALIZED = 0x2da8b3b4, SIGNATURE_NATIVE_INITIALIZED = 0x12345678 };
	uint32_t signature;
	pthread_rwlock_t native; // this will fit on Linux
};

struct __darwin_pthread_mutex_t
{
	enum { SIGNATURE_MACRO_INITIALIZED = 0x32aaaba7, SIGNATURE_MACRO_INITIALIZED_R = 0x32aaaba2, SIGNATURE_MACRO_INITIALIZED_E = 0x32aaaba1, SIGNATURE_NATIVE_INITIALIZED = 0x12345678 };
	uint32_t signature;
	pthread_mutex_t native; // this will fit on Linux (40 vs 64)
};

struct __darwin_pthread_cond_t
{
	enum { SIGNATURE_MACRO_INITIALIZED = 0x3cb0b1bb, SIGNATURE_NATIVE_INITIALIZED = 0x12345678 };
	uint32_t signature;
	pthread_cond_t *native; // this will NOT fit on linux (48 vs 44) (allocated instead)
};

struct __darwin_pthread_once_t
{
	enum { SIGNATURE_MACRO_INITIALIZED = 0x30b1bcba };
	uint32_t signature;
	union
	{
		uint32_t data[2];
		pthread_once_t native;
	};
};

static_assert(sizeof(pthread_mutex_t) <= 60, "pthread_mutex_t is too big on this platform!");
static_assert(sizeof(pthread_rwlock_t) <= 196, "pthread_rwlock_t is too big on this platform!");

extern "C"
{

int __darwin_pthread_once(__darwin_pthread_once_t *once_control, void (*init_routine)(void));

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

// pthread_mutex
int __darwin_pthread_mutex_init(__darwin_pthread_mutex_t*, const pthread_mutexattr_t* attr);
int __darwin_pthread_mutex_destroy(__darwin_pthread_mutex_t* mutex);
int __darwin_pthread_mutex_lock(__darwin_pthread_mutex_t* mutex);
int __darwin_pthread_mutex_trylock(__darwin_pthread_mutex_t* mutex);
int __darwin_pthread_mutex_unlock(__darwin_pthread_mutex_t* mutex);

// pthread_cond
int __darwin_pthread_cond_init(__darwin_pthread_cond_t *cond, const pthread_condattr_t *attr);
int __darwin_pthread_cond_destroy(__darwin_pthread_cond_t *cond);
int __darwin_pthread_cond_signal(__darwin_pthread_cond_t *cond);
int __darwin_pthread_cond_broadcast(__darwin_pthread_cond_t *cond);
int __darwin_pthread_cond_timedwait(__darwin_pthread_cond_t *cond, __darwin_pthread_mutex_t* mutex, const struct timespec *abstime);
int __darwin_pthread_cond_wait(__darwin_pthread_cond_t *cond, __darwin_pthread_mutex_t* mutex);

pid_t __darwin_pthread_mach_thread_np(pthread_t pth);

size_t pthread_get_stacksize_np(pthread_t pth);
void* pthread_get_stackaddr_np(pthread_t pth);

int __darwin_pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int __darwin_pthread_attr_getdetachstate(pthread_attr_t *attr, int *detachstate);

// TODO: add other pthread functions for errno translation

}

#endif
