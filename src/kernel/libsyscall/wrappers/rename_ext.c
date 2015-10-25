/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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
 #include <errno.h>
 
/*
 * XXXXX REMOVE AND REPLACE the definition of VFS_RENAME_FLAGS_MASK below with 
 * appropriate header file if/when defined in a header file.
 */
#define VFS_RENAME_FLAGS_MASK 0x00000001

void __inc_remove_counter(void);
int __rename_ext(const char *old, const char *new, int flags);

int
rename_ext(const char *old, const char *new, unsigned int flags)
{
	if  (!(flags & VFS_RENAME_FLAGS_MASK)) {
		errno = EINVAL;
 		return -1;
 	}
 	
	int res = __rename_ext(old, new, flags & VFS_RENAME_FLAGS_MASK);
	if (res == 0) __inc_remove_counter();
	return res;
}
