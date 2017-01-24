/*
 * Copyright (c) 2005, 2014 Apple Inc. All rights reserved.
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

// NOTE: This system call emulation should move to libsystem_kernel.dylib

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/attr.h>
#include <unistd.h>
#include <strings.h>

int
lchmod(const char *path, mode_t mode)
{
	struct attrlist a;
	int m;

	bzero(&a, sizeof(a));
	a.bitmapcount = ATTR_BIT_MAP_COUNT;
	a.commonattr = ATTR_CMN_ACCESSMASK;
	m = mode;
	return setattrlist(path, &a, &m, sizeof(int), FSOPT_NOFOLLOW);
}
