/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef __GETHOSTUUID_PRIVATE_H
#define __GETHOSTUUID_PRIVATE_H

#include <sys/_types/_timespec.h>
#include <sys/_types/_uuid_t.h>
#include <Availability.h>

/* SPI prototype, TEMPORARY */
int _getprivatesystemidentifier(uuid_t uuid, const struct timespec *timeout) __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0);

/* internal prototype */
int gethostuuid(uuid_t, const struct timespec *) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

/* Callback should return -1 and set errno on failure */
int _register_gethostuuid_callback(int (*)(uuid_t)) __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0);

#endif /* __GETHOSTUUID_PRIVATE_H */
