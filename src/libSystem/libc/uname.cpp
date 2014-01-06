#include "../../config.h"
#include "uname.h"
#include "errno.h"
#include <errno.h>
#include <cstring>
#include "darwin_errno_codes.h"
#include <sys/utsname.h>
#ifndef NO_DARWIN_UNAME
#	include "IniConfig.h"
#endif
#include <bsd/string.h>

int __darwin_uname(__darwin_utsname* buf)
{
	if (!buf)
	{
		errno = DARWIN_EINVAL;
		return -1;
	}

	struct utsname nbuf;
	int rv = uname(&nbuf);

    if (rv == -1)
	{
		errnoOut();
		return rv;
	}

	strcpy(buf->nodename, nbuf.nodename);
	strcpy(buf->machine, nbuf.machine);

#ifdef NO_DARWIN_UNAME
	strcpy(buf->sysname, nbuf.sysname);
	strcpy(buf->release, nbuf.release);
	strcpy(buf->version, nbuf.version);
#else
	static IniConfig iniConfig(ETC_DARLING_PATH "/version.conf", true);
	const char *sysname = nullptr, *release = nullptr, *version = nullptr;

	if (iniConfig.hasSection("uname"))
	{
		const IniConfig::ValueMap* map = iniConfig.getSection("uname");
		sysname = map->find("sysname")->second.c_str();
		release = map->find("release")->second.c_str();
		version = map->find("version")->second.c_str();
	}

	if (!sysname || !*sysname)
	    strcpy(buf->sysname, "Darwin");
	else
		strlcpy(buf->sysname, sysname, __DARWIN_SYS_NAMELEN);

	if (!release || !*release)
	    strcpy(buf->release, "12.0.0");
	else
		strlcpy(buf->release, release, __DARWIN_SYS_NAMELEN);

	if (!version || !*version)
	    strcpy(buf->version, "Darwin Kernel Version 12.0.0");
	else
		strlcpy(buf->version, version, __DARWIN_SYS_NAMELEN);
#endif

    return 0;
}


