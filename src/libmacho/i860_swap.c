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
#import <mach-o/i860/swap.h>

void
swap_i860_thread_state_regs(
struct i860_thread_state_regs *cpu,
enum NXByteOrder target_byte_sex)
{
    int i;

	for(i = 0; i < 31; i++)
	    cpu->ireg[i] = OSSwapInt32(cpu->ireg[i]);
	for(i = 0; i < 30; i++)
	    cpu->freg[i] = OSSwapInt32(cpu->freg[i]);
	cpu->psr = OSSwapInt32(cpu->psr);
	cpu->epsr = OSSwapInt32(cpu->epsr);
	cpu->db = OSSwapInt32(cpu->db);
	cpu->pc = OSSwapInt32(cpu->pc);
	cpu->_padding_ = OSSwapInt32(cpu->_padding_);
	cpu->Mres3 = OSSwapInt64(cpu->Mres3);
	cpu->Ares3 = OSSwapInt64(cpu->Ares3);
	cpu->Mres2 = OSSwapInt64(cpu->Mres2);
	cpu->Ares2 = OSSwapInt64(cpu->Ares2);
	cpu->Mres1 = OSSwapInt64(cpu->Mres1);
	cpu->Ares1 = OSSwapInt64(cpu->Ares1);
	cpu->Ires1 = OSSwapInt64(cpu->Ires1);
	cpu->Lres3m = OSSwapInt64(cpu->Lres3m);
	cpu->Lres2m = OSSwapInt64(cpu->Lres2m);
	cpu->Lres1m = OSSwapInt64(cpu->Lres1m);
	cpu->KR = OSSwapInt64(cpu->KR);
	cpu->KI = OSSwapInt64(cpu->KI);
	cpu->T = OSSwapInt64(cpu->T);
	cpu->Fsr3 = OSSwapInt32(cpu->Fsr3);
	cpu->Fsr2 = OSSwapInt32(cpu->Fsr2);
	cpu->Fsr1 = OSSwapInt32(cpu->Fsr1);
	cpu->Mergelo32 = OSSwapInt32(cpu->Mergelo32);
	cpu->Mergehi32 = OSSwapInt32(cpu->Mergehi32);
}
#endif /* !defined(RLD) */
