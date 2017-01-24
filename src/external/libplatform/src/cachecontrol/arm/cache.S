/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
    .text
    .align 2

#include <mach/arm/syscall_sw.h>

/* void sys_icache_invalidate(addr_t start, int length) */
.globl	_sys_icache_invalidate
_sys_icache_invalidate:
	/* fast trap for icache_invalidate */
	mov	r3, #0
	mov	r12, #0x80000000
	swi	#SWI_SYSCALL
	bx	lr

/* void sys_dcache_flush(addr_t start, int length) */
.globl	_sys_dcache_flush
_sys_dcache_flush:
	/* fast trap for dcache_flush */
	mov	r3, #1
	mov	r12, #0x80000000
	swi	#SWI_SYSCALL
	bx	lr
