#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <pwd.h>
#include <unistd.h>
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

// The per-user directories belong to the effective user as recorded in the
// user database. HOME is only a fallback: a daemon that changed its uid may
// have inherited it from another user.
static const char* user_home(char* buf, size_t size)
{
	struct passwd pwd;
	struct passwd* result = NULL;

	if (getpwuid_r(geteuid(), &pwd, buf, size, &result) == 0 && result != NULL
		&& result->pw_dir != NULL && result->pw_dir[0] != '\0')
	{
		return result->pw_dir;
	}

	return getenv("HOME");
}

// Creates the directories of the path that do not exist yet, each with the mode given. The mode
// is set explicitly, so the umask doesn't narrow it: on macOS 26.7.3, the user directory that
// confstr() creates is 0755 even with a umask of 077 (test: tests/confstr-user-directories.c).
// Directories that already exist are left alone.
static bool create_directories(const char* path, mode_t mode)
{
	char partial[PATH_MAX];
	size_t length = strlen(path);

	if (length == 0 || length >= sizeof(partial))
		return false;

	for (size_t i = 1; i <= length; i++)
	{
		if (path[i] != '/' && path[i] != '\0')
			continue;

		memcpy(partial, path, i);
		partial[i] = '\0';

		struct stat st;
		if (stat(partial, &st) == 0)
		{
			if (!S_ISDIR(st.st_mode))
				return false;
			continue;
		}
		if (errno != ENOENT)
			return false;
		if (mkdir(partial, mode) != 0)
		{
			if (errno != EEXIST)
				return false;
			continue;
		}
		chmod(partial, mode);
	}
	return true;
}

// The suffix set with _set_user_dir_suffix(), or else the one in the environment. An empty
// suffix is no suffix.
static const char* current_suffix(void)
{
	const char* suffix = dir_suffix;

	if (suffix == NULL)
		suffix = getenv(DIRHELPER_ENV_USER_DIR_SUFFIX);
	if (suffix != NULL && suffix[0] == '\0')
		suffix = NULL;
	return suffix;
}

// The per-user directories that confstr() gives for _CS_DARWIN_USER_DIR, _CS_DARWIN_USER_TEMP_DIR
// and _CS_DARWIN_USER_CACHE_DIR. As documented in confstr(3), each is created when it does not
// exist, the user directory with mode 0755 and the others with 0700. A suffix, set with
// _set_user_dir_suffix() or DIRHELPER_USER_DIR_SUFFIX, names a subdirectory of each.
char* _dirhelper(int which, char* p, size_t limit)
{
	char pwbuf[4096];
	const char* home = NULL;
	const char* suffix;
	mode_t mode = 0700;
	int length;

	// Look the user up before taking the lock, the lookup may be remote
	if (which == DIRHELPER_USER_LOCAL || which == DIRHELPER_USER_LOCAL_CACHE)
		home = user_home(pwbuf, sizeof(pwbuf));

	pthread_mutex_lock(&dir_suffix_lock);
	suffix = current_suffix();

	// QUIRK: a suffix of more than one path component gives no directory, so confstr() fails
	// with EIO for the user and cache directories, and for the temporary directory falls back
	// to TMPDIR. Not documented. Evidence: black-box observation on macOS 26.7.3 (tier 2).
	// Test: tests/confstr-user-directories.c, "quirk: suffix with a slash".
	if (suffix != NULL && strchr(suffix, '/') != NULL)
	{
		pthread_mutex_unlock(&dir_suffix_lock);
		errno = EIO;
		return NULL;
	}

	switch (which)
	{
		case DIRHELPER_USER_LOCAL:
		{
			mode = 0755;
			length = snprintf(p, limit, "%s/%s%s", home ? home : "", suffix ? suffix : "", suffix ? "/" : "");
		} break;
		case DIRHELPER_USER_LOCAL_TEMP:
		{
			const char* tmp;

			tmp = getenv("TMPDIR");
			if (tmp == NULL)
				tmp = "/tmp";

			length = snprintf(p, limit, "%s/%s%s", tmp, suffix ? suffix : "", suffix ? "/" : "");
		} break;
		case DIRHELPER_USER_LOCAL_CACHE:
		{
			if (home != NULL)
				length = snprintf(p, limit, "%s/.cache/%s%s", home, suffix ? suffix : "", suffix ? "/" : "");
			else
				length = snprintf(p, limit, "/tmp/%s%s", suffix ? suffix : "", suffix ? "/" : "");
		} break;
		default:
			pthread_mutex_unlock(&dir_suffix_lock);
			return NULL;
	}
	pthread_mutex_unlock(&dir_suffix_lock);

	if (length < 0 || (size_t)length >= limit)
	{
		errno = ENAMETOOLONG;
		return NULL;
	}
	if (!create_directories(p, mode))
	{
		errno = EIO;
		return NULL;
	}
	return p;
}

int _set_user_dir_suffix(const char* suffix) {
	pthread_mutex_lock(&dir_suffix_lock);
	dir_suffix = suffix;
	pthread_mutex_unlock(&dir_suffix_lock);
	return 1; // weird, it's supposed to return non-zero on success (maybe it's actually a bool?)
};

