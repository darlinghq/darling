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
#import <mach-o/m88k/swap.h>
#import <architecture/nrw/reg_help.h>

void
swap_m88k_thread_state_grf_t(
m88k_thread_state_grf_t *cpu,
enum NXByteOrder target_byte_sex)
{
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
	cpu->xip = OSSwapInt32(cpu->xip);
	cpu->xip_in_bd = OSSwapInt32(cpu->xip_in_bd);
	cpu->nip = OSSwapInt32(cpu->nip);
}

void
swap_m88k_thread_state_xrf_t(
m88k_thread_state_xrf_t *fpu,
enum NXByteOrder target_byte_sex)
{
    enum NXByteOrder host_byte_sex;

    struct swapped_m88k_fpsr {
	union {
	    struct {
		unsigned	afinx:BIT_WIDTH(0);
		unsigned	afovf:BIT_WIDTH(1);
		unsigned	afunf:BIT_WIDTH(2);
		unsigned	afdvz:BIT_WIDTH(3);
		unsigned	afinv:BIT_WIDTH(4);
		unsigned	:BITS_WIDTH(15,5);
		unsigned	xmod:BIT_WIDTH(16);
		unsigned	:BITS_WIDTH(31,17);
	    } fields;
	    uint32_t word;
	} u;
    } ssr;
    struct swapped_m88k_fpcr {
	union {
	    struct {
		unsigned	efinx:BIT_WIDTH(0);
		unsigned	efovf:BIT_WIDTH(1);
		unsigned	efunf:BIT_WIDTH(2);
		unsigned	efdvz:BIT_WIDTH(3);
		unsigned	efinv:BIT_WIDTH(4);
		unsigned	:BITS_WIDTH(13,5);
		m88k_fpcr_rm_t	rm:BITS_WIDTH(15,14);
		unsigned	:BITS_WIDTH(31,16);
	    } fields;
	    uint32_t word;
	} u;
    } scr;

	host_byte_sex = NXHostByteOrder();

	fpu->x1.x[0] = OSSwapInt32(fpu->x1.x[0]);
	fpu->x1.x[1] = OSSwapInt32(fpu->x1.x[1]);
	fpu->x1.x[2] = OSSwapInt32(fpu->x1.x[2]);
	fpu->x1.x[3] = OSSwapInt32(fpu->x1.x[3]);
	fpu->x2.x[0] = OSSwapInt32(fpu->x2.x[0]);
	fpu->x2.x[1] = OSSwapInt32(fpu->x2.x[1]);
	fpu->x2.x[2] = OSSwapInt32(fpu->x2.x[2]);
	fpu->x2.x[3] = OSSwapInt32(fpu->x2.x[3]);
	fpu->x3.x[0] = OSSwapInt32(fpu->x3.x[0]);
	fpu->x3.x[1] = OSSwapInt32(fpu->x3.x[1]);
	fpu->x3.x[2] = OSSwapInt32(fpu->x3.x[2]);
	fpu->x3.x[3] = OSSwapInt32(fpu->x3.x[3]);
	fpu->x4.x[0] = OSSwapInt32(fpu->x4.x[0]);
	fpu->x4.x[1] = OSSwapInt32(fpu->x4.x[1]);
	fpu->x4.x[2] = OSSwapInt32(fpu->x4.x[2]);
	fpu->x4.x[3] = OSSwapInt32(fpu->x4.x[3]);
	fpu->x5.x[0] = OSSwapInt32(fpu->x5.x[0]);
	fpu->x5.x[1] = OSSwapInt32(fpu->x5.x[1]);
	fpu->x5.x[2] = OSSwapInt32(fpu->x5.x[2]);
	fpu->x5.x[3] = OSSwapInt32(fpu->x5.x[3]);
	fpu->x6.x[0] = OSSwapInt32(fpu->x6.x[0]);
	fpu->x6.x[1] = OSSwapInt32(fpu->x6.x[1]);
	fpu->x6.x[2] = OSSwapInt32(fpu->x6.x[2]);
	fpu->x6.x[3] = OSSwapInt32(fpu->x6.x[3]);
	fpu->x7.x[0] = OSSwapInt32(fpu->x7.x[0]);
	fpu->x7.x[1] = OSSwapInt32(fpu->x7.x[1]);
	fpu->x7.x[2] = OSSwapInt32(fpu->x7.x[2]);
	fpu->x7.x[3] = OSSwapInt32(fpu->x7.x[3]);
	fpu->x8.x[0] = OSSwapInt32(fpu->x8.x[0]);
	fpu->x8.x[1] = OSSwapInt32(fpu->x8.x[1]);
	fpu->x8.x[2] = OSSwapInt32(fpu->x8.x[2]);
	fpu->x8.x[3] = OSSwapInt32(fpu->x8.x[3]);
	fpu->x9.x[0] = OSSwapInt32(fpu->x9.x[0]);
	fpu->x9.x[1] = OSSwapInt32(fpu->x9.x[1]);
	fpu->x9.x[2] = OSSwapInt32(fpu->x9.x[2]);
	fpu->x9.x[3] = OSSwapInt32(fpu->x9.x[3]);
	fpu->x10.x[0] = OSSwapInt32(fpu->x10.x[0]);
	fpu->x10.x[1] = OSSwapInt32(fpu->x10.x[1]);
	fpu->x10.x[2] = OSSwapInt32(fpu->x10.x[2]);
	fpu->x10.x[3] = OSSwapInt32(fpu->x10.x[3]);
	fpu->x11.x[0] = OSSwapInt32(fpu->x11.x[0]);
	fpu->x11.x[1] = OSSwapInt32(fpu->x11.x[1]);
	fpu->x11.x[2] = OSSwapInt32(fpu->x11.x[2]);
	fpu->x11.x[3] = OSSwapInt32(fpu->x11.x[3]);
	fpu->x12.x[0] = OSSwapInt32(fpu->x12.x[0]);
	fpu->x12.x[1] = OSSwapInt32(fpu->x12.x[1]);
	fpu->x12.x[2] = OSSwapInt32(fpu->x12.x[2]);
	fpu->x12.x[3] = OSSwapInt32(fpu->x12.x[3]);
	fpu->x13.x[0] = OSSwapInt32(fpu->x13.x[0]);
	fpu->x13.x[1] = OSSwapInt32(fpu->x13.x[1]);
	fpu->x13.x[2] = OSSwapInt32(fpu->x13.x[2]);
	fpu->x13.x[3] = OSSwapInt32(fpu->x13.x[3]);
	fpu->x14.x[0] = OSSwapInt32(fpu->x14.x[0]);
	fpu->x14.x[1] = OSSwapInt32(fpu->x14.x[1]);
	fpu->x14.x[2] = OSSwapInt32(fpu->x14.x[2]);
	fpu->x14.x[3] = OSSwapInt32(fpu->x14.x[3]);
	fpu->x15.x[0] = OSSwapInt32(fpu->x15.x[0]);
	fpu->x15.x[1] = OSSwapInt32(fpu->x15.x[1]);
	fpu->x15.x[2] = OSSwapInt32(fpu->x15.x[2]);
	fpu->x15.x[3] = OSSwapInt32(fpu->x15.x[3]);
	fpu->x16.x[0] = OSSwapInt32(fpu->x16.x[0]);
	fpu->x16.x[1] = OSSwapInt32(fpu->x16.x[1]);
	fpu->x16.x[2] = OSSwapInt32(fpu->x16.x[2]);
	fpu->x16.x[3] = OSSwapInt32(fpu->x16.x[3]);
	fpu->x17.x[0] = OSSwapInt32(fpu->x17.x[0]);
	fpu->x17.x[1] = OSSwapInt32(fpu->x17.x[1]);
	fpu->x17.x[2] = OSSwapInt32(fpu->x17.x[2]);
	fpu->x17.x[3] = OSSwapInt32(fpu->x17.x[3]);
	fpu->x18.x[0] = OSSwapInt32(fpu->x18.x[0]);
	fpu->x18.x[1] = OSSwapInt32(fpu->x18.x[1]);
	fpu->x18.x[2] = OSSwapInt32(fpu->x18.x[2]);
	fpu->x18.x[3] = OSSwapInt32(fpu->x18.x[3]);
	fpu->x19.x[0] = OSSwapInt32(fpu->x19.x[0]);
	fpu->x19.x[1] = OSSwapInt32(fpu->x19.x[1]);
	fpu->x19.x[2] = OSSwapInt32(fpu->x19.x[2]);
	fpu->x19.x[3] = OSSwapInt32(fpu->x19.x[3]);
	fpu->x20.x[0] = OSSwapInt32(fpu->x20.x[0]);
	fpu->x20.x[1] = OSSwapInt32(fpu->x20.x[1]);
	fpu->x20.x[2] = OSSwapInt32(fpu->x20.x[2]);
	fpu->x20.x[3] = OSSwapInt32(fpu->x20.x[3]);
	fpu->x21.x[0] = OSSwapInt32(fpu->x21.x[0]);
	fpu->x21.x[1] = OSSwapInt32(fpu->x21.x[1]);
	fpu->x21.x[2] = OSSwapInt32(fpu->x21.x[2]);
	fpu->x21.x[3] = OSSwapInt32(fpu->x21.x[3]);
	fpu->x22.x[0] = OSSwapInt32(fpu->x22.x[0]);
	fpu->x22.x[1] = OSSwapInt32(fpu->x22.x[1]);
	fpu->x22.x[2] = OSSwapInt32(fpu->x22.x[2]);
	fpu->x22.x[3] = OSSwapInt32(fpu->x22.x[3]);
	fpu->x23.x[0] = OSSwapInt32(fpu->x23.x[0]);
	fpu->x23.x[1] = OSSwapInt32(fpu->x23.x[1]);
	fpu->x23.x[2] = OSSwapInt32(fpu->x23.x[2]);
	fpu->x23.x[3] = OSSwapInt32(fpu->x23.x[3]);
	fpu->x24.x[0] = OSSwapInt32(fpu->x24.x[0]);
	fpu->x24.x[1] = OSSwapInt32(fpu->x24.x[1]);
	fpu->x24.x[2] = OSSwapInt32(fpu->x24.x[2]);
	fpu->x24.x[3] = OSSwapInt32(fpu->x24.x[3]);
	fpu->x25.x[0] = OSSwapInt32(fpu->x25.x[0]);
	fpu->x25.x[1] = OSSwapInt32(fpu->x25.x[1]);
	fpu->x25.x[2] = OSSwapInt32(fpu->x25.x[2]);
	fpu->x25.x[3] = OSSwapInt32(fpu->x25.x[3]);
	fpu->x26.x[0] = OSSwapInt32(fpu->x26.x[0]);
	fpu->x26.x[1] = OSSwapInt32(fpu->x26.x[1]);
	fpu->x26.x[2] = OSSwapInt32(fpu->x26.x[2]);
	fpu->x26.x[3] = OSSwapInt32(fpu->x26.x[3]);
	fpu->x27.x[0] = OSSwapInt32(fpu->x27.x[0]);
	fpu->x27.x[1] = OSSwapInt32(fpu->x27.x[1]);
	fpu->x27.x[2] = OSSwapInt32(fpu->x27.x[2]);
	fpu->x27.x[3] = OSSwapInt32(fpu->x27.x[3]);
	fpu->x28.x[0] = OSSwapInt32(fpu->x28.x[0]);
	fpu->x28.x[1] = OSSwapInt32(fpu->x28.x[1]);
	fpu->x28.x[2] = OSSwapInt32(fpu->x28.x[2]);
	fpu->x28.x[3] = OSSwapInt32(fpu->x28.x[3]);
	fpu->x29.x[0] = OSSwapInt32(fpu->x29.x[0]);
	fpu->x29.x[1] = OSSwapInt32(fpu->x29.x[1]);
	fpu->x29.x[2] = OSSwapInt32(fpu->x29.x[2]);
	fpu->x29.x[3] = OSSwapInt32(fpu->x29.x[3]);
	fpu->x30.x[0] = OSSwapInt32(fpu->x30.x[0]);
	fpu->x30.x[1] = OSSwapInt32(fpu->x30.x[1]);
	fpu->x30.x[2] = OSSwapInt32(fpu->x30.x[2]);
	fpu->x30.x[3] = OSSwapInt32(fpu->x30.x[3]);
	fpu->x31.x[0] = OSSwapInt32(fpu->x31.x[0]);
	fpu->x31.x[1] = OSSwapInt32(fpu->x31.x[1]);
	fpu->x31.x[2] = OSSwapInt32(fpu->x31.x[2]);
	fpu->x31.x[3] = OSSwapInt32(fpu->x31.x[3]);

	if(target_byte_sex == host_byte_sex){
	    memcpy(&ssr, &(fpu->fpsr), sizeof(struct swapped_m88k_fpsr));
	    ssr.u.word = OSSwapInt32(ssr.u.word);
	    fpu->fpsr.afinx = ssr.u.fields.afinx;
	    fpu->fpsr.afovf = ssr.u.fields.afovf;
	    fpu->fpsr.afunf = ssr.u.fields.afunf;
	    fpu->fpsr.afdvz = ssr.u.fields.afdvz;
	    fpu->fpsr.afinv = ssr.u.fields.afinv;
	    fpu->fpsr.xmod = ssr.u.fields.xmod;

	    memcpy(&scr, &(fpu->fpcr), sizeof(struct swapped_m88k_fpcr));
	    scr.u.word = OSSwapInt32(scr.u.word);
	    fpu->fpcr.efinx = scr.u.fields.efinx;
	    fpu->fpcr.efovf = scr.u.fields.efovf;
	    fpu->fpcr.efunf = scr.u.fields.efunf;
	    fpu->fpcr.efdvz = scr.u.fields.efdvz;
	    fpu->fpcr.efinv = scr.u.fields.efinv;
	    fpu->fpcr.rm = scr.u.fields.rm;
	}
	else{
	    ssr.u.fields.afinx = fpu->fpsr.afinx;
	    ssr.u.fields.afovf = fpu->fpsr.afovf;
	    ssr.u.fields.afunf = fpu->fpsr.afunf;
	    ssr.u.fields.afdvz = fpu->fpsr.afdvz;
	    ssr.u.fields.afinv = fpu->fpsr.afinv;
	    ssr.u.fields.xmod = fpu->fpsr.xmod;
	    ssr.u.word = OSSwapInt32(ssr.u.word);
	    memcpy(&(fpu->fpsr), &ssr, sizeof(struct swapped_m88k_fpsr));

	    scr.u.fields.efinx = fpu->fpcr.efinx;
	    scr.u.fields.efovf = fpu->fpcr.efovf;
	    scr.u.fields.efunf = fpu->fpcr.efunf;
	    scr.u.fields.efdvz = fpu->fpcr.efdvz;
	    scr.u.fields.efinv = fpu->fpcr.efinv;
	    scr.u.fields.rm = fpu->fpcr.rm;
	    scr.u.word = OSSwapInt32(scr.u.word);
	    memcpy(&(fpu->fpcr), &scr, sizeof(struct swapped_m88k_fpcr));
	}
}

void
swap_m88k_thread_state_user_t(
m88k_thread_state_user_t *user,
enum NXByteOrder target_byte_sex)
{
	user->user = OSSwapInt32(user->user);
}

void
swap_m88110_thread_state_impl_t(
m88110_thread_state_impl_t *spu,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;
    enum NXByteOrder host_byte_sex;

    struct swapped_m88110_bp_ctrl {
	union {
	    struct {
		unsigned	v:BIT_WIDTH(0);
		m88110_match_t	addr_match:BITS_WIDTH(12,1);
		unsigned	:BITS_WIDTH(26,13);
		unsigned	rwm:BIT_WIDTH(27);
		unsigned	rw:BIT_WIDTH(28);
		unsigned	:BITS_WIDTH(31,29);
	    } fields;
	    uint32_t word;
	} u;
    } sbpc;

    struct swap_m88110_psr {
	union {
	    struct {
		unsigned	:BITS_WIDTH(1,0);
		unsigned	mxm_dis:BIT_WIDTH(2);
		unsigned	sfu1dis:BIT_WIDTH(3);
		unsigned	:BITS_WIDTH(22,4);
		unsigned	trace:BIT_WIDTH(23);
		unsigned	:BIT_WIDTH(24);
		unsigned	sm:BIT_WIDTH(25);
		unsigned	sgn_imd:BIT_WIDTH(26);
		unsigned	:BIT_WIDTH(27);
		unsigned	c:BIT_WIDTH(28);
		unsigned	se:BIT_WIDTH(29);
		unsigned	le:BIT_WIDTH(30);
		unsigned	supr:BIT_WIDTH(31);
	    } fields;
	    uint32_t word;
	} u;
    } spsr;

    struct swapped_m88110_fp_trap_status {
	union {
	    struct {
		unsigned	efinx:BIT_WIDTH(0);
		unsigned	efovf:BIT_WIDTH(1);
		unsigned	efunf:BIT_WIDTH(2);
		unsigned	efdvz:BIT_WIDTH(3);
		unsigned	efinv:BIT_WIDTH(4);
		unsigned	priv:BIT_WIDTH(5);
		unsigned	unimp:BIT_WIDTH(6);
		unsigned	int:BIT_WIDTH(7);
		unsigned	sfu1_disabled:BIT_WIDTH(8);
		unsigned	:BITS_WIDTH(13,9);
		m88110_iresult_size_t	iresult_size:BITS_WIDTH(15,14);
		unsigned	:BITS_WIDTH(31,16);
	    } fields;
	    uint32_t word;
	} u;
    } sfps;

	host_byte_sex = NXHostByteOrder();

	if(target_byte_sex == host_byte_sex){
	    for(i = 0; i < M88110_N_DATA_BP; i++){
		spu->data_bp[i].addr = OSSwapInt32(spu->data_bp[i].addr);
		memcpy(&sbpc, &(spu->data_bp[i].ctrl),
		       sizeof(struct swapped_m88110_bp_ctrl));
		sbpc.u.word = OSSwapInt32(sbpc.u.word);
		spu->data_bp[i].ctrl.v = sbpc.u.fields.v;
		spu->data_bp[i].ctrl.addr_match = sbpc.u.fields.addr_match;
		spu->data_bp[i].ctrl.rwm = sbpc.u.fields.rwm;
		spu->data_bp[i].ctrl.rw = sbpc.u.fields.rw;
	    }

	    memcpy(&spsr, &(spu->psr), sizeof(struct swap_m88110_psr));
	    spsr.u.word = OSSwapInt32(spsr.u.word);
	    spu->psr.mxm_dis = spsr.u.fields.mxm_dis;
	    spu->psr.sfu1dis = spsr.u.fields.sfu1dis;
	    spu->psr.trace = spsr.u.fields.trace;
	    spu->psr.sm = spsr.u.fields.sm;
	    spu->psr.sgn_imd = spsr.u.fields.sgn_imd;
	    spu->psr.c = spsr.u.fields.c;
	    spu->psr.se = spsr.u.fields.se;
	    spu->psr.le = spsr.u.fields.le;
	    spu->psr.supr = spsr.u.fields.supr;

	    memcpy(&sfps, &(spu->fp_trap_status),
		   sizeof(struct swapped_m88110_fp_trap_status));
	    sfps.u.word = OSSwapInt32(sfps.u.word);
	    spu->fp_trap_status.efinx = sfps.u.fields.efinx;
	    spu->fp_trap_status.efovf = sfps.u.fields.efovf;
	    spu->fp_trap_status.efunf = sfps.u.fields.efunf;
	    spu->fp_trap_status.efdvz = sfps.u.fields.efdvz;
	    spu->fp_trap_status.efinv = sfps.u.fields.efinv;
	    spu->fp_trap_status.priv = sfps.u.fields.priv;
	    spu->fp_trap_status.unimp = sfps.u.fields.unimp;
	    spu->fp_trap_status.sfu1_disabled = sfps.u.fields.sfu1_disabled;
	    spu->fp_trap_status.iresult_size = sfps.u.fields.iresult_size;
	}
	else{
	    for(i = 0; i < M88110_N_DATA_BP; i++){
		spu->data_bp[i].addr = OSSwapInt32(spu->data_bp[i].addr);
		sbpc.u.fields.v = spu->data_bp[i].ctrl.v;
		sbpc.u.fields.addr_match = spu->data_bp[i].ctrl.addr_match;
		sbpc.u.fields.rwm = spu->data_bp[i].ctrl.rwm;
		sbpc.u.fields.rw = spu->data_bp[i].ctrl.rw;
		sbpc.u.word = OSSwapInt32(sbpc.u.word);
		memcpy(&(spu->data_bp[i].ctrl), &sbpc,
		       sizeof(struct swapped_m88110_bp_ctrl));
	    }

	    spsr.u.fields.mxm_dis = spu->psr.mxm_dis;
	    spsr.u.fields.sfu1dis = spu->psr.sfu1dis;
	    spsr.u.fields.trace = spu->psr.trace;
	    spsr.u.fields.sm = spu->psr.sm;
	    spsr.u.fields.sgn_imd = spu->psr.sgn_imd;
	    spsr.u.fields.c = spu->psr.c;
	    spsr.u.fields.se = spu->psr.se;
	    spsr.u.fields.le = spu->psr.le;
	    spsr.u.fields.supr = spu->psr.supr;
	    spsr.u.word = OSSwapInt32(spsr.u.word);
	    memcpy(&(spu->psr), &spsr, sizeof(struct swap_m88110_psr));

	    sfps.u.fields.efinx = spu->fp_trap_status.efinx;
	    sfps.u.fields.efovf = spu->fp_trap_status.efovf;
	    sfps.u.fields.efunf = spu->fp_trap_status.efunf;
	    sfps.u.fields.efdvz = spu->fp_trap_status.efdvz;
	    sfps.u.fields.efinv = spu->fp_trap_status.efinv;
	    sfps.u.fields.priv = spu->fp_trap_status.priv;
	    sfps.u.fields.unimp = spu->fp_trap_status.unimp;
	    sfps.u.fields.sfu1_disabled = spu->fp_trap_status.sfu1_disabled;
	    sfps.u.fields.iresult_size = spu->fp_trap_status.iresult_size;
	    sfps.u.word = OSSwapInt32(sfps.u.word);
	    memcpy(&(spu->fp_trap_status), &sfps,
		   sizeof(struct swapped_m88110_fp_trap_status));
	}
	spu->intermediate_result.x[0] =
	    OSSwapInt32(spu->intermediate_result.x[0]);
	spu->intermediate_result.x[1] =
	    OSSwapInt32(spu->intermediate_result.x[1]);
	spu->intermediate_result.x[2] =
	    OSSwapInt32(spu->intermediate_result.x[2]);
	spu->intermediate_result.x[3] =
	    OSSwapInt32(spu->intermediate_result.x[3]);
}
#endif /* !defined(RLD) */
