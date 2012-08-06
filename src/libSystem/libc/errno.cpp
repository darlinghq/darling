
#include <string.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <err.h>
#include "errno.h"
#include "darwin_errno_codes.h"

static void initErrnoMappingTable() __attribute__ ((constructor));
static int darwin_to_linux[130];
static int linux_to_darwin[130];

// This is deprecated, we don't bother to support that
extern "C" const char * const __darwin_sys_errlist[0];
extern "C" const int __darwin_sys_nerr = 0;

// For a non-gnu version of strerror_r
extern "C" int __xpg_strerror_r (int __errnum, char *__buf, size_t __buflen)
     __THROW __nonnull ((2));

int cthread_errno()
{
	return errno;
}

void initErrnoMappingTable()
{
	::memset(darwin_to_linux, 0, sizeof(darwin_to_linux));
	::memset(linux_to_darwin, 0, sizeof(linux_to_darwin));
	
#	include "darwin_to_linux.map"
#	include "linux_to_darwin.map"

	darwin_to_linux[_DARWIN_EBADEXEC] = ENOEXEC;
	darwin_to_linux[_DARWIN_EBADARCH] = ENOEXEC;
	darwin_to_linux[_DARWIN_EBADMACHO] = ENOEXEC;
}

int errnoDarwinToLinux(int err)
{
	if (err < sizeof(darwin_to_linux)/sizeof(darwin_to_linux[0])-1)
		return darwin_to_linux[err];
	else
		return 0;
}

int errnoLinuxToDarwin(int err)
{
	if (err < sizeof(linux_to_darwin)/sizeof(linux_to_darwin[0])-1)
		return linux_to_darwin[err];
    else
        return 0;
}

char* __darwin_strerror(int errnum)
{
	errnum = errnoDarwinToLinux(errnum);
	return ::strerror(errnum);
}

int __darwin_strerror_r(int errnum, char *strerrbuf, size_t buflen)
{
	errnum = errnoDarwinToLinux(errnum);
	return ::__xpg_strerror_r(errnum, strerrbuf, buflen);
}

void __darwin_perror(const char *s)
{
	// First map the error back
	errno = errnoDarwinToLinux(errno);
	::perror(s);
	// Now map it back again
	errno = errnoLinuxToDarwin(errno);
}


