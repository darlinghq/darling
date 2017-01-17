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
#ifndef RLD

#define __cr cr
#define __ctr ctr
#define __dar dar
#define __dsisr dsisr
#define __exception exception
#define __fpregs fpregs
#define __fpscr fpscr
#define __fpscr_pad fpscr_pad
#define __lr lr
#define __mq mq
#define __pad0 pad0
#define __pad1 pad1
#define __r0 r0
#define __r1 r1
#define __r10 r10
#define __r11 r11
#define __r12 r12
#define __r13 r13
#define __r14 r14
#define __r15 r15
#define __r16 r16
#define __r17 r17
#define __r18 r18
#define __r19 r19
#define __r2 r2
#define __r20 r20
#define __r21 r21
#define __r22 r22
#define __r23 r23
#define __r24 r24
#define __r25 r25
#define __r26 r26
#define __r27 r27
#define __r28 r28
#define __r29 r29
#define __r3 r3
#define __r30 r30
#define __r31 r31
#define __r4 r4
#define __r5 r5
#define __r6 r6
#define __r7 r7
#define __r8 r8
#define __r9 r9
#define __srr0 srr0
#define __srr1 srr1
#define __vrsave vrsave
#define __xer xer

#import <mach-o/ppc/swap.h>
#import <architecture/nrw/reg_help.h>

void
swap_ppc_thread_state_t(
ppc_thread_state_t *cpu,
enum NXByteOrder target_byte_sex)
{
	cpu->srr0 = OSSwapInt32(cpu->srr0);
	cpu->srr1 = OSSwapInt32(cpu->srr1);
	cpu->r0 = OSSwapInt32(cpu->r0);
	cpu->r1 = OSSwapInt32(cpu->r1);
	cpu->r2 = OSSwapInt32(cpu->r2);
	cpu->r3 = OSSwapInt32(cpu->r3);
	cpu->r4 = OSSwapInt32(cpu->r4);
	cpu->r5 = OSSwapInt32(cpu->r5);
	cpu->r6 = OSSwapInt32(cpu->r6);
	cpu->r7 = OSSwapInt32(cpu->r7);
	cpu->r8 = OSSwapInt32(cpu->r8);
	cpu->r9 = OSSwapInt32(cpu->r9);
	cpu->r10 = OSSwapInt32(cpu->r10);
	cpu->r11 = OSSwapInt32(cpu->r11);
	cpu->r12 = OSSwapInt32(cpu->r12);
	cpu->r13 = OSSwapInt32(cpu->r13);
	cpu->r14 = OSSwapInt32(cpu->r14);
	cpu->r15 = OSSwapInt32(cpu->r15);
	cpu->r16 = OSSwapInt32(cpu->r16);
	cpu->r17 = OSSwapInt32(cpu->r17);
	cpu->r18 = OSSwapInt32(cpu->r18);
	cpu->r19 = OSSwapInt32(cpu->r19);
	cpu->r20 = OSSwapInt32(cpu->r20);
	cpu->r21 = OSSwapInt32(cpu->r21);
	cpu->r22 = OSSwapInt32(cpu->r22);
	cpu->r23 = OSSwapInt32(cpu->r23);
	cpu->r24 = OSSwapInt32(cpu->r24);
	cpu->r25 = OSSwapInt32(cpu->r25);
	cpu->r26 = OSSwapInt32(cpu->r26);
	cpu->r27 = OSSwapInt32(cpu->r27);
	cpu->r28 = OSSwapInt32(cpu->r28);
	cpu->r29 = OSSwapInt32(cpu->r29);
	cpu->r30 = OSSwapInt32(cpu->r30);
	cpu->r31 = OSSwapInt32(cpu->r31);
	cpu->lr  = OSSwapInt32(cpu->lr);
	cpu->cr  = OSSwapInt32(cpu->cr);
	cpu->xer = OSSwapInt32(cpu->xer);
	cpu->ctr = OSSwapInt32(cpu->ctr);
	cpu->mq  = OSSwapInt32(cpu->mq);
	cpu->vrsave = OSSwapInt32(cpu->vrsave);

}

void
swap_ppc_float_state_t(
ppc_float_state_t *fpu,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;
	
	for(i = 0; i < 32; i++)
	    fpu->fpregs[i] = OSSwapInt64(fpu->fpregs[i]);

	fpu->fpscr_pad = OSSwapInt32(fpu->fpscr_pad);
	fpu->fpscr = OSSwapInt32(fpu->fpscr);
}

void
swap_ppc_exception_state_t(
ppc_exception_state_t *state,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;
	
	state->dar = OSSwapInt32(state->dar);
	state->dsisr = OSSwapInt32(state->dsisr);
	state->exception = OSSwapInt32(state->exception);
	state->pad0 = OSSwapInt32(state->pad0);

	for(i = 0; i < 4; i++)
	    state->pad1[i] = OSSwapInt32(state->pad1[i]);
}
#endif /* !defined(RLD) */
