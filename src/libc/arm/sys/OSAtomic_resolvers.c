/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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

/*
 * This holds the C versions of OSAtomic.s resolver functions, used by
 * everyone except dyld.
 */

#include <arm/arch.h>
#if defined(_ARM_ARCH_7)

#include <stdlib.h>
#include <machine/cpu_capabilities.h>
#include <mach/machine.h>

#if defined(VARIANT_DYLD)

#define _STRDEF(x) __STRING(x)

#define makeResolver_up_mp(name) \
	void* name ## $VARIANT$up(void); \
	void* name ## $VARIANT$mp(void); \
	void* name ## Resolver(void) __asm__( "_" #name ); \
	void* name ## Resolver(void) { \
		__asm__ __volatile__ ( \
			"ldr ip, L" #name "$commpage ;" \
			"ldr ip, [ip] ;" \
			"tst ip, $(" _STRDEF(kUP) ") ;" \
			"bne 1f ; " \
			"b   _" __STRING(name) "$VARIANT$mp ;" \
			"1: b   _" __STRING(name) "$VARIANT$up ;" \
			"L" #name "$commpage: .long " _STRDEF(_COMM_PAGE_CPU_CAPABILITIES) " ;" \
			: : : ); \
	}

#define makeResolver_up_mp_wfe(name) makeResolver_up_mp(name)

#else

#define makeResolver_up_mp(name) \
	void name ## $VARIANT$up(void); \
	void name ## $VARIANT$mp(void); \
	void* name ## Resolver(void) __asm__( "_" #name ) ; \
	void* name ## Resolver(void) { \
		__asm__(".symbol_resolver _" #name); \
		/* return MP variant functions on kNumCPUs > 1 */ \
		if ((*(uint32_t*)(uintptr_t)_COMM_PAGE_CPU_CAPABILITIES & kUP) == 0) { \
			return name ## $VARIANT$mp; \
		} \
		return name ## $VARIANT$up; \
	}

#define makeResolver_up_mp_wfe(name) \
	void name ## $VARIANT$up(void); \
	void name ## $VARIANT$mp(void); \
	void name ## $VARIANT$wfe(void); \
	void* name ## Resolver(void) __asm__( "_" #name ) ; \
	void* name ## Resolver(void) { \
		__asm__(".symbol_resolver _" #name); \
		uint32_t caps = *(uint32_t*)(uintptr_t)_COMM_PAGE_CPU_CAPABILITIES; \
		/* return WFE variant if we will get woken up */ \
		if ((caps & (kHasEvent | kUP)) == kHasEvent) { \
			return name ## $VARIANT$wfe; \
		} \
		/* return MP variant functions on kNumCPUs > 1 */ \
		else if ((caps & kUP) == 0) { \
			return name ## $VARIANT$mp; \
		} \
		return name ## $VARIANT$up; \
	}

#endif // defined VARIANT_DYLD

#include "OSAtomic_resolvers.h"

#else // defined _ARM_ARCH_7

typedef int emptyFilesArentCFiles;

#endif // defined _ARM_ARCH_7 && !defined VARIANT_DYLD
