/*
 * Copyright (c) 2006-2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef NO_SYSCALL_LEGACY

#define _NONSTD_SOURCE
#include <sys/cdefs.h>

/*
 * We need conformance on so that EOPNOTSUPP=102.  But the routine symbol
 * will still be the legacy (undecorated) one.
 */
#undef __DARWIN_UNIX03
#define __DARWIN_UNIX03 1

#include <sys/attr.h>
#include "_errno.h"

#ifdef NO_SYSCALL_LEGACY
extern int __getattrlist(const char *, void *, void *, size_t, unsigned int);
#else /* !__LP64__ */
extern int __getattrlist(const char *, void *, void *, size_t, unsigned long);
#endif /* __LP64__ */

/*
 * getattrlist stub, legacy version
 */
int
#ifdef __LP64__
getattrlist(const char *path, void *attrList, void *attrBuf,
    size_t attrBufSize, unsigned int options)
#else /* !__LP64__ */
getattrlist(const char *path, void *attrList, void *attrBuf,
    size_t attrBufSize, unsigned long options)
#endif /* __LP64__ */
{
	int ret = __getattrlist(path, attrList, attrBuf, attrBufSize, options);

	/* use ENOTSUP for legacy behavior */
	if (ret < 0 && errno == EOPNOTSUPP) {
		errno = ENOTSUP;
	}
	return ret;
}

#endif /* NO_SYSCALL_LEGACY */
