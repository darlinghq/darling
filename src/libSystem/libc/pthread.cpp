#include "pthread.h"
#include "errno.h"
#include <cstdio>
#include <unistd.h>
#include <map>
#include <memory>
#include <util/mutex.h>
#include <sys/syscall.h>

static Darling::Mutex g_pthreadMutex;
static pthread_key_t g_pthreadDestructor;
static std::map<pthread_t, pid_t> g_pthreadToTid __attribute__ ((init_priority (101)));

static void pthread_destroyed(void*)
{
	Darling::MutexLock lock(g_pthreadMutex);
	auto it = g_pthreadToTid.find(pthread_self());

	if (it != g_pthreadToTid.end())
		g_pthreadToTid.erase(it);
}

__attribute__((constructor))
static void initPthreadDestructor()
{
	pthread_key_create(&g_pthreadDestructor, pthread_destroyed);
	g_pthreadToTid[pthread_self()] = syscall(SYS_gettid);
}

struct WrapPthreadArg
{
	void *(*start_routine) (void *);
	void *arg;
};

static void* StartRoutineWrapper(void* a)
{
	WrapPthreadArg* wrap = static_cast<WrapPthreadArg*>(a);
	WrapPthreadArg mywrap = *wrap;

	// Set something into the key so that we get called when the thread exits
	// (and however it exits)
	pthread_setspecific(g_pthreadDestructor, &mywrap);

	g_pthreadMutex.lock();
	g_pthreadToTid[pthread_self()] = syscall(SYS_gettid);
	g_pthreadMutex.unlock();

	delete wrap;

	return mywrap.start_routine(mywrap.arg);
}

pid_t Darling::tidForPthread(pthread_t pth)
{
	Darling::MutexLock lock(g_pthreadMutex);
	auto it = g_pthreadToTid.find(pth);

	if (it != g_pthreadToTid.end())
		return it->second;
	else
		return 0;
}

int __darwin_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	WrapPthreadArg* wrap = new WrapPthreadArg { start_routine, arg };
	int rv = pthread_create(thread, attr, StartRoutineWrapper, wrap);

	if (rv != 0)
		delete wrap;

	return errnoLinuxToDarwin(rv);
}

pid_t __darwin_pthread_mach_thread_np(pthread_t pth)
{
	return Darling::tidForPthread(pth);
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

int __darwin_pthread_once(__darwin_pthread_once_t *once_control, void (*init_routine)(void))
{
	return pthread_once(&once_control->native, init_routine);
}

size_t pthread_get_stacksize_np(pthread_t pth)
{
	pthread_attr_t attr;
	void* addr;
	size_t size = 0;

	if (pthread_getattr_np(pth, &attr) == 0)
	{
		pthread_attr_getstack(&attr, &addr, &size);
		pthread_attr_destroy(&attr);
	}

	return size;
}

void* pthread_get_stackaddr_np(pthread_t pth)
{
	pthread_attr_t attr;
	void* addr = nullptr;
	size_t size = 0;

	if (pthread_getattr_np(pth, &attr) == 0)
	{
		pthread_attr_getstack(&attr, &addr, &size);
		pthread_attr_destroy(&attr);
	}

	return addr;
}

int __darwin_pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
	int err;

	if (detachstate == __DARWIN_PTHREAD_CREATE_JOINABLE)
		detachstate = PTHREAD_CREATE_JOINABLE;
	else if (detachstate == __DARWIN_PTHREAD_CREATE_DETACHED)
		detachstate = PTHREAD_CREATE_DETACHED;

	err = pthread_attr_setdetachstate(attr, detachstate);

	if (err != 0)
		err = errnoLinuxToDarwin(err);

	return err;
}

int __darwin_pthread_attr_getdetachstate(pthread_attr_t *attr, int *detachstate)
{
	int err;

	err = pthread_attr_getdetachstate(attr, detachstate);

	if (err != 0)
	{
		err = errnoLinuxToDarwin(err);
		return err;
	}
	
	if (*detachstate == PTHREAD_CREATE_JOINABLE)
		*detachstate = __DARWIN_PTHREAD_CREATE_JOINABLE;
	else if (*detachstate == PTHREAD_CREATE_DETACHED)
		*detachstate = __DARWIN_PTHREAD_CREATE_DETACHED;

	return 0;
}

int __darwin_pthread_sigmask(int how, const __darwin_sigset_t *set, __darwin_sigset_t *oldset)
{
	std::unique_ptr<sigset_t> nset;
	std::unique_ptr<sigset_t> noldset;

	if (set)
	{
		nset.reset(new sigset_t);
		*nset = Darling::sigsetDarwinToLinux(set);
	}
	if (oldset)
		noldset.reset(new sigset_t);

	// -1 -> conversion from Darwin to Linux
	int rv = pthread_sigmask(how - 1, nset.get(), noldset.get());

	if (rv == -1)
		errnoOut();
	else if (oldset)
		*oldset = Darling::sigsetLinuxToDarwin(noldset.get());
	
	return rv;
}

