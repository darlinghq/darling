#ifndef CGLCONTEXTIMPL_H
#define CGLCONTEXTIMPL_H
#include <pthread.h>

class CGLContextImpl
{
public:
	CGLContextImpl();
	virtual ~CGLContextImpl();

	void lock();
	void unlock();
private:
	pthread_mutex_t m_lock;
};

#endif

