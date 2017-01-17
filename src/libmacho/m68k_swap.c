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
#import <mach-o/m68k/swap.h>

void
swap_m68k_thread_state_regs(
struct m68k_thread_state_regs *cpu,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < 8; i++)
	    cpu->dreg[i] = OSSwapInt32(cpu->dreg[i]);
	for(i = 0; i < 8; i++)
	    cpu->areg[i] = OSSwapInt32(cpu->areg[i]);
	cpu->pad0 = OSSwapInt16(cpu->pad0);
	cpu->sr = OSSwapInt16(cpu->sr);
	cpu->pc = OSSwapInt32(cpu->pc);
}

void
swap_m68k_thread_state_68882(
struct m68k_thread_state_68882 *fpu,
enum NXByteOrder target_byte_sex)
{
    uint32_t i, tmp;

	for(i = 0; i < 8; i++){
	                   tmp = OSSwapInt32(fpu->regs[i].fp[0]);
	    fpu->regs[i].fp[1] = OSSwapInt32(fpu->regs[i].fp[1]);
	    fpu->regs[i].fp[0] = OSSwapInt32(fpu->regs[i].fp[2]);
	    fpu->regs[i].fp[2] = tmp;
	}
	fpu->cr = OSSwapInt32(fpu->cr);
	fpu->sr = OSSwapInt32(fpu->sr);
	fpu->iar = OSSwapInt32(fpu->iar);
	fpu->state = OSSwapInt32(fpu->state);
}

void
swap_m68k_thread_state_user_reg(
struct m68k_thread_state_user_reg *user_reg,
enum NXByteOrder target_byte_sex)
{
	user_reg->user_reg = OSSwapInt32(user_reg->user_reg);
}
#endif /* !defined(RLD) */
