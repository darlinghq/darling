#include "semwait_signal.h"
#include "../base.h"
#include "../../../../../platform-include/sys/errno.h"
#include "../time/gettimeofday.h"
#include <asm/unistd.h>
#include <stddef.h>

typedef int kern_return_t;
extern kern_return_t semaphore_timedwait_signal_trap(int cond_sem, int mutex_sem, unsigned int tv_sec, unsigned int tv_nsec);
extern kern_return_t semaphore_timedwait_trap(int cond_sem, unsigned int tv_sec, unsigned int tv_nsec);
extern kern_return_t semaphore_wait_signal_trap(int cond_sem, int mutex_sem);
extern kern_return_t semaphore_wait_trap(int cond_sem);

#define KERN_SUCCESS	0
#define KERN_ABORTED	14

long sys_semwait_signal(int cond_sem, int mutex_sem, int timeout, int relative, int64_t tv_sec, int32_t tv_nsec)
{
	return sys_semwait_signal_nocancel(cond_sem, mutex_sem, timeout, relative, tv_sec, tv_nsec);
}

long sys_semwait_signal_nocancel(int cond_sem, int mutex_sem, int timeout, int relative, int64_t tv_sec, int32_t tv_nsec)
{
	kern_return_t kr;

	if (timeout)
	{
		if (!relative)
		{
			struct bsd_timeval tv;
			sys_gettimeofday(&tv, NULL);

			if (tv.tv_usec*1000 < tv_nsec)
				tv_nsec -= tv.tv_usec * 1000;
			else
			{
				tv_nsec = 1000000000ull + tv_nsec - (tv.tv_usec * 1000);
				tv_sec--;
			}
			tv_sec -= tv.tv_sec;

		}

		if (mutex_sem != 0)
			kr = semaphore_timedwait_signal_trap(cond_sem, mutex_sem, tv_sec, tv_nsec);
		else
			kr = semaphore_timedwait_trap(cond_sem, tv_sec, tv_nsec);
	}
	else
	{
		if (mutex_sem != 0)
			kr = semaphore_wait_signal_trap(cond_sem, mutex_sem);
		else
			kr = semaphore_wait_trap(cond_sem);
	}

	if (kr == KERN_ABORTED)
		return -EINTR;
	else if (kr != KERN_SUCCESS)
		return -EINVAL;

	return 0;
}

