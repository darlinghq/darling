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
 * Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved
 */
/*
 * NeXT 386 setjmp/longjmp
 *
 * Written by Bruce Martin, NeXT Inc. 4/9/92
 */

/*
 * C library -- setjmp, longjmp
 *
 *	longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	setjmp(a)
 * by restoring registers from the stack,
 * The previous value of the signal mask is
 * restored.
 *
 */

#include <architecture/i386/asm_help.h>

// The FP control word is actually two bytes, but there's no harm in
// using four bytes for it and keeping the struct aligned.
#define JB_FPCW		0
#define JB_MASK		4
#define JB_MXCSR	8
#define JB_EBX		12
#define JB_ONSTACK	16
#define JB_EDX		20
#define JB_EDI		24
#define JB_ESI		28
#define JB_EBP		32
#define JB_ESP		36
#define JB_SS		40
#define JB_EFLAGS	44
#define JB_EIP		48
#define JB_CS		52
#define JB_DS		56
#define JB_ES		60
#define JB_FS		64
#define JB_GS		68
#define JB_SAVEMASK	72	// sigsetjmp/siglongjmp only

LEAF(_sigsetjmp, 0)
	movl	4(%esp), %eax 		// sigjmp_buf * jmpbuf; 
	movl	8(%esp), %ecx		// int savemask;
	movl	%ecx, JB_SAVEMASK(%eax)	// jmpbuf[_JBLEN] = savemask;
	cmpl	$0, %ecx		// if savemask != 0
	jne	_setjmp			//     setjmp(jmpbuf); 
	jmp L_do__setjmp		// else _setjmp(jmpbuf); 
	
LEAF(_setjmp, 0)
	subl	$16, %esp		// make space for return from sigprocmask 
							// + 12 to align stack
	pushl	%esp			// oset
	pushl	$0				// set = NULL
	pushl	$1				// how = SIG_BLOCK
	CALL_EXTERN(_sigprocmask)
	movl	12(%esp),%eax	// save the mask
	addl	$28, %esp		// restore original esp
	movl	4(%esp), %ecx		// jmp_buf (struct sigcontext *)
	movl	%eax, JB_MASK(%ecx)

	subl	$20, %esp		// temporary struct sigaltstack + 8 to 
							// align stack
	pushl	%esp			// oss
	pushl	$0			// ss == NULL
	CALL_EXTERN(_sigaltstack)	// get alternate signal stack info
	movl	16(%esp), %eax		// oss->ss_flags
	addl	$28, %esp			// Restore %esp
	movl	%eax, JB_ONSTACK(%ecx)

L_do__setjmp:
	BRANCH_EXTERN(__setjmp)

LEAF(_siglongjmp, 0)
	movl 4(%esp), %eax		// sigjmp_buf * jmpbuf; 
	cmpl $0, JB_SAVEMASK(%eax)	// if jmpbuf[_JBLEN] != 0
	jne 	_longjmp		//     longjmp(jmpbuf, var); 
	jmp L_do__longjmp		// else _longjmp(jmpbuf, var); 
	
LEAF(_longjmp, 0)
	movl	4(%esp), %ecx		// address of jmp_buf (saved context)
	movl	JB_MASK(%ecx),%eax	// get the mask
	subl	$12, %esp			// Make sure the stack is 16-byte 
								// aligned when we call sigprocmask
	pushl	%eax				// store the mask
	movl	%esp, %edx			// save the address where we stored the mask
	pushl	$0					// oset = NULL
	pushl	%edx				// set
	pushl	$3					// how = SIG_SETMASK
	CALL_EXTERN_AGAIN(_sigprocmask)
	addl	$28, %esp			// restore original esp

	movl	4(%esp), %ecx		// address of jmp_buf
	movl	JB_ONSTACK(%ecx), %eax	// ss_flags
	subl	$8, %esp
	pushl	%eax
	CALL_EXTERN(__sigunaltstack)
	addl	$12, %esp
	
L_do__longjmp:
	BRANCH_EXTERN(__longjmp)	// else
END(_longjmp)
