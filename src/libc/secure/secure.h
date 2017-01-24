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

#ifndef _SECURE_H_

#include <sys/types.h>

extern void __chk_fail_overflow (void) __attribute__((__noreturn__));
extern void __chk_fail_overlap (void) __attribute__((__noreturn__));

/* Assert if a -> a+an and b -> b+bn overlap.
 * 0-lengths don't overlap anything.
 */
extern void __chk_overlap (const void *a, size_t an, const void *b, size_t bn);

/* Do we avoid the overlap check for older APIs? */
extern uint32_t __chk_assert_no_overlap;

#endif
