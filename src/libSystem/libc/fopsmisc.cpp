#include "config.h"
#include "fopsmisc.h"
#include "errno.h"
#include "darwin_errno_codes.h"
#include "common/path.h"
#include "common/auto.h"
#include "trace.h"
#include <cstdlib>
#include <cstdio>

char *__darwin_realpath(const char *path, char *resolved_path)
{
	TRACE2(path, resolved_path);
	if (!path)
	{
		errno = DARWIN_EINVAL;
		return 0;
	}
	
	path = translatePathCI(path);
	
	if (!resolved_path) // DARWIN_EXTSN
		resolved_path = static_cast<char*>(malloc(DARWIN_MAXPATHLEN));
	
	char* rv = realpath(path, resolved_path);
	if (!rv)
		errnoOut();
	
	return rv;
}

int __darwin_mkstemp(char *tpl)
{
	TRACE1(tpl);
	return AutoPathErrno<int>(mkstemp, tpl);
}

int __darwin_mkostemp (char *tpl, int flags)
{
	TRACE2(tpl, flags);
	return AutoPathErrno<int>(mkostemp, tpl, flags);
}

char *__darwin_mktemp(char *tpl)
{
	TRACE1(tpl);
	return AutoPathErrno<char*>(mktemp, tpl);
}

char *__darwin_tempnam(const char *dir, const char *pfx)
{
	TRACE2(dir, pfx);
	return AutoPathErrno<char*>(tempnam, dir, pfx);
}

