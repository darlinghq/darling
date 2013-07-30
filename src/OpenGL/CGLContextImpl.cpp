#include "CGLContextImpl.h"

CGLContextImpl::CGLContextImpl()
{
	pthread_mutexattr_t mta;

	pthread_mutexattr_init(&mta);
	pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&m_lock, &mta);
}

CGLContextImpl::~CGLContextImpl()
{
	pthread_mutex_destroy(&m_lock);
}

void CGLContextImpl::lock()
{
	pthread_mutex_lock(&m_lock);
}

void CGLContextImpl::unlock()
{
	pthread_mutex_unlock(&m_lock);
}

