#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/getprogname.c,v 1.4 2002/03/29 22:43:41 markm Exp $");

#include "namespace.h"
#include <stdlib.h>
#include <crt_externs.h>
#define	__progname	(*_NSGetProgname())
#include "un-namespace.h"

#include "libc_private.h"

__weak_reference(_getprogname, getprogname);

const char *
_getprogname(void)
{

	return (__progname);
}
