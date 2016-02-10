/*
 * Copyright (c) 2009, 2011 Apple Inc. All rights reserved.
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

#define	__APPLE_API_PRIVATE
#include <machine/cpu_capabilities.h>
#undef	__APPLE_API_PRIVATE

// This routine is never called directly by user code, jumped from kernel
// args 0 to 3 are already in the regs 0 to 3
// should set stack with the 2 extra args before calling pthread_wqthread()
// arg4 is in r[4]
// arg5 is in r[5]
  
        .text
        .align 2
        .globl _thread_start
_thread_start:
    stmfd sp!, {r4, r5}
	bl __pthread_start

// Stackshots will show the routine that happens to link immediately following
// _thread_start.  So we add an extra instruction (nop) to make stackshots
// more readable.
	nop
