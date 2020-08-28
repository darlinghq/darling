#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "dirhelper_priv.h"

static const char* dir_suffix = NULL;
static pthread_mutex_t dir_suffix_lock = PTHREAD_MUTEX_INITIALIZER;

void _dirhelper_fork_child(void)
{
}

void _libcoreservices_fork_child(void)
{
	/**
	 * my god, threading is *incredibly* tricky when mixed with `fork()`!
	 * and the POSIX spec is so unclear about the behavior of mutexes when `fork()`ing
	 * and the macOS docs are no help
	 *
	 * the *only* viable resources i found was this commit in jemalloc:
	 * https://github.com/jemalloc/jemalloc/commit/0a4f5a7eea5e42292cea95fd30a88201c8d4a1ca
	 *
	 * basically, it *seems* that it's fine to reinitialize mutexes in the child---on macOS at least
	 * but i really wish the POSIX spec or macOS docs mentioned this kind of stuff!
	 *
	 * undefined behavior is really the **worst** behavior
	 */
	dir_suffix_lock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;

	/**
	 * i checked on a real macOS system and confirmed this behavior:
	 * `fork()` does NOT reset the directory suffix
	 */
	//dir_suffix = NULL;
}

char* _dirhelper(int which, char* p, size_t limit)
{
	pthread_mutex_lock(&dir_suffix_lock);
	switch (which)
	{
		case DIRHELPER_USER_LOCAL:
		{
			const char* home;

			home = getenv("HOME");
			if (home == NULL)
				home = "";

			snprintf(p, limit, "%s/%s%s", home, dir_suffix ? dir_suffix : "", dir_suffix ? "/" : "");
		} break;
		case DIRHELPER_USER_LOCAL_TEMP:
		{
			const char* tmp;

			tmp = getenv("TMPDIR");
			if (tmp == NULL)
				tmp = "/tmp";

			snprintf(p, limit, "%s/%s%s", tmp, dir_suffix ? dir_suffix : "", dir_suffix ? "/" : "");
		} break;
		case DIRHELPER_USER_LOCAL_CACHE:
		{
			const char* home;

			home = getenv("HOME");
			if (home != NULL)
				snprintf(p, limit, "%s/.cache/%s%s", home, dir_suffix ? dir_suffix : "", dir_suffix ? "/" : "");
			else
				snprintf(p, limit, "/tmp/%s%s", dir_suffix ? dir_suffix : "", dir_suffix ? "/" : "");
		} break;
		default:
			p = NULL;
	}
	pthread_mutex_unlock(&dir_suffix_lock);
	return p;
}

int _set_user_dir_suffix(const char* suffix) {
	pthread_mutex_lock(&dir_suffix_lock);
	dir_suffix = suffix;
	pthread_mutex_unlock(&dir_suffix_lock);
	return 0;
};

