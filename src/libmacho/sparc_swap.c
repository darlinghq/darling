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
#include <string.h>
#import <mach-o/sparc/swap.h>
#import <architecture/nrw/reg_help.h>

void
swap_sparc_thread_state_regs(
struct sparc_thread_state_regs *cpu,
enum NXByteOrder target_byte_sex)
{
    struct swapped_psr {
	union {
	    struct {
		unsigned int 
		cwp:BITS_WIDTH(4,0),
		et:BIT_WIDTH(5),
		ps:BIT_WIDTH(6),
		s:BIT_WIDTH(7),
		pil:BITS_WIDTH(11,8),
		ef:BIT_WIDTH(12),
		ec:BIT_WIDTH(13),
		reserved:BITS_WIDTH(19,14),
		icc:BITS_WIDTH(23,20),
		ver:BITS_WIDTH(27,24),
		impl:BITS_WIDTH(31,28);
	    } fields;
	    unsigned int word;
	} u;
    } spsr;
    struct p_status *pr_status;
    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();

	cpu->regs.r_pc = OSSwapInt32(cpu->regs.r_pc);
	cpu->regs.r_npc = OSSwapInt32(cpu->regs.r_npc);
	cpu->regs.r_y = OSSwapInt32(cpu->regs.r_y);
	cpu->regs.r_g1 = OSSwapInt32(cpu->regs.r_g1);
	cpu->regs.r_g2 = OSSwapInt32(cpu->regs.r_g2);
	cpu->regs.r_g3 = OSSwapInt32(cpu->regs.r_g3);
	cpu->regs.r_g4 = OSSwapInt32(cpu->regs.r_g4);
	cpu->regs.r_g5 = OSSwapInt32(cpu->regs.r_g5);
	cpu->regs.r_g6 = OSSwapInt32(cpu->regs.r_g6);
	cpu->regs.r_g7 = OSSwapInt32(cpu->regs.r_g7);
	cpu->regs.r_o0 = OSSwapInt32(cpu->regs.r_o0);
	cpu->regs.r_o1 = OSSwapInt32(cpu->regs.r_o1);
	cpu->regs.r_o2 = OSSwapInt32(cpu->regs.r_o2);
	cpu->regs.r_o3 = OSSwapInt32(cpu->regs.r_o3);
	cpu->regs.r_o4 = OSSwapInt32(cpu->regs.r_o4);
	cpu->regs.r_o5 = OSSwapInt32(cpu->regs.r_o5);
	cpu->regs.r_o6 = OSSwapInt32(cpu->regs.r_o6);
	cpu->regs.r_o7 = OSSwapInt32(cpu->regs.r_o7);

    	pr_status = (struct p_status *) &(cpu->regs.r_psr);
	if(target_byte_sex == host_byte_sex){
	    memcpy(&spsr, &(cpu->regs.r_psr), sizeof(struct swapped_psr));
	    spsr.u.word = OSSwapInt32(spsr.u.word);
	    pr_status->PSRREG.psr_bits.cwp = spsr.u.fields.cwp;
	    pr_status->PSRREG.psr_bits.ps = spsr.u.fields.ps;
	    pr_status->PSRREG.psr_bits.s = spsr.u.fields.s;
	    pr_status->PSRREG.psr_bits.pil = spsr.u.fields.pil;
	    pr_status->PSRREG.psr_bits.ef = spsr.u.fields.ef;
	    pr_status->PSRREG.psr_bits.ec = spsr.u.fields.ec;
	    pr_status->PSRREG.psr_bits.reserved = spsr.u.fields.reserved;
	    pr_status->PSRREG.psr_bits.icc = spsr.u.fields.icc;
	    pr_status->PSRREG.psr_bits.et = spsr.u.fields.ver;
	    pr_status->PSRREG.psr_bits.impl = spsr.u.fields.impl;
	}
	else{
	    spsr.u.fields.cwp = pr_status->PSRREG.psr_bits.cwp;
	    spsr.u.fields.ps = pr_status->PSRREG.psr_bits.ps;
	    spsr.u.fields.s = pr_status->PSRREG.psr_bits.s;
	    spsr.u.fields.pil = pr_status->PSRREG.psr_bits.pil;
	    spsr.u.fields.ef = pr_status->PSRREG.psr_bits.ef;
	    spsr.u.fields.ec = pr_status->PSRREG.psr_bits.ec;
	    spsr.u.fields.reserved = pr_status->PSRREG.psr_bits.reserved;
	    spsr.u.fields.icc = pr_status->PSRREG.psr_bits.icc;
	    spsr.u.fields.ver = pr_status->PSRREG.psr_bits.et;
	    spsr.u.fields.impl = pr_status->PSRREG.psr_bits.impl;
	    spsr.u.word = OSSwapInt32(spsr.u.word);
	    memcpy(&(cpu->regs.r_psr), &spsr, sizeof(struct swapped_psr));
	}
}

void
swap_sparc_thread_state_fpu(
struct sparc_thread_state_fpu *fpu,
enum NXByteOrder target_byte_sex)
{
    struct swapped_fsr {
	union {
	    struct {
		unsigned int
		cexc:BITS_WIDTH(4,0),
		aexc:BITS_WIDTH(9,5),
		fcc:BITS_WIDTH(11,10),
		pr:BIT_WIDTH(12),
		qne:BIT_WIDTH(13),
		ftt:BITS_WIDTH(16,14),
		res:BITS_WIDTH(22,17),
		tem:BITS_WIDTH(27,23),
		rp:BITS_WIDTH(29,28),
		rd:BITS_WIDTH(31,30);
	    } fields;
	    unsigned int word;
	} u;
    } sfsr;
    uint32_t i;
    struct f_status *fpu_status;
    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();

	
	/* floating point registers */
	for(i = 0; i < 16; i++)		/* 16 doubles */
	    fpu->fpu.fpu_fr.Fpu_dregs[i] =
		OSSwapInt64(fpu->fpu.fpu_fr.Fpu_dregs[i]);

	fpu->fpu.Fpu_q[0].FQu.whole = OSSwapInt64(fpu->fpu.Fpu_q[0].FQu.whole);
	fpu->fpu.Fpu_q[1].FQu.whole = OSSwapInt64(fpu->fpu.Fpu_q[1].FQu.whole);
	fpu->fpu.Fpu_flags = OSSwapInt32(fpu->fpu.Fpu_flags);
	fpu->fpu.Fpu_extra = OSSwapInt32(fpu->fpu.Fpu_extra);
	fpu->fpu.Fpu_qcnt = OSSwapInt32(fpu->fpu.Fpu_qcnt);

	fpu_status = (struct f_status *) &(fpu->fpu.Fpu_fsr);
	if(target_byte_sex == host_byte_sex){
	    memcpy(&sfsr, &(fpu->fpu.Fpu_fsr), sizeof(unsigned int));
	    sfsr.u.word = OSSwapInt32(sfsr.u.word);
	    fpu_status->FPUREG.Fpu_fsr_bits.rd = sfsr.u.fields.rd;
	    fpu_status->FPUREG.Fpu_fsr_bits.rp = sfsr.u.fields.rp;
	    fpu_status->FPUREG.Fpu_fsr_bits.tem = sfsr.u.fields.tem;
	    fpu_status->FPUREG.Fpu_fsr_bits.res = sfsr.u.fields.res;
	    fpu_status->FPUREG.Fpu_fsr_bits.ftt = sfsr.u.fields.ftt;
	    fpu_status->FPUREG.Fpu_fsr_bits.qne = sfsr.u.fields.qne;
	    fpu_status->FPUREG.Fpu_fsr_bits.pr = sfsr.u.fields.pr;
	    fpu_status->FPUREG.Fpu_fsr_bits.fcc = sfsr.u.fields.fcc;
	    fpu_status->FPUREG.Fpu_fsr_bits.aexc = sfsr.u.fields.aexc;
	    fpu_status->FPUREG.Fpu_fsr_bits.cexc = sfsr.u.fields.cexc;
	}
	else{
	    sfsr.u.fields.rd = fpu_status->FPUREG.Fpu_fsr_bits.rd;
	    sfsr.u.fields.rp = fpu_status->FPUREG.Fpu_fsr_bits.rp;
	    sfsr.u.fields.tem = fpu_status->FPUREG.Fpu_fsr_bits.tem;
	    sfsr.u.fields.res = fpu_status->FPUREG.Fpu_fsr_bits.res;
	    sfsr.u.fields.ftt = fpu_status->FPUREG.Fpu_fsr_bits.ftt;
	    sfsr.u.fields.qne = fpu_status->FPUREG.Fpu_fsr_bits.qne;
	    sfsr.u.fields.pr = fpu_status->FPUREG.Fpu_fsr_bits.pr;
	    sfsr.u.fields.fcc = fpu_status->FPUREG.Fpu_fsr_bits.fcc;
	    sfsr.u.fields.aexc = fpu_status->FPUREG.Fpu_fsr_bits.aexc;
	    sfsr.u.fields.cexc = fpu_status->FPUREG.Fpu_fsr_bits.cexc;
	    sfsr.u.word = OSSwapInt32(sfsr.u.word);
	    memcpy(&(fpu->fpu.Fpu_fsr), &sfsr, sizeof(struct swapped_fsr));
	}
}
#endif /* !defined(RLD) */
