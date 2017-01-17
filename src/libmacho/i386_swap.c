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
#ifdef SHLIB
#undef environ
#endif

#define __busy busy
#define __c0 c0
#define __c1 c1
#define __c2 c2
#define __c3 c3
#define __cs cs
#define __darwin_fp_control fp_control
#define __darwin_fp_status fp_status
#define __darwin_i386_exception_state i386_exception_state
#define __darwin_i386_float_state i386_float_state
#define __darwin_i386_thread_state i386_thread_state
#define __darwin_mmst_reg mmst_reg
#define __darwin_xmm_reg xmm_reg
#define __denorm denorm
#define __ds ds
#define __eax eax
#define __ebp ebp
#define __ebx ebx
#define __ecx ecx
#define __edi edi
#define __edx edx
#define __eflags eflags
#define __eip eip
#define __err err
#define __errsumm errsumm
#define __es es
#define __esi esi
#define __esp esp
#define __faultvaddr faultvaddr
#define __fpu_cs fpu_cs
#define __fpu_dp fpu_dp
#define __fpu_ds fpu_ds
#define __fpu_fcw fpu_fcw
#define __fpu_fop fpu_fop
#define __fpu_fsw fpu_fsw
#define __fpu_ftw fpu_ftw
#define __fpu_ip fpu_ip
#define __fpu_mxcsr fpu_mxcsr
#define __fpu_mxcsrmask fpu_mxcsrmask
#define __fpu_reserved fpu_reserved
#define __fpu_reserved1 fpu_reserved1
#define __fpu_rsrv1 fpu_rsrv1
#define __fpu_rsrv2 fpu_rsrv2
#define __fpu_rsrv3 fpu_rsrv3
#define __fpu_rsrv4 fpu_rsrv4
#define __fpu_stmm0 fpu_stmm0
#define __fpu_stmm1 fpu_stmm1
#define __fpu_stmm2 fpu_stmm2
#define __fpu_stmm3 fpu_stmm3
#define __fpu_stmm4 fpu_stmm4
#define __fpu_stmm5 fpu_stmm5
#define __fpu_stmm6 fpu_stmm6
#define __fpu_stmm7 fpu_stmm7
#define __fpu_xmm0 fpu_xmm0
#define __fpu_xmm1 fpu_xmm1
#define __fpu_xmm2 fpu_xmm2
#define __fpu_xmm3 fpu_xmm3
#define __fpu_xmm4 fpu_xmm4
#define __fpu_xmm5 fpu_xmm5
#define __fpu_xmm6 fpu_xmm6
#define __fpu_xmm7 fpu_xmm7
#define __fs fs
#define __gs gs
#define __invalid invalid
#define __mmst_reg mmst_reg
#define __mmst_rsrv mmst_rsrv
#define __ovrfl ovrfl
#define __pc pc
#define __precis precis
#define __rc rc
#define __ss ss
#define __stkflt stkflt
#define __tos tos
#define __trapno trapno
#define __undfl undfl
#define __xmm_reg xmm_reg
#define __zdiv zdiv

#define __rax rax
#define __rbx rbx
#define __rcx rcx
#define __rdx rdx
#define __rdi rdi
#define __rsi rsi
#define __rbp rbp
#define __rsp rsp
#define __r8 r8
#define __r9 r9
#define __r10 r10
#define __r11 r11
#define __r12 r12
#define __r13 r13
#define __r14 r14
#define __r15 r15
#define __rip rip
#define __rflags rflags

#define __dr0 dr0
#define __dr1 dr1
#define __dr2 dr2
#define __dr3 dr3
#define __dr4 dr4
#define __dr5 dr5
#define __dr6 dr6
#define __dr7 dr7

#include <string.h>
#include <libkern/OSByteOrder.h>
#import <mach-o/i386/swap.h>

void
swap_i386_thread_state(
i386_thread_state_t *cpu,
enum NXByteOrder target_byte_sex)
{
	cpu->eax = OSSwapInt32(cpu->eax);
	cpu->ebx = OSSwapInt32(cpu->ebx);
	cpu->ecx = OSSwapInt32(cpu->ecx);
	cpu->edx = OSSwapInt32(cpu->edx);
	cpu->edi = OSSwapInt32(cpu->edi);
	cpu->esi = OSSwapInt32(cpu->esi);
	cpu->ebp = OSSwapInt32(cpu->ebp);
	cpu->esp = OSSwapInt32(cpu->esp);
	cpu->ss = OSSwapInt32(cpu->ss);
	cpu->eflags = OSSwapInt32(cpu->eflags);
	cpu->eip = OSSwapInt32(cpu->eip);
	cpu->cs = OSSwapInt32(cpu->cs);
	cpu->ds = OSSwapInt32(cpu->ds);
	cpu->es = OSSwapInt32(cpu->es);
	cpu->fs = OSSwapInt32(cpu->fs);
	cpu->gs = OSSwapInt32(cpu->gs);
}

#ifdef x86_THREAD_STATE64
void
swap_x86_thread_state64(
x86_thread_state64_t *cpu,
enum NXByteOrder target_byte_sex)
{
	cpu->rax = OSSwapInt64(cpu->rax);
	cpu->rbx = OSSwapInt64(cpu->rbx);
	cpu->rcx = OSSwapInt64(cpu->rcx);
	cpu->rdx = OSSwapInt64(cpu->rdx);
	cpu->rdi = OSSwapInt64(cpu->rdi);
	cpu->rsi = OSSwapInt64(cpu->rsi);
	cpu->rbp = OSSwapInt64(cpu->rbp);
	cpu->rsp = OSSwapInt64(cpu->rsp);
	cpu->rflags = OSSwapInt64(cpu->rflags);
	cpu->rip = OSSwapInt64(cpu->rip);
	cpu->r8 = OSSwapInt64(cpu->r8);
	cpu->r9 = OSSwapInt64(cpu->r9);
	cpu->r10 = OSSwapInt64(cpu->r10);
	cpu->r11 = OSSwapInt64(cpu->r11);
	cpu->r12 = OSSwapInt64(cpu->r12);
	cpu->r13 = OSSwapInt64(cpu->r13);
	cpu->r14 = OSSwapInt64(cpu->r14);
	cpu->r15 = OSSwapInt64(cpu->r15);
	cpu->cs = OSSwapInt64(cpu->cs);
	cpu->fs = OSSwapInt64(cpu->fs);
	cpu->gs = OSSwapInt64(cpu->gs);
}

void
swap_x86_state_hdr(
x86_state_hdr_t *hdr,
enum NXByteOrder target_byte_sex)
{
	hdr->flavor = OSSwapInt32(hdr->flavor);
	hdr->count = OSSwapInt32(hdr->count);
}

void
swap_x86_float_state64(
x86_float_state64_t *fpu,
enum NXByteOrder target_byte_sex)
{
    struct swapped_fp_control {
	union {
	    struct {
		unsigned short
			    :3,
		    /*inf*/ :1,
		    rc	    :2,
		    pc	    :2,
			    :2,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfpc;

    struct swapped_fp_status {
	union {
	    struct {
		unsigned short
		    busy    :1,
		    c3	    :1,
		    tos	    :3,
		    c2	    :1,
		    c1	    :1,
		    c0	    :1,
		    errsumm :1,
		    stkflt  :1,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfps;

    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();

	fpu->fpu_reserved[0] = OSSwapInt32(fpu->fpu_reserved[0]);
	fpu->fpu_reserved[1] = OSSwapInt32(fpu->fpu_reserved[1]);

	if(target_byte_sex == host_byte_sex){
	    memcpy(&sfpc, &(fpu->fpu_fcw),
		   sizeof(struct swapped_fp_control));
	    sfpc.u.half = OSSwapInt16(sfpc.u.half);
	    fpu->fpu_fcw.rc = sfpc.u.fields.rc;
	    fpu->fpu_fcw.pc = sfpc.u.fields.pc;
	    fpu->fpu_fcw.precis = sfpc.u.fields.precis;
	    fpu->fpu_fcw.undfl = sfpc.u.fields.undfl;
	    fpu->fpu_fcw.ovrfl = sfpc.u.fields.ovrfl;
	    fpu->fpu_fcw.zdiv = sfpc.u.fields.zdiv;
	    fpu->fpu_fcw.denorm = sfpc.u.fields.denorm;
	    fpu->fpu_fcw.invalid = sfpc.u.fields.invalid;

	    memcpy(&sfps, &(fpu->fpu_fsw),
		   sizeof(struct swapped_fp_status));
	    sfps.u.half = OSSwapInt16(sfps.u.half);
	    fpu->fpu_fsw.busy = sfps.u.fields.busy;
	    fpu->fpu_fsw.c3 = sfps.u.fields.c3;
	    fpu->fpu_fsw.tos = sfps.u.fields.tos;
	    fpu->fpu_fsw.c2 = sfps.u.fields.c2;
	    fpu->fpu_fsw.c1 = sfps.u.fields.c1;
	    fpu->fpu_fsw.c0 = sfps.u.fields.c0;
	    fpu->fpu_fsw.errsumm = sfps.u.fields.errsumm;
	    fpu->fpu_fsw.stkflt = sfps.u.fields.stkflt;
	    fpu->fpu_fsw.precis = sfps.u.fields.precis;
	    fpu->fpu_fsw.undfl = sfps.u.fields.undfl;
	    fpu->fpu_fsw.ovrfl = sfps.u.fields.ovrfl;
	    fpu->fpu_fsw.zdiv = sfps.u.fields.zdiv;
	    fpu->fpu_fsw.denorm = sfps.u.fields.denorm;
	    fpu->fpu_fsw.invalid = sfps.u.fields.invalid;
	}
	else{
	    sfpc.u.fields.rc = fpu->fpu_fcw.rc;
	    sfpc.u.fields.pc = fpu->fpu_fcw.pc;
	    sfpc.u.fields.precis = fpu->fpu_fcw.precis;
	    sfpc.u.fields.undfl = fpu->fpu_fcw.undfl;
	    sfpc.u.fields.ovrfl = fpu->fpu_fcw.ovrfl;
	    sfpc.u.fields.zdiv = fpu->fpu_fcw.zdiv;
	    sfpc.u.fields.denorm = fpu->fpu_fcw.denorm;
	    sfpc.u.fields.invalid = fpu->fpu_fcw.invalid;
	    sfpc.u.half = OSSwapInt16(sfpc.u.half);
	    memcpy(&(fpu->fpu_fcw), &sfpc,
		   sizeof(struct swapped_fp_control));

	    sfps.u.fields.busy = fpu->fpu_fsw.busy;
	    sfps.u.fields.c3 = fpu->fpu_fsw.c3;
	    sfps.u.fields.tos = fpu->fpu_fsw.tos;
	    sfps.u.fields.c2 = fpu->fpu_fsw.c2;
	    sfps.u.fields.c1 = fpu->fpu_fsw.c1;
	    sfps.u.fields.c0 = fpu->fpu_fsw.c0;
	    sfps.u.fields.errsumm = fpu->fpu_fsw.errsumm;
	    sfps.u.fields.stkflt = fpu->fpu_fsw.stkflt;
	    sfps.u.fields.precis = fpu->fpu_fsw.precis;
	    sfps.u.fields.undfl = fpu->fpu_fsw.undfl;
	    sfps.u.fields.ovrfl = fpu->fpu_fsw.ovrfl;
	    sfps.u.fields.zdiv = fpu->fpu_fsw.zdiv;
	    sfps.u.fields.denorm = fpu->fpu_fsw.denorm;
	    sfps.u.fields.invalid = fpu->fpu_fsw.invalid;
	    sfps.u.half = OSSwapInt16(sfps.u.half);
	    memcpy(&(fpu->fpu_fsw), &sfps,
		   sizeof(struct swapped_fp_status));
	}
	fpu->fpu_fop = OSSwapInt16(fpu->fpu_fop);
	fpu->fpu_ip = OSSwapInt32(fpu->fpu_ip);
	fpu->fpu_cs = OSSwapInt16(fpu->fpu_cs);
	fpu->fpu_rsrv2 = OSSwapInt16(fpu->fpu_rsrv2);
	fpu->fpu_dp = OSSwapInt32(fpu->fpu_dp);
	fpu->fpu_ds = OSSwapInt16(fpu->fpu_ds);
	fpu->fpu_rsrv3 = OSSwapInt16(fpu->fpu_rsrv3);
	fpu->fpu_mxcsr = OSSwapInt32(fpu->fpu_mxcsr);
	fpu->fpu_mxcsrmask = OSSwapInt32(fpu->fpu_mxcsrmask);
	fpu->fpu_reserved1 = OSSwapInt32(fpu->fpu_reserved1);
}

void
swap_x86_exception_state64(
x86_exception_state64_t *exc,
enum NXByteOrder target_byte_sex)
{
	exc->trapno = OSSwapInt32(exc->trapno);
	exc->err = OSSwapInt32(exc->err);
    	exc->faultvaddr = OSSwapInt64(exc->faultvaddr);
}

void
swap_x86_thread_state(
x86_thread_state_t *cpu,
enum NXByteOrder target_byte_sex)
{
    x86_state_hdr_t hdr;
    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();
	hdr = cpu->tsh;
	if(target_byte_sex == host_byte_sex)
	    swap_x86_state_hdr(&hdr, host_byte_sex);

	swap_x86_state_hdr(&cpu->tsh, target_byte_sex);

	if(hdr.flavor == x86_THREAD_STATE32)
	    swap_i386_thread_state(&cpu->uts.ts32, target_byte_sex);
	else if(hdr.flavor == x86_THREAD_STATE64)
	    swap_x86_thread_state64(&cpu->uts.ts64, target_byte_sex);
}

void
swap_x86_float_state(
x86_float_state_t *fpu,
enum NXByteOrder target_byte_sex)
{
    x86_state_hdr_t hdr;
    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();
	hdr = fpu->fsh;
	if(target_byte_sex == host_byte_sex)
	    swap_x86_state_hdr(&hdr, host_byte_sex);

	swap_x86_state_hdr(&fpu->fsh, target_byte_sex);

/* current i386 thread states */
#if i386_THREAD_STATE == 1
	if(hdr.flavor == x86_FLOAT_STATE32)
	    swap_i386_float_state(&fpu->ufs.fs32, target_byte_sex);
	else
#endif
	if(hdr.flavor == x86_FLOAT_STATE64)
	    swap_x86_float_state64(&fpu->ufs.fs64, target_byte_sex);
}

void
swap_x86_exception_state(
x86_exception_state_t *exc,
enum NXByteOrder target_byte_sex)
{
    x86_state_hdr_t hdr;
    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();
	hdr = exc->esh;
	if(target_byte_sex == host_byte_sex)
	    swap_x86_state_hdr(&hdr, host_byte_sex);

	swap_x86_state_hdr(&exc->esh, target_byte_sex);

/* current i386 thread states */
#if i386_THREAD_STATE == 1
	if(hdr.flavor == x86_EXCEPTION_STATE32)
	    swap_i386_exception_state(&exc->ues.es32, target_byte_sex);
	else
#endif
	if(hdr.flavor == x86_EXCEPTION_STATE64)
	    swap_x86_exception_state64(&exc->ues.es64, target_byte_sex);
}

void
swap_x86_debug_state32(
x86_debug_state32_t *debug,
enum NXByteOrder target_byte_sex)
{
	debug->dr0 = OSSwapInt32(debug->dr0);
	debug->dr1 = OSSwapInt32(debug->dr1);
	debug->dr2 = OSSwapInt32(debug->dr2);
	debug->dr3 = OSSwapInt32(debug->dr3);
	debug->dr4 = OSSwapInt32(debug->dr4);
	debug->dr5 = OSSwapInt32(debug->dr5);
	debug->dr6 = OSSwapInt32(debug->dr6);
	debug->dr7 = OSSwapInt32(debug->dr7);
}

void
swap_x86_debug_state64(
x86_debug_state64_t *debug,
enum NXByteOrder target_byte_sex)
{
	debug->dr0 = OSSwapInt64(debug->dr0);
	debug->dr1 = OSSwapInt64(debug->dr1);
	debug->dr2 = OSSwapInt64(debug->dr2);
	debug->dr3 = OSSwapInt64(debug->dr3);
	debug->dr4 = OSSwapInt64(debug->dr4);
	debug->dr5 = OSSwapInt64(debug->dr5);
	debug->dr6 = OSSwapInt64(debug->dr6);
	debug->dr7 = OSSwapInt64(debug->dr7);
}

void
swap_x86_debug_state(
x86_debug_state_t *debug,
enum NXByteOrder target_byte_sex)
{
    x86_state_hdr_t hdr;
    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();
	hdr = debug->dsh;
	if(target_byte_sex == host_byte_sex)
	    swap_x86_state_hdr(&hdr, host_byte_sex);

	swap_x86_state_hdr(&debug->dsh, target_byte_sex);

	if(hdr.flavor == x86_DEBUG_STATE32)
	    swap_x86_debug_state32(&debug->uds.ds32, target_byte_sex);
	else if(hdr.flavor == x86_DEBUG_STATE64)
	    swap_x86_debug_state64(&debug->uds.ds64, target_byte_sex);
}
#endif /* x86_THREAD_STATE64 */

/* current i386 thread states */
#if i386_THREAD_STATE == 1
void
swap_i386_float_state(
struct i386_float_state *fpu,
enum NXByteOrder target_byte_sex)
{
#ifndef i386_EXCEPTION_STATE_COUNT
    /* this routine does nothing as their are currently no non-byte fields */
#else /* !defined(i386_EXCEPTION_STATE_COUNT) */
    struct swapped_fp_control {
	union {
	    struct {
		unsigned short
			    :3,
		    /*inf*/ :1,
		    rc	    :2,
		    pc	    :2,
			    :2,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfpc;

    struct swapped_fp_status {
	union {
	    struct {
		unsigned short
		    busy    :1,
		    c3	    :1,
		    tos	    :3,
		    c2	    :1,
		    c1	    :1,
		    c0	    :1,
		    errsumm :1,
		    stkflt  :1,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfps;

    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();

	fpu->fpu_reserved[0] = OSSwapInt32(fpu->fpu_reserved[0]);
	fpu->fpu_reserved[1] = OSSwapInt32(fpu->fpu_reserved[1]);

	if(target_byte_sex == host_byte_sex){
	    memcpy(&sfpc, &(fpu->fpu_fcw),
		   sizeof(struct swapped_fp_control));
	    sfpc.u.half = OSSwapInt16(sfpc.u.half);
	    fpu->fpu_fcw.rc = sfpc.u.fields.rc;
	    fpu->fpu_fcw.pc = sfpc.u.fields.pc;
	    fpu->fpu_fcw.precis = sfpc.u.fields.precis;
	    fpu->fpu_fcw.undfl = sfpc.u.fields.undfl;
	    fpu->fpu_fcw.ovrfl = sfpc.u.fields.ovrfl;
	    fpu->fpu_fcw.zdiv = sfpc.u.fields.zdiv;
	    fpu->fpu_fcw.denorm = sfpc.u.fields.denorm;
	    fpu->fpu_fcw.invalid = sfpc.u.fields.invalid;

	    memcpy(&sfps, &(fpu->fpu_fsw),
		   sizeof(struct swapped_fp_status));
	    sfps.u.half = OSSwapInt16(sfps.u.half);
	    fpu->fpu_fsw.busy = sfps.u.fields.busy;
	    fpu->fpu_fsw.c3 = sfps.u.fields.c3;
	    fpu->fpu_fsw.tos = sfps.u.fields.tos;
	    fpu->fpu_fsw.c2 = sfps.u.fields.c2;
	    fpu->fpu_fsw.c1 = sfps.u.fields.c1;
	    fpu->fpu_fsw.c0 = sfps.u.fields.c0;
	    fpu->fpu_fsw.errsumm = sfps.u.fields.errsumm;
	    fpu->fpu_fsw.stkflt = sfps.u.fields.stkflt;
	    fpu->fpu_fsw.precis = sfps.u.fields.precis;
	    fpu->fpu_fsw.undfl = sfps.u.fields.undfl;
	    fpu->fpu_fsw.ovrfl = sfps.u.fields.ovrfl;
	    fpu->fpu_fsw.zdiv = sfps.u.fields.zdiv;
	    fpu->fpu_fsw.denorm = sfps.u.fields.denorm;
	    fpu->fpu_fsw.invalid = sfps.u.fields.invalid;
	}
	else{
	    sfpc.u.fields.rc = fpu->fpu_fcw.rc;
	    sfpc.u.fields.pc = fpu->fpu_fcw.pc;
	    sfpc.u.fields.precis = fpu->fpu_fcw.precis;
	    sfpc.u.fields.undfl = fpu->fpu_fcw.undfl;
	    sfpc.u.fields.ovrfl = fpu->fpu_fcw.ovrfl;
	    sfpc.u.fields.zdiv = fpu->fpu_fcw.zdiv;
	    sfpc.u.fields.denorm = fpu->fpu_fcw.denorm;
	    sfpc.u.fields.invalid = fpu->fpu_fcw.invalid;
	    sfpc.u.half = OSSwapInt16(sfpc.u.half);
	    memcpy(&(fpu->fpu_fcw), &sfpc,
		   sizeof(struct swapped_fp_control));

	    sfps.u.fields.busy = fpu->fpu_fsw.busy;
	    sfps.u.fields.c3 = fpu->fpu_fsw.c3;
	    sfps.u.fields.tos = fpu->fpu_fsw.tos;
	    sfps.u.fields.c2 = fpu->fpu_fsw.c2;
	    sfps.u.fields.c1 = fpu->fpu_fsw.c1;
	    sfps.u.fields.c0 = fpu->fpu_fsw.c0;
	    sfps.u.fields.errsumm = fpu->fpu_fsw.errsumm;
	    sfps.u.fields.stkflt = fpu->fpu_fsw.stkflt;
	    sfps.u.fields.precis = fpu->fpu_fsw.precis;
	    sfps.u.fields.undfl = fpu->fpu_fsw.undfl;
	    sfps.u.fields.ovrfl = fpu->fpu_fsw.ovrfl;
	    sfps.u.fields.zdiv = fpu->fpu_fsw.zdiv;
	    sfps.u.fields.denorm = fpu->fpu_fsw.denorm;
	    sfps.u.fields.invalid = fpu->fpu_fsw.invalid;
	    sfps.u.half = OSSwapInt16(sfps.u.half);
	    memcpy(&(fpu->fpu_fsw), &sfps,
		   sizeof(struct swapped_fp_status));
	}
	fpu->fpu_fop = OSSwapInt16(fpu->fpu_fop);
	fpu->fpu_ip = OSSwapInt32(fpu->fpu_ip);
	fpu->fpu_cs = OSSwapInt16(fpu->fpu_cs);
	fpu->fpu_rsrv2 = OSSwapInt16(fpu->fpu_rsrv2);
	fpu->fpu_dp = OSSwapInt32(fpu->fpu_dp);
	fpu->fpu_ds = OSSwapInt16(fpu->fpu_ds);
	fpu->fpu_rsrv3 = OSSwapInt16(fpu->fpu_rsrv3);
	fpu->fpu_mxcsr = OSSwapInt32(fpu->fpu_mxcsr);
	fpu->fpu_mxcsrmask = OSSwapInt32(fpu->fpu_mxcsrmask);
	fpu->fpu_reserved1 = OSSwapInt32(fpu->fpu_reserved1);

#endif /* !defined(i386_EXCEPTION_STATE_COUNT) */
}

void
swap_i386_exception_state(
i386_exception_state_t *exc,
enum NXByteOrder target_byte_sex)
{
	exc->trapno = OSSwapInt32(exc->trapno);
	exc->err = OSSwapInt32(exc->err);
    	exc->faultvaddr = OSSwapInt32(exc->faultvaddr);
}
#endif /* i386_THREAD_STATE == 1 */

/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
void
swap_i386_thread_fpstate(
i386_thread_fpstate_t *fpu,
enum NXByteOrder target_byte_sex)
{
    struct swapped_fp_control {
	union {
	    struct {
		unsigned short
			    :3,
		    /*inf*/ :1,
		    rc	    :2,
		    pc	    :2,
			    :2,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfpc;

    struct swapped_fp_status {
	union {
	    struct {
		unsigned short
		    busy    :1,
		    c3	    :1,
		    tos	    :3,
		    c2	    :1,
		    c1	    :1,
		    c0	    :1,
		    errsumm :1,
		    stkflt  :1,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfps;

    struct swapped_fp_tag {
	union {
	    struct {
		unsigned short
		    tag7 :2,
		    tag6 :2,
		    tag5 :2,
		    tag4 :2,
		    tag3 :2,
		    tag2 :2,
		    tag1 :2,
		    tag0 :2;
	    } fields;
	    unsigned short half;
	} u;
    } sfpt;

    struct swapped_fp_data_reg {
	unsigned short mant;
	unsigned short mant1 :16,
		       mant2 :16,
		       mant3 :16;
	union {
	    struct {
		unsigned short sign :1,
			       exp  :15;
	    } fields;
	    unsigned short half;
	} u;
    } sfpd;

    struct swapped_sel {
	union {
	    struct {
    		unsigned short
		    index :13,
		    ti    :1,
		    rpl   :2;
	    } fields;
	    unsigned short half;
	} u;
    } ss;

    enum NXByteOrder host_byte_sex;
    int i;

	host_byte_sex = NXHostByteOrder();

	fpu->environ.ip = OSSwapInt32(fpu->environ.ip);
	fpu->environ.opcode = OSSwapInt16(fpu->environ.opcode);
	fpu->environ.dp = OSSwapInt32(fpu->environ.dp);

	if(target_byte_sex == host_byte_sex){
	    memcpy(&sfpc, &(fpu->environ.control),
		   sizeof(struct swapped_fp_control));
	    sfpc.u.half = OSSwapInt16(sfpc.u.half);
	    fpu->environ.control.rc = sfpc.u.fields.rc;
	    fpu->environ.control.pc = sfpc.u.fields.pc;
	    fpu->environ.control.precis = sfpc.u.fields.precis;
	    fpu->environ.control.undfl = sfpc.u.fields.undfl;
	    fpu->environ.control.ovrfl = sfpc.u.fields.ovrfl;
	    fpu->environ.control.zdiv = sfpc.u.fields.zdiv;
	    fpu->environ.control.denorm = sfpc.u.fields.denorm;
	    fpu->environ.control.invalid = sfpc.u.fields.invalid;

	    memcpy(&sfps, &(fpu->environ.status),
		   sizeof(struct swapped_fp_status));
	    sfps.u.half = OSSwapInt16(sfps.u.half);
	    fpu->environ.status.busy = sfps.u.fields.busy;
	    fpu->environ.status.c3 = sfps.u.fields.c3;
	    fpu->environ.status.tos = sfps.u.fields.tos;
	    fpu->environ.status.c2 = sfps.u.fields.c2;
	    fpu->environ.status.c1 = sfps.u.fields.c1;
	    fpu->environ.status.c0 = sfps.u.fields.c0;
	    fpu->environ.status.errsumm = sfps.u.fields.errsumm;
	    fpu->environ.status.stkflt = sfps.u.fields.stkflt;
	    fpu->environ.status.precis = sfps.u.fields.precis;
	    fpu->environ.status.undfl = sfps.u.fields.undfl;
	    fpu->environ.status.ovrfl = sfps.u.fields.ovrfl;
	    fpu->environ.status.zdiv = sfps.u.fields.zdiv;
	    fpu->environ.status.denorm = sfps.u.fields.denorm;
	    fpu->environ.status.invalid = sfps.u.fields.invalid;

	    memcpy(&sfpt, &(fpu->environ.tag),
		   sizeof(struct swapped_fp_tag));
	    sfpt.u.half = OSSwapInt16(sfpt.u.half);
	    fpu->environ.tag.tag7 = sfpt.u.fields.tag7;
	    fpu->environ.tag.tag6 = sfpt.u.fields.tag6;
	    fpu->environ.tag.tag5 = sfpt.u.fields.tag5;
	    fpu->environ.tag.tag4 = sfpt.u.fields.tag4;
	    fpu->environ.tag.tag3 = sfpt.u.fields.tag3;
	    fpu->environ.tag.tag2 = sfpt.u.fields.tag2;
	    fpu->environ.tag.tag1 = sfpt.u.fields.tag1;
	    fpu->environ.tag.tag0 = sfpt.u.fields.tag0;

	    memcpy(&ss, &(fpu->environ.cs),
		   sizeof(struct swapped_sel));
	    ss.u.half = OSSwapInt16(ss.u.half);
	    fpu->environ.cs.index = ss.u.fields.index;
	    fpu->environ.cs.ti = ss.u.fields.ti;
	    fpu->environ.cs.rpl = ss.u.fields.rpl;

	    memcpy(&ss, &(fpu->environ.ds),
		   sizeof(struct swapped_sel));
	    ss.u.half = OSSwapInt16(ss.u.half);
	    fpu->environ.ds.index = ss.u.fields.index;
	    fpu->environ.ds.ti = ss.u.fields.ti;
	    fpu->environ.ds.rpl = ss.u.fields.rpl;
	
	    for(i = 0; i < 8; i++){
		memcpy(&sfpd, &(fpu->stack.ST[i]),
		       sizeof(struct swapped_fp_data_reg));
		fpu->stack.ST[i].mant = OSSwapInt16(sfpd.mant);
		fpu->stack.ST[i].mant1 = OSSwapInt16(sfpd.mant1);
		fpu->stack.ST[i].mant2 = OSSwapInt16(sfpd.mant2);
		fpu->stack.ST[i].mant3 = OSSwapInt16(sfpd.mant3);
		sfpd.u.half = OSSwapInt16(sfpd.u.half);
		fpu->stack.ST[i].exp = sfpd.u.fields.exp;
		fpu->stack.ST[i].sign = sfpd.u.fields.sign;
	    }
	}
	else{
	    sfpc.u.fields.rc = fpu->environ.control.rc;
	    sfpc.u.fields.pc = fpu->environ.control.pc;
	    sfpc.u.fields.precis = fpu->environ.control.precis;
	    sfpc.u.fields.undfl = fpu->environ.control.undfl;
	    sfpc.u.fields.ovrfl = fpu->environ.control.ovrfl;
	    sfpc.u.fields.zdiv = fpu->environ.control.zdiv;
	    sfpc.u.fields.denorm = fpu->environ.control.denorm;
	    sfpc.u.fields.invalid = fpu->environ.control.invalid;
	    sfpc.u.half = OSSwapInt16(sfpc.u.half);
	    memcpy(&(fpu->environ.control), &sfpc,
		   sizeof(struct swapped_fp_control));

	    sfps.u.fields.busy = fpu->environ.status.busy;
	    sfps.u.fields.c3 = fpu->environ.status.c3;
	    sfps.u.fields.tos = fpu->environ.status.tos;
	    sfps.u.fields.c2 = fpu->environ.status.c2;
	    sfps.u.fields.c1 = fpu->environ.status.c1;
	    sfps.u.fields.c0 = fpu->environ.status.c0;
	    sfps.u.fields.errsumm = fpu->environ.status.errsumm;
	    sfps.u.fields.stkflt = fpu->environ.status.stkflt;
	    sfps.u.fields.precis = fpu->environ.status.precis;
	    sfps.u.fields.undfl = fpu->environ.status.undfl;
	    sfps.u.fields.ovrfl = fpu->environ.status.ovrfl;
	    sfps.u.fields.zdiv = fpu->environ.status.zdiv;
	    sfps.u.fields.denorm = fpu->environ.status.denorm;
	    sfps.u.fields.invalid = fpu->environ.status.invalid;
	    sfps.u.half = OSSwapInt16(sfps.u.half);
	    memcpy(&(fpu->environ.status), &sfps,
		   sizeof(struct swapped_fp_status));

	    sfpt.u.fields.tag7 = fpu->environ.tag.tag7;
	    sfpt.u.fields.tag6 = fpu->environ.tag.tag6;
	    sfpt.u.fields.tag5 = fpu->environ.tag.tag5;
	    sfpt.u.fields.tag4 = fpu->environ.tag.tag4;
	    sfpt.u.fields.tag3 = fpu->environ.tag.tag3;
	    sfpt.u.fields.tag2 = fpu->environ.tag.tag2;
	    sfpt.u.fields.tag1 = fpu->environ.tag.tag1;
	    sfpt.u.fields.tag0 = fpu->environ.tag.tag0;
	    sfpt.u.half = OSSwapInt16(sfpt.u.half);
	    memcpy(&(fpu->environ.tag), &sfpt,
		   sizeof(struct swapped_fp_tag));

	    ss.u.fields.index = fpu->environ.cs.index;
	    ss.u.fields.ti = fpu->environ.cs.ti;
	    ss.u.fields.rpl = fpu->environ.cs.rpl;
	    ss.u.half = OSSwapInt16(ss.u.half);
	    memcpy(&(fpu->environ.cs), &ss,
		   sizeof(struct swapped_sel));

	    ss.u.fields.index = fpu->environ.ds.index;
	    ss.u.fields.ti = fpu->environ.ds.ti;
	    ss.u.fields.rpl = fpu->environ.ds.rpl;
	    ss.u.half = OSSwapInt16(ss.u.half);
	    memcpy(&(fpu->environ.cs), &ss,
		   sizeof(struct swapped_sel));

	    for(i = 0; i < 8; i++){
		sfpd.mant = OSSwapInt16(fpu->stack.ST[i].mant);
		sfpd.mant1 = OSSwapInt16(fpu->stack.ST[i].mant1);
		sfpd.mant2 = OSSwapInt16(fpu->stack.ST[i].mant2);
		sfpd.mant3 = OSSwapInt16(fpu->stack.ST[i].mant3);
		sfpd.u.fields.exp = fpu->stack.ST[i].exp;
		sfpd.u.fields.sign = fpu->stack.ST[i].sign;
		sfpd.u.half = OSSwapInt16(sfpd.u.half);
		memcpy(&(fpu->stack.ST[i]), &sfpd,
		       sizeof(struct swapped_fp_data_reg));
	    }
	}
}

void
swap_i386_thread_exceptstate(
i386_thread_exceptstate_t *exc,
enum NXByteOrder target_byte_sex)
{
    struct swapped_err_code {
	union {
	    struct err_code_normal {
		unsigned int		:16,
				index	:13,
				tbl	:2,
				ext	:1;
	    } normal;
	    struct err_code_pgfault {
		unsigned int		:29,
				user	:1,
				wrtflt	:1,
				prot	:1;
	    } pgfault;
	    uint32_t word;
	} u;
    } sec;
    uint32_t word;
    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();

	exc->trapno = OSSwapInt32(exc->trapno);
	if(exc->trapno == 14){
	    if(target_byte_sex == host_byte_sex){
		memcpy(&sec, &(exc->err), sizeof(struct swapped_err_code));
		sec.u.word = OSSwapInt32(sec.u.word);
		exc->err.pgfault.user   = sec.u.pgfault.user;
		exc->err.pgfault.wrtflt = sec.u.pgfault.wrtflt;
		exc->err.pgfault.prot   = sec.u.pgfault.prot;
	    }
	    else{
		sec.u.pgfault.prot   = exc->err.pgfault.prot;
		sec.u.pgfault.wrtflt = exc->err.pgfault.wrtflt;
		sec.u.pgfault.user   = exc->err.pgfault.user;
		sec.u.word = OSSwapInt32(sec.u.word);
		memcpy(&(exc->err), &sec, sizeof(struct swapped_err_code));
	    }
	}
	else{
	    if(target_byte_sex == host_byte_sex){
		memcpy(&sec, &(exc->err), sizeof(struct swapped_err_code));
		sec.u.word = OSSwapInt32(sec.u.word);
		word = sec.u.normal.index;
		exc->err.normal.index = OSSwapInt32(word);
		exc->err.normal.tbl   = sec.u.normal.tbl;
		exc->err.normal.ext   = sec.u.normal.ext;
	    }
	    else{
		sec.u.normal.ext   = exc->err.normal.ext;
		sec.u.normal.tbl   = exc->err.normal.tbl;
		word = exc->err.normal.index;
		sec.u.normal.index = OSSwapInt32(word);
		sec.u.word = OSSwapInt32(sec.u.word);
		memcpy(&(exc->err), &sec, sizeof(struct swapped_err_code));
	    }
	}
}

void
swap_i386_thread_cthreadstate(
i386_thread_cthreadstate_t *user,
enum NXByteOrder target_byte_sex)
{
	user->self = OSSwapInt32(user->self);
}
#endif /* i386_THREAD_STATE == -1 */
#endif /* !defined(RLD) */
