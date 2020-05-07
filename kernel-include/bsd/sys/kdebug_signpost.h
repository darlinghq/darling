/*
 * Copyright (c) 2016-2019 Apple Inc. All rights reserved.
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

#ifndef BSD_SYS_KDEBUG_SIGNPOST_H
#define BSD_SYS_KDEBUG_SIGNPOST_H

#include <Availability.h>
#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

#ifndef KERNEL

/*
 * kdebug_signpost(2) is deprecated.  Use the os_signpost(3) family of tracing
 * functions, instead.
 */

int kdebug_signpost(uint32_t code, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4)
__API_DEPRECATED_WITH_REPLACEMENT("os_signpost_event_emit",
    macos(10.12, 10.15), ios(10.0, 13.0), watchos(3.0, 6.0), tvos(10.0, 13.0));

int kdebug_signpost_start(uint32_t code, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4)
__API_DEPRECATED_WITH_REPLACEMENT("os_signpost_interval_begin",
    macos(10.12, 10.15), ios(10.0, 13.0), watchos(3.0, 6.0), tvos(10.0, 13.0));

int kdebug_signpost_end(uint32_t code, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4)
__API_DEPRECATED_WITH_REPLACEMENT("os_signpost_interval_end",
    macos(10.12, 10.15), ios(10.0, 13.0), watchos(3.0, 6.0), tvos(10.0, 13.0));

#endif /* !KERNEL */

__END_DECLS

#endif /* !BSD_SYS_KDEBUG_SIGNPOST_H */
