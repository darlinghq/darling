/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#define	_DARWIN_C_SOURCE

#include <errno.h>
#include <pwd.h>
#include <stdint.h>
#include <unistd.h>

int32_t getgroupcount(const char *name, gid_t basegid);

/*
 * getgroups extension; not limited by NGROUPS_MAX
 */
int
getgroups(int gidsetsize, gid_t grouplist[])
{
    struct passwd *pw;
    int n;

    if ((pw = getpwuid(getuid())) == NULL) {
	errno = EINVAL;
	return -1;
    }
    if (gidsetsize == 0) {
	if ((n = getgroupcount(pw->pw_name, pw->pw_gid)) == 0) {
	    errno = EINVAL;
	    return -1;
	}
	return n;
    }
    n = gidsetsize;
    if (getgrouplist(pw->pw_name, pw->pw_gid, (int *)grouplist, &n) < 0) {
	errno = EINVAL;
	return -1;
    }
    return n;
}
