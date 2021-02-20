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

#include <signal.h>

extern int __kill(pid_t pid, int sig, int posix);

/*
 * kill stub, which wraps a modified kill system call that takes a posix
 * behaviour indicator as the third parameter to indicate whether or not
 * conformance to standards is needed.  We use a trailing parameter in
 * case the call is called directly via syscall(), since for most uses,
 * it won't matter to the caller.
 */
int
kill(pid_t pid, int sig)
{
#if __DARWIN_UNIX03
	return __kill(pid, sig, 1);
#else   /* !__DARWIN_UNIX03 */
	return __kill(pid, sig, 0);
#endif  /* !__DARWIN_UNIX03 */
}
