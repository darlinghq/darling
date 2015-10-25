/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

#ifndef	_SYS_KAS_INFO_H_
#define	_SYS_KAS_INFO_H_

#include <sys/cdefs.h>
#include <Availability.h>

/*
 * kas_info() ("Kernel Address Space Info") is a private interface that allows
 * appropriately privileged system components to introspect the overall
 * kernel address space layout.
 */

__BEGIN_DECLS

/* The slide of the main kernel compared to its static link address */
#define	KAS_INFO_KERNEL_TEXT_SLIDE_SELECTOR	(0)	/* returns uint64_t	*/
#define	KAS_INFO_MAX_SELECTOR		(1)

#ifndef KERNEL

int kas_info(int selector, void *value, size_t *size) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_NA);

#endif /* KERNEL */

__END_DECLS

#endif	/* !_SYS_KAS_INFO_H_ */
