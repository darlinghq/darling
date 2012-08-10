#include "config.h"
#include "fopsmisc.h"
#include "errno.h"
#include "darwin_errno_codes.h"
#include "common/path.h"
#include "common/auto.h"
#include <cstdlib>
#include <cstdio>

char *__darwin_realpath(const char *path, char *resolved_path)
{
	if (!path)
	{
		errno = DARWIN_EINVAL;
		return 0;
	}
	
	path = translatePathCI(path);
	
	char* rv = realpath(path, resolved_path);
	if (!rv)
		errnoOut();
	
	return rv;
}

int __darwin_mkstemp(char *tpl)
{
	return AutoPathErrno<int>(mkstemp, tpl);
}

int __darwin_mkostemp (char *tpl, int flags)
{
	return AutoPathErrno<int>(mkostemp, tpl, flags);
}

char *__darwin_mktemp(char *tpl)
{
	return AutoPathErrno<char*>(mktemp, tpl);
}

char *__darwin_tempnam(const char *dir, const char *pfx)
{
	return AutoPathErrno<char*>(tempnam, dir, pfx);
}

