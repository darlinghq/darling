#ifndef RWLOCK_H
#define RWLOCK_H
#include <pthread.h>

namespace Darling {

class RWMutex
{
public:
	RWMutex()
	{
		pthread_rwlock_init(&m_lock, nullptr);
	}
	~RWMutex()
	{
		pthread_rwlock_destroy(&m_lock);
	}
	
	void rdlock()
	{
		pthread_rwlock_rdlock(&m_lock);
	}
	
	void wrlock()
	{
		pthread_rwlock_wrlock(&m_lock);
	}
	
	void unlock()
	{
		pthread_rwlock_unlock(&m_lock);
	}
private:
	pthread_rwlock_t m_lock;
};

class RWMutexReadLock
{
public:
	RWMutexReadLock(RWMutex& mut)
	: m_mutex(mut)
	{
		m_mutex.rdlock();
	}
	
	~RWMutexReadLock()
	{
		m_mutex.unlock();
	}
private:
	RWMutex& m_mutex;
};

class RWMutexWriteLock
{
public:
	RWMutexWriteLock(RWMutex& mut)
	: m_mutex(mut)
	{
		m_mutex.wrlock();
	}
	
	~RWMutexWriteLock()
	{
		m_mutex.unlock();
	}
private:
	RWMutex& m_mutex;
};

}

#endif
