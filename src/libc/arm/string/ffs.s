/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#include <arm/arch.h>

#ifdef _ARM_ARCH_5
	.text

	.align	2
	.globl	_ffs
_ffs:
	.globl	_ffsl
_ffsl:
	rsb		r3, r0, #0
	and		r0, r0, r3
	clz		r0, r0
	rsb		r0, r0, #32
	bx		lr

	.align	2
	.globl _fls
_fls:
	.globl	_flsl
_flsl:
	clz		r0, r0
	rsb		r0, r0, #32
	bx		lr
#else
#error need to define ffs for this architecture
#endif
