/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef _TZLINK_H_
#define _TZLINK_H_

/*!
 * @function tzlink
 * Create the timezone link at TZDEFAULT
 *
 * @param tz
 * New timezone, e.g. "America/Los_Angeles". This path is relative to TZDIR,
 * and must not contain any relative path components or stray slashes.
 * The file must exist and must be a valid timezone file with correct
 * ownership (root:wheel) and permissions (0644).
 *
 * @result
 * If the call succeeds, will return zero. Otherwise, returns an error:
 *   EINVAL: Invalid input, e.g. NULL or a path with relative components.
 *   ENAMETOOLONG: Input too long (generates a path > PATH_MAX)
 *   ENOENT: Specified file doesn't exist or fails owner/perm check.
 *   EPERM: Entitlement check failed.
 *   EIO: Failed to communicate with backing daemon.
 *   ENOTSUP: Always returned on OS X.
 * And possibly others not documented here.
 *
 * @discussion
 * This call can be used by any sufficiently-entitled client to overwrite
 * the timezone link at TZDEFAULT (see <tzfile.h>). It communicates with a
 * root daemon that does the necessary validation and file system work.
 * Upon success, the "SignificantTimeChangeNotification" notification is
 * posted.
 */
errno_t tzlink(const char *tz);

#endif /* !_TZLINK_H_ */
