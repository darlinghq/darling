/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _SECTION_KEYWORDS_H
#define _SECTION_KEYWORDS_H


/* Default behaviour */
#ifndef SECURITY_READ_ONLY_EARLY
#define __PLACE_IN_SECTION(__segment__section) \
	__attribute__((used, section(__segment__section)))

#define SECURITY_READ_ONLY_SPECIAL_SECTION(_t, __segment__section) \
	const _t __PLACE_IN_SECTION(__segment__section)

#define SECURITY_READ_ONLY_EARLY(_t) const _t

#define SECURITY_READ_ONLY_LATE(_t) _t

#define SECURITY_READ_WRITE(_t) _t __attribute__((used))
#endif /* SECURITY_READ_ONLY_EARLY */


#endif /* _SECTION_KEYWORDS_H_ */
