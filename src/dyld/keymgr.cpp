#include "keymgr.h"
#include <pthread.h>
#include <map>

struct Entry
{
	void* value;
	pthread_mutex_t mutex;
};
static std::map<unsigned,Entry> m_keymgr;
static pthread_mutex_t m_mutexNew = PTHREAD_MUTEX_INITIALIZER;

void* _keymgr_get_and_lock_processwide_ptr(unsigned key)
{
	pthread_mutex_lock(&m_mutexNew);

	auto iter = m_keymgr.find(key);
	void* rv;

	if (iter == m_keymgr.end())
	{
		Entry e = { 0, PTHREAD_MUTEX_INITIALIZER };
		m_keymgr[key] = e;
		rv = 0;

		pthread_mutex_lock(&m_keymgr[key].mutex);
	}
	else
	{
		pthread_mutex_lock(&m_keymgr[key].mutex);
		rv = iter->second.value;
	}

	pthread_mutex_unlock(&m_mutexNew);
	return rv;
}

void _keymgr_set_and_unlock_processwide_ptr(unsigned key, void* ptr)
{
	m_keymgr[key].value = ptr;
	pthread_mutex_unlock(&m_keymgr[key].mutex);
}

void _keymgr_unlock_processwide_ptr(unsigned key)
{
	pthread_mutex_unlock(&m_keymgr[key].mutex);
}

