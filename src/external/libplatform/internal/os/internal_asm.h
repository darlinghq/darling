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

#ifndef __INTERNAL_ASM_H__
#define __INTERNAL_ASM_H__

#include <machine/cpu_capabilities.h>

#define OS_STRINGIFY1(s) #s
#define OS_STRINGIFY(s) OS_STRINGIFY1(s)
#define OS_CONCAT1(x, y) x##y
#define OS_CONCAT(x, y) OS_CONCAT1(x,y)

#ifdef	__ASSEMBLER__
#define OS_VARIANT(f, v) OS_CONCAT(_,OS_CONCAT(f,OS_CONCAT($VARIANT$,v)))
#else
#define OS_VARIANT(f, v) OS_CONCAT(f,OS_CONCAT($VARIANT$,v))
#endif

#if defined(__ASSEMBLER__)

#if defined(__i386__) || defined(__x86_64__)

#define OS_VARIANT_FUNCTION_START(name, variant, alignment) \
	.text ; \
	.align alignment, 0x90 ; \
	.private_extern OS_VARIANT(name,variant) ; \
	OS_VARIANT(name,variant) ## :

// GENERIC indicates that this function will be chosen as the generic
// implementation (at compile time) when building targets which do not
// support dyld variant resolves.
#if defined(VARIANT_NO_RESOLVERS) || defined(VARIANT_DYLD)
#define OS_VARIANT_FUNCTION_START_GENERIC(name, variant, alignment) \
	OS_VARIANT_FUNCTION_START(name,variant,alignment) \
	.globl _ ## name ; \
	_ ## name ## :
#else
#define OS_VARIANT_FUNCTION_START_GENERIC OS_VARIANT_FUNCTION_START
#endif

#define OS_ATOMIC_FUNCTION_START(name, alignment) \
	.text ; \
	.align alignment, 0x90 ; \
	.globl _ ## name ; \
	_ ## name ## :

#endif // defined(__i386__) || defined(__x86_64__)

#endif // defined(__ASSEMBLER__)

#endif // __INTERNAL_ASM_H__
