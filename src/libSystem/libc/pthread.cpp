#include "pthread.h"
#include "errno.h"
#include <cstdio>

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

int __darwin_pthread_rwlockattr_setpshared(pthread_rwlockattr_t* attr, int pshared)
{
	pshared = TranslatePshared(pshared);
	int rv = pthread_rwlockattr_setpshared(attr, pshared);
	if (rv == -1)
		errnoOut();
	return rv;
}
