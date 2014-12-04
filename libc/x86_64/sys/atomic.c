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
	PLATFUNC_DESCRIPTOR(symbol, up, kUP, 0); \
	PLATFUNC_DESCRIPTOR(symbol, mp, 0, kUP); \
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

RESOLVER_UP_MP(OSAtomicAnd32)
RESOLVER_UP_MP(OSAtomicAnd32Barrier)
RESOLVER_UP_MP(OSAtomicOr32)
RESOLVER_UP_MP(OSAtomicOr32Barrier)
RESOLVER_UP_MP(OSAtomicXor32)
RESOLVER_UP_MP(OSAtomicXor32Barrier)
RESOLVER_UP_MP(OSAtomicAnd32Orig)
RESOLVER_UP_MP(OSAtomicAnd32OrigBarrier)
RESOLVER_UP_MP(OSAtomicOr32Orig)
RESOLVER_UP_MP(OSAtomicOr32OrigBarrier)
RESOLVER_UP_MP(OSAtomicXor32Orig)
RESOLVER_UP_MP(OSAtomicXor32OrigBarrier)
RESOLVER_UP_MP(OSAtomicCompareAndSwapInt)
RESOLVER_UP_MP(OSAtomicCompareAndSwapIntBarrier)
RESOLVER_UP_MP(OSAtomicCompareAndSwap32)
RESOLVER_UP_MP(OSAtomicCompareAndSwap32Barrier)
RESOLVER_UP_MP(OSAtomicCompareAndSwapPtr)
RESOLVER_UP_MP(OSAtomicCompareAndSwapPtrBarrier)
RESOLVER_UP_MP(OSAtomicCompareAndSwapLong)
RESOLVER_UP_MP(OSAtomicCompareAndSwapLongBarrier)
RESOLVER_UP_MP(OSAtomicCompareAndSwap64)
RESOLVER_UP_MP(OSAtomicCompareAndSwap64Barrier)
RESOLVER_UP_MP(OSAtomicAdd32)
RESOLVER_UP_MP(OSAtomicAdd32Barrier)
RESOLVER_UP_MP(OSAtomicAdd64)
RESOLVER_UP_MP(OSAtomicAdd64Barrier)
RESOLVER_UP_MP(OSAtomicTestAndSet)
RESOLVER_UP_MP(OSAtomicTestAndSetBarrier)
RESOLVER_UP_MP(OSAtomicTestAndClear)
RESOLVER_UP_MP(OSAtomicTestAndClearBarrier)
