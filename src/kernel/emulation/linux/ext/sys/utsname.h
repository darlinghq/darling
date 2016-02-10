#ifndef EXT_UNAME_H
#define EXT_UNAME_H

#define LINUX_UTSNAME_LENGTH 65

struct linux_utsname
{
	char sysname[LINUX_UTSNAME_LENGTH];
	char nodename[LINUX_UTSNAME_LENGTH];
	char release[LINUX_UTSNAME_LENGTH];
	char version[LINUX_UTSNAME_LENGTH];
	char machine[LINUX_UTSNAME_LENGTH];
	char domainname[LINUX_UTSNAME_LENGTH];
};

int __linux_uname(struct linux_utsname* p);

#endif

