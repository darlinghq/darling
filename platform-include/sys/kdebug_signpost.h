/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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
 * In previous versions of the operating system, applications could use:
 *
 * syscall(SYS_kdebug_trace, APPSDBG_CODE(DBG_MACH_CHUD, <your event code>) | DBG_FUNC_<type>, arg1, arg2, arg3, arg4);
 *
 * to record events that would be displayed by Instruments.
 *
 * syscall(2) is now deprecated and this interface replaces the above call as follows:
 *
 * The code argument is <your event code>.  Only the low 14-bits of the code are
 * preserved.
 */

/*
 * When <type> is NONE, use kdebug_signpost.
 */
int kdebug_signpost(uint32_t code, uintptr_t arg1, uintptr_t arg2,  uintptr_t arg3,  uintptr_t arg4)
	__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0);

/*
 * When <type> is START, use kdebug_signpost_start.
 */
int kdebug_signpost_start(uint32_t code, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
	__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0);

/*
 * When <type> is END, use kdebug_signpost_end.
 */
int kdebug_signpost_end(uint32_t code, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
	__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0);

#endif /* !KERNEL */

__END_DECLS

#endif /* !BSD_SYS_KDEBUG_SIGNPOST_H */
