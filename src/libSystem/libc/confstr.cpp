#include "config.h"
#include "confstr.h"
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <string>
#include <bsd/string.h>
#include "darwin_errno_codes.h"

size_t __darwin_confstr(int name, char* buf, size_t len)
{
	switch (name)
	{
		case DARWIN__CS_DARWIN_USER_DIR:
		{
			const char* home = getenv("HOME");
			if (!home)
			{
				if (len > 0)
					*buf = 0;
				return 0;
			}
			else
				return strlcpy(buf, home, len) + 1;
		}
		case DARWIN__CS_DARWIN_USER_TEMP_DIR:
		{
			const char* tmp = getenv("TMPDIR");
			
			if (!tmp)
				tmp = "/tmp";
			
			return strlcpy(buf, tmp, len) + 1;
		}
		case DARWIN__CS_DARWIN_USER_CACHE_DIR:
		{
			std::string cacheDir;
			const char* home = getenv("HOME");
			
			if (!home)
			{
				if (len > 0)
					*buf = 0;
				return 0;
			}
			
			cacheDir = home;
			cacheDir += "/.cache";
			
			return strlcpy(buf, cacheDir.c_str(), len) + 1;
		}
		case _CS_PATH:
			return confstr(name, buf, len);
		default:
			errno = DARWIN_EINVAL;
			return 0;
	}
}
