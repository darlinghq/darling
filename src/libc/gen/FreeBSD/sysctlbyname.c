/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.org> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/sysctlbyname.c,v 1.5 2002/02/01 00:57:29 obrien Exp $");

#include <sys/types.h>
#include <sys/sysctl.h>
#include <string.h>
#include <sys/errno.h>
#include <TargetConditionals.h>

extern int __sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp,
					void *newp, size_t newlen);
extern int
__sysctlbyname(const char *name, size_t namelen, void *oldp, size_t *oldlenp, void *newp,
			   size_t newlen);

int
sysctlbyname(const char *name, void *oldp, size_t *oldlenp, void *newp,
	     size_t newlen)
{
	int name2oid_oid[2];
	int real_oid[CTL_MAXNAME+2];
	int error;
	size_t oidlen;

#if !TARGET_IPHONE_SIMULATOR
	/* Try primary system call first, fall back if not supported */
	error = __sysctlbyname(name, strlen(name), oldp, oldlenp, newp, newlen);
	if ((error == 0) || (errno != ENOSYS))
		return error;
#endif /* !TARGET_IPHONE_SIMULATOR */
	
	name2oid_oid[0] = 0;	/* This is magic & undocumented! */
	name2oid_oid[1] = 3;

	oidlen = sizeof(real_oid);
	error = __sysctl(name2oid_oid, 2, real_oid, &oidlen, (void *)name,
		       strlen(name));
	if (error < 0) 
		return error;
	oidlen /= sizeof (int);
	error = __sysctl(real_oid, oidlen, oldp, oldlenp, newp, newlen);
	return (error);
}

