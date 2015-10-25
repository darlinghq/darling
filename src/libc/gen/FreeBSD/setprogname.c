#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/setprogname.c,v 1.8 2002/03/29 22:43:41 markm Exp $");

#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <crt_externs.h>
#define	__progname	(*_NSGetProgname())

#include "libc_private.h"

void
setprogname(const char *progname)
{
	const char *p;
	char buf[2*MAXCOMLEN+1];
	int mib[2];
	
	p = strrchr(progname, '/');
	if (p != NULL)
		__progname = (char *)(p = p + 1);
	else
		__progname = (char *)(p = progname);

	strlcpy(&buf[0], (char *)(p), sizeof(buf));

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROCNAME;

	/* ignore errors as this is not a hard error */
	sysctl(mib, 2, NULL, NULL, &buf[0], strlen(buf));
}
