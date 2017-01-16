#include "alloc_once_impl.h"
#include <pthread.h>

static pthread_mutex_t mutexes[OS_ALLOC_ONCE_KEY_MAX];
static pthread_once_t mutexes_once = PTHREAD_ONCE_INIT;
static void* blocks[OS_ALLOC_ONCE_KEY_MAX];

static void init_mutexes(void);

void* os_alloc_once(int key, size_t size, void(*ctor)(void*))
{
	if (key < 0 || key >= OS_ALLOC_ONCE_KEY_MAX)
		return NULL;

	pthread_once(&mutexes_once, init_mutexes);

	if (size != 0 && blocks[key] == NULL)
	{
		pthread_mutex_lock(&mutexes[key]);

		if (blocks[key] == NULL)
		{
			blocks[key] = malloc(size);
			memset(blocks[key], 0, size);
			if (ctor != NULL)
				ctor(blocks[key]);
		}

		pthread_mutex_unlock(&mutexes[key]);
	}

	return blocks[key];
}

static void init_mutexes(void)
{
	int i = 0;
	for (i = 0; i < OS_ALLOC_ONCE_KEY_MAX-1; i++)
		pthread_mutex_init(&mutexes, NULL);
}

