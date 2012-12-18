#ifndef LIBC_UNAME_H
#define LIBC_UNAME_H

#define __DARWIN_SYS_NAMELEN 256

struct __darwin_utsname
{
	char sysname[__DARWIN_SYS_NAMELEN];
	char nodename[__DARWIN_SYS_NAMELEN];
	char release[__DARWIN_SYS_NAMELEN];
	char version[__DARWIN_SYS_NAMELEN];
	char machine[__DARWIN_SYS_NAMELEN];
};


extern "C" int __darwin_uname(__darwin_utsname* buf);

#endif

