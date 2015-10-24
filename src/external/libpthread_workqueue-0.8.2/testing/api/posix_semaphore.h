#ifndef DISPATCH_WIN_POSIX_SEMAPHORE_
#define DISPATCH_WIN_POSIX_SEMAPHORE_

typedef HANDLE sem_t;

static int sem_init(sem_t * sem, int shared, unsigned int val)
{
	*sem = CreateSemaphore(0, val, 1, 0);
	// TODO: Proper error handling
	return *sem == 0;
}

static inline int sem_destroy(sem_t* s)
{
	return CloseHandle(s) != 1;
}

static inline int sem_post(sem_t* s)
{
	return !ReleaseSemaphore(s, 1, 0);
}

static inline int sem_wait(sem_t* s)
{
	return WaitForSingleObject(s, INFINITE) == WAIT_FAILED;
}

static int sem_timedwait(sem_t * sem, const struct timespec * timeout)
{
	DWORD duration = (DWORD)(timeout->tv_nsec / 1000000) + (DWORD)(timeout->tv_sec * 1000);

	switch(WaitForSingleObject(sem,duration) ){
	case WAIT_TIMEOUT:
		return ETIMEDOUT;
	case WAIT_FAILED:
		return EINVAL;
	default:
		return 0;
	}
}

#endif /* DISPATCH_WIN_POSIX_SEMAPHORE_ */
