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
 *
 * HISTORY
 *  20-Apr-92    Bruce Martin (bmartin@next.com)
 *      Created from M68K sources.
 */

/*
 * C library -- _setjmp, _longjmp
 *
 *	_longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	_setjmp(a)
 * by restoring registers from the stack,
 * The previous signal state is NOT restored.
 *
 */

#include <architecture/i386/asm_help.h>

// The FP control word is actually two bytes, but there's no harm in
// using four bytes for it and keeping the struct aligned.
#define JB_FPCW         0
#define JB_MASK         4
#define JB_MXCSR        8
#define JB_EBX          12
#define JB_ONSTACK      16
#define JB_EDX          20
#define JB_EDI          24
#define JB_ESI          28
#define JB_EBP          32
#define JB_ESP          36
#define JB_SS           40
#define JB_EFLAGS       44
#define JB_EIP          48
#define JB_CS           52
#define JB_DS           56
#define JB_ES           60
#define JB_FS           64
#define JB_GS           68

LEAF(__setjmp, 0)
        movl    4(%esp), %ecx           // jmp_buf (struct sigcontext *)

        // Build the jmp_buf
        fnstcw  JB_FPCW(%ecx)			// Save the FP control word
        stmxcsr JB_MXCSR(%ecx)			// Save the MXCSR
        movl    %ebx, JB_EBX(%ecx)
        movl    %edi, JB_EDI(%ecx)
        movl    %esi, JB_ESI(%ecx)
        movl    %ebp, JB_EBP(%ecx)

        // EIP is set to the frame return address value
        movl    (%esp), %eax
        movl    %eax, JB_EIP(%ecx)
        // ESP is set to the frame return address plus 4
        leal    4(%esp), %eax
        movl    %eax, JB_ESP(%ecx)

        // return 0
        xorl    %eax, %eax
        ret


LEAF(__longjmp, 0)
	fninit				// Clear all FP exceptions
	movl    4(%esp), %ecx           // jmp_buf (struct sigcontext *)
	movl	8(%esp), %eax		// return value
	testl	%eax, %eax
	jnz 1f
	incl    %eax

	// general registers
1:	movl	JB_EBX(%ecx), %ebx
	movl	JB_ESI(%ecx), %esi
	movl	JB_EDI(%ecx), %edi
	movl	JB_EBP(%ecx), %ebp
	movl	JB_ESP(%ecx), %esp

	fldcw	JB_FPCW(%ecx)			// Restore FP control word
	ldmxcsr JB_MXCSR(%ecx)			// Restore the MXCSR

	cld					// Make sure DF is reset
	jmp	*JB_EIP(%ecx)
