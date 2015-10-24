/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#include <sys/cdefs.h>

#if __DARWIN_UNIX03

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

extern int __getrlimit(int resource, struct rlimit *rlp);

/*
 * getrlimit stub, for conformance, OR in _RLIMIT_POSIX_FLAG
 *
 * This is for UNIX03 only.
 */
int
getrlimit(int resource, struct rlimit *rlp)
{
	resource |= _RLIMIT_POSIX_FLAG;
	return(__getrlimit(resource, rlp));
}

#endif /* __DARWIN_UNIX03 */
