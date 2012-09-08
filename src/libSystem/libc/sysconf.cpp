#include "config.h"
#include "sysconf.h"
#include "trace.h"
#include <unistd.h>

long __darwin_sysconf(int name)
{
	TRACE1(name);
	switch (name)
	{
		case DARWIN_SC_PAGESIZE:
			name = _SC_PAGESIZE;
			break;
		case DARWIN_SC_GETPW_R_SIZE_MAX:
			name = _SC_GETPW_R_SIZE_MAX;
			break;
		default:
			break;
	}

	return sysconf(name);
}

