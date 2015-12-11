#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

void _dirhelper_fork_child(void)
{
}

enum {
	DIRHELPER_USER_LOCAL = 0,
	DIRHELPER_USER_LOCAL_TEMP,
	DIRHELPER_USER_LOCAL_CACHE
};

char* _dirhelper(int which, char* p, size_t limit)
{
	switch (which)
	{
		case DIRHELPER_USER_LOCAL:
		{
			const char* home;

			home = getenv("HOME");
			if (home == NULL)
				home = "/";

			strlcpy(p, home, limit);
			return p;
		}
		case DIRHELPER_USER_LOCAL_TEMP:
		{
			const char* tmp;

			tmp = getenv("TMPDIR");
			if (tmp == NULL)
				tmp = "/tmp";

			strlcpy(p, tmp, limit);
			return p;
		}
		case DIRHELPER_USER_LOCAL_CACHE:
		{
			const char* home;

			home = getenv("HOME");
			if (home != NULL)
				snprintf(p, limit, "%s/.cache", home);
			else
				strlcpy(p, "/tmp", limit);

			return p;
		}
		default:
			return NULL;
	}
}

