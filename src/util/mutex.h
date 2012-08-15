#ifndef DARLING_MUTEX_H
#define DARLING_MUTEX_H
#include <pthread.h>
#include <cassert>

namespace Darling
{

class Mutex
{
public:
	Mutex()
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&m_mutex, &attr);
	}
	~Mutex()
	{
		pthread_mutex_destroy(&m_mutex);
	}
	void lock()
	{
		pthread_mutex_lock(&m_mutex);
	}
	void unlock()
	{
		pthread_mutex_unlock(&m_mutex);
	}
	bool trylock()
	{
		return pthread_mutex_trylock(&m_mutex) == 0;
	}
private:
	pthread_mutex_t m_mutex;
};

class MutexLock
{
public:
	MutexLock(Mutex& mutex)
	: m_mutex(&mutex)
	{
		m_mutex->lock();
	}
	MutexLock(Mutex* mutex)
	: m_mutex(mutex)
	{
		assert(m_mutex != 0);
		m_mutex->lock();
	}
	~MutexLock()
	{
		m_mutex->unlock();
	}
private:
	Mutex* m_mutex;
};

}

#endif
