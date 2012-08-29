#ifndef LIBC_SYSCONF_H
#define LIBC_SYSCONF_H

enum {
	DARWIN_SC_PAGESIZE = 29
};

extern "C" long __darwin_sysconf(int name);

#endif

