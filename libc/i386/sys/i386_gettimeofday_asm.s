/*
 * Copyright (c) 2003-2007 Apple Inc. All rights reserved.
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

#define NSEC_PER_SEC	1000*1000*1000
#define NSEC_PER_USEC	1000

	.align	4
	.private_extern ___commpage_gettimeofday
___commpage_gettimeofday:
	push	%ebp
	mov	%esp,%ebp
	push	%esi
	push	%ebx

0:
	movl	_COMM_PAGE_GTOD_GENERATION,%esi	/* get generation (0 if disabled) */
	testl	%esi,%esi			/* disabled? */
	jz	4f

	call	_mach_absolute_time_direct
	
	sub	_COMM_PAGE_GTOD_NS_BASE,%eax
	sbb	_COMM_PAGE_GTOD_NS_BASE+4,%edx
	mov	_COMM_PAGE_GTOD_SEC_BASE,%ebx	/* load all the data before checking generation */
	mov	$ NSEC_PER_SEC,%ecx
	
	cmpl	_COMM_PAGE_GTOD_GENERATION,%esi	/* has time data changed out from under us? */
	jne	0b
	
	div	%ecx
	add	%eax,%ebx

	mov	$ NSEC_PER_USEC,%ecx
	mov	%edx,%eax
	xor	%edx,%edx
	div	%ecx

	mov	8(%ebp),%ecx
	mov	%ebx,(%ecx)
	mov	%eax,4(%ecx)
	xor	%eax,%eax

3:
	pop	%ebx
	pop	%esi
	pop	%ebp
	ret
4:				/* fail */
	movl	$1,%eax
	jmp	3b
