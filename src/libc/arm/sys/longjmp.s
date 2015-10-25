/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1998-2008 Apple Inc. All rights reserved.
 *
 *	File: sys/arm/longjmp.s
 *
 *	Implements siglongjmp(), longjmp(), _longjmp() 
 *
 */

#include <architecture/arm/asm_help.h>
#include "_setjmp.h"

/*
 *	longjmp routines
 */

/*	void siglongjmp(sigjmp_buf env, int val); */

ENTRY_POINT(_siglongjmp)
	ldr	r2, [ r0, #JMP_SIGFLAG ]	//  load sigflag
	tst	r2, #0				// test if zero
	beq	L__exit				// if zero do _longjmp()
	// else *** fall through *** to longjmp()

/*	void longjmp(jmp_buf env, int val); */

ENTRY_POINT(_longjmp)
	mov	r6, r0				// preserve args across _sigsetmask
	mov	r8, r1
	ldr	r0, [ r0, #JMP_sig ]		// restore the signal mask
	CALL_EXTERNAL(_sigsetmask)		// make a (deprecated!) syscall to set the mask
	mov	r1, r8
	mov	r0, r6
L__exit:
	BRANCH_EXTERNAL(__longjmp)
