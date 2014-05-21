#include "Multiprocessing.h"
#include <unistd.h>
#include <ctime>
#include <pthread.h>
#include "MacErrors.h"

Boolean _MPIsFullyInitialized()
{
	return true;
}

OSStatus MPDelayUntil(AbsoluteTime* time)
{
	struct timespec ts = { time_t(*time / 1000000000ll), long(*time % 1000000000ll) };
	nanosleep(&ts, nullptr);
	return noErr;
}

unsigned long MPProcessors()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

OSStatus MPCreateCriticalRegion(MPCriticalRegionID* criticalRegion)
{
	pthread_mutex_t* pp = new pthread_mutex_t;
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	*criticalRegion = pp;
	
	pthread_mutex_init(pp, &attr);
	return noErr;
}

OSStatus MPDeleteCriticalRegion(MPCriticalRegionID criticalRegion)
{
	pthread_mutex_t* pp = (pthread_mutex_t*) criticalRegion;
	
	if (pp != nullptr)
	{
		pthread_mutex_destroy(pp);
		delete pp;
	}
	
	return noErr;
}

OSStatus MPEnterCriticalRegion(MPCriticalRegionID criticalRegion, Duration timeout)
{
	pthread_mutex_t* pp = (pthread_mutex_t*) criticalRegion;
	
	if (!pp)
		return paramErr;
	
	if (timeout == kDurationForever)
	{
		pthread_mutex_lock(pp);
		return noErr;
	}
	else
	{
		do
		{
			if (pthread_mutex_trylock(pp) == 0)
				return noErr;
			
			if (timeout > 0)
			{
				usleep(10*1000);
				timeout -= 10;
			}
		}
		while (timeout > 0);
		
		return kMPTimeoutErr;
	}
}

OSStatus MPExitCriticalRegion(MPCriticalRegionID criticalRegion)
{
	pthread_mutex_t* pp = (pthread_mutex_t*) criticalRegion;
	
	if (pp != nullptr)
	{
		pthread_mutex_unlock(pp);
		return noErr;
	}
	else
		return paramErr;
}
