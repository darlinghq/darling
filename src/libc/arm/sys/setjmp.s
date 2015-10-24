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
 *	File: sys/arm/setjmp.s
 *
 *	Implements sigsetjmp(), setjmp(), _setjmp()
 *
 */

#include <architecture/arm/asm_help.h>
#include "_setjmp.h"

/*
 * setjmp  routines
 */

/*	int sigsetjmp(sigjmp_buf env, int savemask); */

ENTRY_POINT(_sigsetjmp)
	str	r1, [ r0, #JMP_SIGFLAG ]	// save sigflag
	tst	r1, #0				// test if r1 is 0
	beq	L__exit				// if r1 == 0 do _setjmp()
	// else *** fall through ***  to setjmp()

/*	int setjmp(jmp_buf env); */

ENTRY_POINT(_setjmp)
	str	lr, [ r0, #JMP_lr ]
	str	r8, [ r0, #JMP_r8 ]
	mov	r8, r0
	mov	r0, #1				// get the previous signal mask
	mov	r1, #0				//
	add	r2, r8, #JMP_sig		// get address where previous mask needs to be
	CALL_EXTERNAL(_sigprocmask)		// make a syscall to get mask
	mov	r0, r8				// restore jmp_buf ptr
	ldr	r8, [ r0,  #JMP_r8 ] 
	ldr	lr, [ r0,  #JMP_lr ] 
L__exit:
	BRANCH_EXTERNAL(__setjmp)
