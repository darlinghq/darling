/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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

#include <sys/appleapiopts.h>
#include <machine/cpu_capabilities.h>
#include <platfunc.h>

/*
 * These platfunc routines provide fast access to the logical cpu number
 * of the calling processor assuming no pre-emption occurs. This number
 * is encoded in the bottom 12-bits of the limit field of the IDTR (the
 * Interrupt Descriptor Table Register). The SIDT instruction is used in
 * userspace to read this register and thus to gain access to the cpu number.
 * The IDTR is loaded by the kernel for each processor at startup - see
 * osfmk/i386/mp_desc.c.
 */

/* return logical cpu number in %eax */

	.align	4
	.private_extern _cpu_number
_cpu_number:
	push	%ebp
	mov	%esp,%ebp
	sub	$8, %esp		// space to read IDTR

	sidt	(%esp)			// store limit:base on stack
	movw	(%esp), %ax		// get limit 
	and	$0xfff, %eax		// mask off lower 12 bits to return

	mov	%ebp,%esp
	pop	%ebp
	ret
