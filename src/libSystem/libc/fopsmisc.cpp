#include "config.h"
#include "fopsmisc.h"
#include "errno.h"
#include "darwin_errno_codes.h"
#include "common/path.h"
#include "common/auto.h"
#include <cstdlib>

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
