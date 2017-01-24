/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

#define	JMP_r19_20	#0x00
#define	JMP_r21_22	#0x10
#define	JMP_r23_24	#0x20
#define	JMP_r25_26	#0x30
#define	JMP_r27_28	#0x40
#define	JMP_r29_lr	#0x50
#define	JMP_fp_sp	#0x60

#define	JMP_d8_d9	#0x70
#define	JMP_d10_d11	#0x80
#define	JMP_d12_d13	#0x90
#define	JMP_d14_d15	#0xA0
#define	JMP_sig		#0xB0
#define	JMP_sigflag	#0xB8

#include <architecture/arm/asm_help.h>

/* int _setjmp(jmp_buf env); */
ENTRY_POINT(__setjmp)
	add		x1, sp, #0	/* can't STP from sp */
	stp		x19, x20,	[x0, JMP_r19_20]
	stp		x21, x22,	[x0, JMP_r21_22]
	stp		x23, x24,	[x0, JMP_r23_24]
	stp		x25, x26,	[x0, JMP_r25_26]
	stp		x27, x28,	[x0, JMP_r27_28]
	stp		x29, lr,	[x0, JMP_r29_lr]
	stp		fp, x1,		[x0, JMP_fp_sp]
	stp		d8, d9,		[x0, JMP_d8_d9]
	stp		d10, d11,	[x0, JMP_d10_d11]
	stp		d12, d13,	[x0, JMP_d12_d13]
	stp		d14, d15,	[x0, JMP_d14_d15]
	mov		x0, #0
	ret

/* void _longjmp(jmp_buf env, int val); */
ENTRY_POINT(__longjmp)
	ldp		x19, x20,	[x0, JMP_r19_20]
	ldp		x21, x22,	[x0, JMP_r21_22]
	ldp		x23, x24,	[x0, JMP_r23_24]
	ldp		x25, x26,	[x0, JMP_r25_26]
	ldp		x27, x28,	[x0, JMP_r27_28]
	ldp		x29, lr,	[x0, JMP_r29_lr]
	ldp		fp, x2,		[x0, JMP_fp_sp]
	ldp		d8, d9,		[x0, JMP_d8_d9]
	ldp		d10, d11,	[x0, JMP_d10_d11]
	ldp		d12, d13,	[x0, JMP_d12_d13]
	ldp		d14, d15,	[x0, JMP_d14_d15]
	add		sp, x2, #0
	mov		x0, x1
	cmp		x0, #0		/* longjmp returns 1 if val is 0 */
	b.ne	1f
	add		x0, x0, #1
1:	ret

/* int sigsetjmp(sigjmp_buf env, int savemask); */
ENTRY_POINT(_sigsetjmp)
	str		x1, [x0, JMP_sigflag]
	cmp		x1, #0
	b.ne	1f
	b		__setjmp
1:
	/* else, fall through */

/* int setjmp(jmp_buf env); */
ENTRY_POINT(_setjmp)
	stp		x21, lr, [x0]
	mov		x21, x0
	
	mov		x0, #1
	mov		x1, #0
	add		x2, x21, JMP_sig
	CALL_EXTERNAL(_sigprocmask)

	mov		x0, x21
	ldp		x21, lr, [x0]
	b		__setjmp

/* void siglongjmp(sigjmp_buf env, int val); */
ENTRY_POINT(_siglongjmp)
	ldr		x2, [x0, JMP_sigflag]
	cmp		x2, #0
	b.ne	1f
	b		__longjmp
1:
	/* else, fall through */

/* void longjmp(jmp_buf env, int val); */
ENTRY_POINT(_longjmp)
	sub     sp, sp, #16
	mov		x21, x0					// x21/x22 will be restored by __longjmp
	mov		x22, x1
	ldr		x0, [x21, JMP_sig]		// restore the signal mask
	str     x0, [sp, #8]
	add     x1, sp, #8				// set
	orr     w0, wzr, #0x3			// SIG_SETMASK
	movz	x2, #0					// oset
	CALL_EXTERNAL(_sigprocmask)
	mov		x0, x21
	mov		x1, x22
	add     sp, sp, #16
	b		__longjmp
