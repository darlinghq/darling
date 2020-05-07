/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#ifndef _I386_FPU_H_
#define _I386_FPU_H_

/*
 * Macro definitions for routines to manipulate the
 * floating-point processor.
 */
#include <kern/kern_types.h>
#include <mach/i386/kern_return.h>
#include <mach/i386/thread_status.h>
#include <i386/proc_reg.h>

#define FP_XMASK     ((uint32_t) (XFEM_X87 | XFEM_SSE))
#define AVX_XMASK    ((uint32_t) (XFEM_X87 | XFEM_SSE | XFEM_YMM))
#define AVX512_XMASK ((uint32_t) (XFEM_X87 | XFEM_SSE | XFEM_YMM | XFEM_ZMM))

typedef enum {
	FXSAVE32 = 1,
	FXSAVE64 = 2,
	XSAVE32  = 3,
	XSAVE64  = 4,
	FP_UNUSED = 5
} fp_save_layout_t;

#define STATE64_FULL 0x10
typedef enum {
	UNDEFINED,
	FP,
	AVX,
	AVX512,
	/*
	 * The following states are never associated with a thread or task.
	 * They are used for array declarations of data used during signal dispatch,
	 * but these values are never assigned to threads' (or tasks') xstate members.
	 */
	UNDEFINED_FULL = UNDEFINED | STATE64_FULL,
	FP_FULL = FP | STATE64_FULL,
	AVX_FULL = AVX | STATE64_FULL,
	AVX512_FULL = AVX512 | STATE64_FULL,
} xstate_t;

static inline uint64_t
xgetbv(uint32_t c)
{
	uint32_t        mask_hi, mask_lo;
	__asm__ __volatile__ ("xgetbv" : "=a"(mask_lo), "=d"(mask_hi) : "c" (c));
	return ((uint64_t) mask_hi << 32) + (uint64_t) mask_lo;
}

static inline void
xsetbv(uint32_t mask_hi, uint32_t mask_lo)
{
	__asm__ __volatile__ ("xsetbv" :: "a"(mask_lo), "d"(mask_hi), "c" (XCR0));
}

extern void             init_fpu(void);
extern void             fpu_module_init(void);
extern void             fpu_free(
	thread_t        thr_act,
	void            *fps);
extern kern_return_t    fpu_set_fxstate(
	thread_t        thr_act,
	thread_state_t  state,
	thread_flavor_t f);
extern kern_return_t    fpu_get_fxstate(
	thread_t        thr_act,
	thread_state_t  state,
	thread_flavor_t f);
extern void             fpu_dup_fxstate(
	thread_t        parent,
	thread_t        child);
extern void             fpnoextflt(void);
extern void             fpextovrflt(void);
extern void             fpexterrflt(void);
extern void             fpSSEexterrflt(void);
extern void             fpflush(thread_t);
extern void             fp_setvalid(boolean_t);

extern void             clear_fpu(void);
extern void             fpu_switch_context(
	thread_t        old,
	thread_t        new);
extern void             fpu_switch_addrmode(
	thread_t        thread,
	boolean_t       is_64bit);

extern xstate_t         fpu_default;
extern xstate_t         fpu_capability;
extern xstate_t         current_xstate(void);
extern int              fpUDflt(user_addr_t rip);
#ifdef MACH_KERNEL_PRIVATE
extern uint32_t thread_fpsimd_hash(thread_t);
extern void vzeroall(void);
extern void xmmzeroall(void);
extern void avx512_zero(void);
#endif /* MKP */
#endif  /* _I386_FPU_H_ */
