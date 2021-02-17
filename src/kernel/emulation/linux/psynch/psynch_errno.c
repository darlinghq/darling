#include "../errno.h"

#ifndef USE_NEW_PSYNC_CODE
	#define USE_NEW_PSYNC_CODE 0
#endif

// psynch syscalls send back extra bits and leave only the bottom 8 bits for errors
// So we have to filter out the top 24 bits, translate errno and put the 24 bits back.
int psynch_errno(int rv)
{
	int top;
	if (rv >= 0)
		return rv;

	rv = -rv;
	top = rv & 0xffffff00;
#if USE_NEW_PSYNC_CODE
	rv = rv & 0xff;
#else
	rv = errno_linux_to_bsd(rv & 0xff);
#endif
	rv |= top;

	return -rv;
}

