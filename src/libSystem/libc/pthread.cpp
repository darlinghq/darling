#include "pthread.h"
#include "errno.h"
#include <cstdio>

// This makes puppies die
struct pthread_internal
{
	void* ptrs[26];
	pid_t tid;
};

pid_t __darwin_pthread_mach_thread_np(pthread_t pth)
{
	const pthread_internal* x = reinterpret_cast<pthread_internal*>(pth);
	return x->tid;
}

int __darwin_pthread_mutexattr_settype(pthread_mutexattr_t* attr, int kind)
{
	switch (kind)
	{
	case __DARWIN_PTHREAD_MUTEX_NORMAL:
		kind = PTHREAD_MUTEX_FAST_NP;
		break;
	case __DARWIN_PTHREAD_MUTEX_ERRORCHECK:
		kind = PTHREAD_MUTEX_ERRORCHECK_NP;
		break;
	case __DARWIN_PTHREAD_MUTEX_RECURSIVE:
		kind = PTHREAD_MUTEX_RECURSIVE_NP;
		break;
	default:
		fprintf(stderr, "Unknown pthread_mutexattr_settype kind: %d\n", kind);
	}
	int rv = pthread_mutexattr_settype(attr, kind);
	if (rv == -1)
		errnoOut();
	return rv;
}


static int TranslatePshared(int pshared)
{
	switch (pshared)
	{
	case __DARWIN_PTHREAD_PROCESS_SHARED:
		return PTHREAD_PROCESS_SHARED;
	case __DARWIN_PTHREAD_PROCESS_PRIVATE:
		return PTHREAD_PROCESS_PRIVATE;
	default:
		fprintf(stderr, "Unknown pthread_mutexattr_setpshared pshared: %d\n",
				pshared);
		return pshared;
	}
}
int __darwin_pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared)
{
	pshared = TranslatePshared(pshared);
	int rv = pthread_mutexattr_setpshared(attr, pshared);
	if (rv == -1)
		errnoOut();
	return rv;
}

template<typename Func, typename... Args> int AutoAllocLockGeneric(Func f, __darwin_pthread_rwlock_t* rwlock, Args... args)
{
	int rv;
	if (rwlock->signature == __darwin_pthread_rwlock_t::SIGNATURE_MACRO_INITIALIZED)
	{
		rv = __darwin_pthread_rwlock_init(rwlock, 0);
		if (rv)
			return rv;
	}
	
	rv = f(&rwlock->native, args...);
	if (rv)
		rv = errnoLinuxToDarwin(rv);
	return rv;
}

static int InitializeMacroInitializedMutex(__darwin_pthread_mutex_t* mutex)
{
	pthread_mutexattr_t mta;
	pthread_mutexattr_init(&mta);
	int type;

	switch (mutex->signature)
	{
	case __darwin_pthread_mutex_t::SIGNATURE_MACRO_INITIALIZED_R:
		type = PTHREAD_MUTEX_RECURSIVE;
		break;
	case __darwin_pthread_mutex_t::SIGNATURE_MACRO_INITIALIZED_E:
		type = PTHREAD_MUTEX_ERRORCHECK;
		break;
	case __darwin_pthread_mutex_t::SIGNATURE_MACRO_INITIALIZED:
	default:
		type = PTHREAD_MUTEX_NORMAL;
	}

	pthread_mutexattr_settype(&mta, type);

	return __darwin_pthread_mutex_init(mutex, &mta);
}

template<typename Func, typename... Args> int AutoAllocMutexGeneric(Func f, __darwin_pthread_mutex_t* mutex, Args... args)
{
	int rv;
	if (mutex->signature != __darwin_pthread_mutex_t::SIGNATURE_NATIVE_INITIALIZED)
	{
		rv = InitializeMacroInitializedMutex(mutex);
		if (rv)
			return rv;
	}

	rv = f(&mutex->native, args...);
	if (rv)
		rv = errnoLinuxToDarwin(rv);
	return rv;
}

int __darwin_pthread_rwlockattr_setpshared(pthread_rwlockattr_t* attr, int pshared)
{
	pshared = TranslatePshared(pshared);
	int rv = pthread_rwlockattr_setpshared(attr, pshared);
	if (rv == -1)
		errnoOut();
	return rv;
}

int __darwin_pthread_rwlock_init(__darwin_pthread_rwlock_t* rwlock, const pthread_rwlockattr_t* attr)
{
	rwlock->signature = __darwin_pthread_rwlock_t::SIGNATURE_NATIVE_INITIALIZED;
	int rv = pthread_rwlock_init(&rwlock->native, attr);
	if (rv != 0)
		rv = errnoLinuxToDarwin(rv);
	return rv;
}

int __darwin_pthread_rwlock_destroy(__darwin_pthread_rwlock_t *rwlock)
{
	if (rwlock->signature == __darwin_pthread_rwlock_t::SIGNATURE_NATIVE_INITIALIZED)
	{
		int rv = pthread_rwlock_destroy(&rwlock->native);
		if (rv != 0)
			rv = errnoLinuxToDarwin(rv);
		return rv;
	}
	else
		return 0;
}

int __darwin_pthread_rwlock_rdlock(__darwin_pthread_rwlock_t *rwlock)
{
	return AutoAllocLockGeneric(pthread_rwlock_rdlock, rwlock);
}

int __darwin_pthread_rwlock_tryrdlock(__darwin_pthread_rwlock_t *rwlock)
{
	return AutoAllocLockGeneric(pthread_rwlock_tryrdlock, rwlock);
}

int __darwin_pthread_rwlock_trywrlock(__darwin_pthread_rwlock_t *rwlock)
{
	return AutoAllocLockGeneric(pthread_rwlock_trywrlock, rwlock);
}

int __darwin_pthread_rwlock_wrlock(__darwin_pthread_rwlock_t *rwlock)
{
	return AutoAllocLockGeneric(pthread_rwlock_wrlock, rwlock);
}

int __darwin_pthread_rwlock_timedrdlock(__darwin_pthread_rwlock_t* rwlock, const struct timespec* abs_timeout)
{
	return AutoAllocLockGeneric(pthread_rwlock_timedrdlock, rwlock, abs_timeout);
}

int __darwin_pthread_rwlock_unlock(__darwin_pthread_rwlock_t *rwlock)
{
	return AutoAllocLockGeneric(pthread_rwlock_unlock, rwlock);
}

int __darwin_pthread_mutex_init(__darwin_pthread_mutex_t* mutex, const pthread_mutexattr_t* attr)
{
	int rv = pthread_mutex_init(&mutex->native, attr);
	mutex->signature = __darwin_pthread_mutex_t::SIGNATURE_NATIVE_INITIALIZED;
	if (rv)
		rv = errnoLinuxToDarwin(rv);
	return rv;
}

int __darwin_pthread_mutex_destroy(__darwin_pthread_mutex_t* mutex)
{
	if (mutex->signature == __darwin_pthread_mutex_t::SIGNATURE_NATIVE_INITIALIZED)
	{
		int rv = pthread_mutex_destroy(&mutex->native);
		if (rv)
			rv = errnoLinuxToDarwin(rv);
		return rv;
	}
	return 0;
}

int __darwin_pthread_mutex_lock(__darwin_pthread_mutex_t* mutex)
{
	return AutoAllocMutexGeneric(pthread_mutex_lock, mutex);
}

int __darwin_pthread_mutex_trylock(__darwin_pthread_mutex_t* mutex)
{
	return AutoAllocMutexGeneric(pthread_mutex_trylock, mutex);
}

int __darwin_pthread_mutex_unlock(__darwin_pthread_mutex_t* mutex)
{
	return AutoAllocMutexGeneric(pthread_mutex_unlock, mutex);
}

template<typename Func, typename... Args> int AutoAllocConditionGeneric(Func f, __darwin_pthread_cond_t* cond, Args... args)
{
	int rv;
	if (cond->signature != __darwin_pthread_cond_t::SIGNATURE_NATIVE_INITIALIZED)
	{
		rv = __darwin_pthread_cond_init(cond, nullptr);
		if (rv)
			return rv;
	}

	rv = f(cond->native, args...);
	if (rv)
		rv = errnoLinuxToDarwin(rv);
	return rv;
}

int __darwin_pthread_cond_init(__darwin_pthread_cond_t *cond, const pthread_condattr_t *attr)
{
	cond->native = new pthread_cond_t;
	int rv = pthread_cond_init(cond->native, attr);
	cond->signature = __darwin_pthread_cond_t::SIGNATURE_NATIVE_INITIALIZED;
	if (rv)
		rv = errnoLinuxToDarwin(rv);
	return rv;
}

int __darwin_pthread_cond_destroy(__darwin_pthread_cond_t *cond)
{
	if (cond->signature == __darwin_pthread_cond_t::SIGNATURE_NATIVE_INITIALIZED)
	{
		int rv = pthread_cond_destroy(cond->native);
		delete cond->native;
		if (rv)
			rv = errnoLinuxToDarwin(rv);
		return rv;
	}
	return 0;
}

int __darwin_pthread_cond_signal(__darwin_pthread_cond_t *cond)
{
	return AutoAllocConditionGeneric(pthread_cond_signal, cond);
}

int __darwin_pthread_cond_broadcast(__darwin_pthread_cond_t *cond)
{
	return AutoAllocConditionGeneric(pthread_cond_broadcast, cond);
}

int __darwin_pthread_cond_timedwait(__darwin_pthread_cond_t *cond, __darwin_pthread_mutex_t* mutex, const struct timespec *abstime)
{
	int rv;
	if (mutex->signature != __darwin_pthread_mutex_t::SIGNATURE_NATIVE_INITIALIZED)
	{
		rv = InitializeMacroInitializedMutex(mutex);
        if (rv)
            return rv;
	}

	return AutoAllocConditionGeneric(pthread_cond_timedwait, cond, &mutex->native, abstime);
}

int __darwin_pthread_cond_wait(__darwin_pthread_cond_t *cond, __darwin_pthread_mutex_t* mutex)
{
	int rv;
	if (mutex->signature != __darwin_pthread_mutex_t::SIGNATURE_NATIVE_INITIALIZED)
	{
		rv = InitializeMacroInitializedMutex(mutex);
        if (rv)
            return rv;
	}

	return AutoAllocConditionGeneric(pthread_cond_wait, cond, &mutex->native);
}

