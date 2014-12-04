/*
 * Copyright (c) 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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

#include <machine/cpu_capabilities.h>
#include "platfunc.h"

#define	RESOLVER_UP_MP(symbol)	\
	PLATFUNC_DESCRIPTOR_PROTOTYPE(symbol, up); \
	PLATFUNC_DESCRIPTOR_PROTOTYPE(symbol, mp); \
	static const platfunc_descriptor* symbol ## _platfunc_descriptors[] = { \
		PLATFUNC_DESCRIPTOR_REFERENCE(symbol, mp), \
		PLATFUNC_DESCRIPTOR_REFERENCE(symbol, up), \
		0 \
	}; \
	void* symbol ## _chooser() __asm__("_" #symbol); \
	void* symbol ## _chooser() { \
		__asm__(".symbol_resolver _" #symbol); \
		return find_platform_function((const platfunc_descriptor**) symbol ## _platfunc_descriptors); \
	}

RESOLVER_UP_MP(OSSpinLockTry)
RESOLVER_UP_MP(_spin_lock_try)
RESOLVER_UP_MP(OSSpinLockLock)
RESOLVER_UP_MP(_spin_lock)
RESOLVER_UP_MP(spin_lock)
