/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
/*
 */

#ifndef _ARM_TRAP_H_
#define _ARM_TRAP_H_

/*
 * Hardware trap vectors for ARM.
 */

#define T_RESET                 0
#define T_UNDEF                 1
#define T_SWI                   2
#define T_PREFETCH_ABT          3
#define T_DATA_ABT              4
#define T_IRQ                   6
#define T_FIQ                   7
#define T_PMU                   8


#define TRAP_NAMES "reset", "undefined instruction", "software interrupt", \
	           "prefetch abort", "data abort", "irq interrupt", \
	           "fast interrupt", "perfmon"

/*
 * Page-fault trap codes.
 */
#define T_PF_PROT               0x1             /* protection violation */
#define T_PF_WRITE              0x2             /* write access */
#define T_PF_USER               0x4             /* from user state */

#if !defined(ASSEMBLER) && defined(MACH_KERNEL)

#include <arm/thread.h>

#define GDB_TRAP_INSTR1 0xe7ffdefe
#define GDB_TRAP_INSTR2 0xe7ffdeff

#define ARM_GDB_INSTR1  GDB_TRAP_INSTR1
#define ARM_GDB_INSTR2  GDB_TRAP_INSTR2

#define IS_ARM_GDB_TRAP(op)     \
	(((op) == ARM_GDB_INSTR1) || ((op) == ARM_GDB_INSTR2))

#define THUMB_GDB_INSTR1        (GDB_TRAP_INSTR1 & 0xFFFF)
#define THUMB_GDB_INSTR2        (GDB_TRAP_INSTR2 & 0xFFFF)

#define IS_THUMB_GDB_TRAP(op)   \
	(((op) == THUMB_GDB_INSTR1) || ((op) == THUMB_GDB_INSTR2))


#define ARM_STR                 0x04000000      /* STR */
#define ARM_STRH                0x000000B0      /* STRH */
#define ARM_STRH_MASK           0x0E1000F0      /* STRH MASK */
#define ARM_SDX_MASK            0x0C100000      /* SINGLE DATA TRANSFER */
#define ARM_SNGL_DX_MASK        0x0C000000      /* SINGLE DATA TRANSFER MASK */
#define ARM_SDX                 0x04000000

#define ARM_STM                 0x08000000      /* STM */
#define ARM_BDX_MASK            0x0E100000      /* BLOCK DATA TRANSFER */
#define ARM_BLK_MASK            0x0E000000      /* BLOCK DATA TRANSFER */
#define ARM_BDX                 0x08000000      /* BLOCK DATA TRANSFER */

#define ARM_WRITE_BACK          0x00200000
#define ARM_BASE_REG            0x000F0000
#define ARM_INCREMENT           0x00800000

#define ARM_STC                 0x0C000000      /* STC */
#define ARM_CDX_MASK            ARM_BDX_MASK    /* COPROCESSOR DATA TRANSFER */
#define ARM_CBLK_MASK           ARM_BLK_MASK
#define ARM_CDX                 0x0C000000      /* COPROCESSOR DATA TRANSFER */

#define ARM_SWP                 0x01000090      /* SWP */
#define ARM_SWP_MASK            0x0FB00FF0      /* SWP */

#define ARM_POST_INDEXING       0x01000000
#define ARM_IMMEDIATE           0x02000000
#define ARM_LSL                 0
#define ARM_LSR                 1
#define ARM_ASR                 2
#define ARM_ROR                 3

#define MCR_MASK        0x0F100F10
#define MCR_CP15        0x0E000F10
#define MCRR_MASK       0x0FF00F00
#define MCRR_CP15       0x0C400F00

#define arm_mcr_cp15(op)        (((op)&MCR_MASK) == 0x0E000F10)
#define arm_mcrr_cp15(op)       (((op)&0x0FF00F00) == 0x0C400F00)

#define IS_THUMB32(op)  (       \
	(((op) & 0xE000) == 0xE000) && (((op) & 0x1800) != 0x0000))

#define THUMB_LDR_1_MASK        0x8800          /* (1) forms of LD* instructions */
#define THUMB_STR_1_MASK        0xF800          /* (1) forms of ST* instructions */
#define THUMB_STR_2_MASK        0xFE00          /* (2) forms of ST* instructions */
#define THUMB_STR_3_MASK        0xF800          /* (3) forms of ST* instructions */
#define THUMB_PUSH_MASK         0xFE00          /* PUSH instruction */

#define THUMB_LDRH_1            0x8800          /* LDRH(1) */
#define THUMB_STMIA             0xC000          /* STMIA */
#define THUMB_STR_1             0x6000          /* STR(1) */
#define THUMB_STR_2             0x5000          /* STR(2) */
#define THUMB_STR_3             0x9000          /* STR(3) */
#define THUMB_STRB_1            0x7000          /* STRB(1) */
#define THUMB_STRB_2            0x5400          /* STRB(2) */
#define THUMB_STRH_1            0x8000          /* STRH(1) */
#define THUMB_STRH_2            0x5200          /* STRH(2) */
#define THUMB_PUSH              0xB400          /* PUSH */
#define THUMB_LDMIA             0xC800          /* LDMIA */
#define THUMB_POP               0xBC00          /* POP */


/*
 * Shifts, masks, and other values for load/store multiple decoding; largely needed for
 * supporting misaligned accesses.
 */
#define THUMB_STR_1_BASE_OFFSET 8               /* Offset of the base register field */
#define THUMB_PUSH_EXTRA_OFFSET 8               /* Offset of the "extra" register field */
#define ARM_STM_BASE_OFFSET     16              /* Offset of the base register field */
#define ARM_STM_LOAD_OFFSET     20              /* Offset of the load flag */
#define ARM_STM_WBACK_OFFSET    21              /* Offset of the writeback flag */
#define ARM_STM_INCR_OFFSET     23              /* Offset of the increment flag */
#define ARM_STM_BEFORE_OFFSET   24              /* Offset of the pre-index flag */
#define ARM_REG_LIST_LR_OFFSET  14              /* Offset of LR in the register list */
#define ARM_REG_LIST_PC_OFFSET  15              /* Offset of PC in the register list */

#define THUMB_STR_REG_LIST_MASK 0x000000FF      /* Offset of the reg list is 0 */
#define THUMB_STR_1_BASE_MASK   0x00000700
#define THUMB_PUSH_EXTRA_MASK   0x00000100
#define ARM_STM_REG_LIST_MASK   0x0000FFFF      /* Offset of the reg list is 0 */
#define ARM_STM_BASE_MASK       0x000F0000
#define ARM_STM_LOAD_MASK       0x00100000
#define ARM_STM_WBACK_MASK      0x00200000
#define ARM_STM_INCR_MASK       0x00800000
#define ARM_STM_BEFORE_MASK     0x01000000
#define ARM_COND_MASK           0xF0000000      /* Mask for the condition code */

#define ARM_COND_UNCOND         0xF0000000      /* Instruction does not support condition codes */

#define ARM_SIMD_MASK0          0xFE000000
#define ARM_SIMD_CODE0          0xF2000000

#define ARM_VFP_MASK0           0x0F000E10
#define ARM_VFP_CODE0           0x0E000A00

#define ARM_SIMD_VFP_MASK0      0x0E000E00
#define ARM_SIMD_VFP_CODE0      0x0C000A00
#define ARM_SIMD_VFP_MASK1      0xFF100000
#define ARM_SIMD_VFP_CODE1      0xF4000000
#define ARM_SIMD_VFP_MASK2      0x0F000E10
#define ARM_SIMD_VFP_CODE2      0x0E000A10
#define ARM_SIMD_VFP_MASK3      0x0FE00E00
#define ARM_SIMD_VFP_CODE3      0x0C400A00

#define IS_ARM_VFP(op)  (       \
	(((op) & ARM_SIMD_MASK0) == ARM_SIMD_CODE0)     \
	 ||(((op) & ARM_VFP_MASK0) == ARM_VFP_CODE0)    \
	 ||(((op) & ARM_SIMD_VFP_MASK0) == ARM_SIMD_VFP_CODE0)  \
	 ||(((op) & ARM_SIMD_VFP_MASK1) == ARM_SIMD_VFP_CODE1)  \
	 ||(((op) & ARM_SIMD_VFP_MASK2) == ARM_SIMD_VFP_CODE2)  \
	 || (((op) & ARM_SIMD_VFP_MASK3) == ARM_SIMD_VFP_CODE3))

#define THUMB_SIMD_MASK0        0xEF000000
#define THUMB_SIMD_CODE0        0xEF000000

#define THUMB_VFP_MASK0         0xEF000E10
#define THUMB_VFP_CODE0         0xEE000A00

#define THUMB_SIMD_VFP_MASK0    0xEE000E00
#define THUMB_SIMD_VFP_CODE0    0xEC000A00
#define THUMB_SIMD_VFP_MASK1    0xFF100000
#define THUMB_SIMD_VFP_CODE1    0xF9000000
#define THUMB_SIMD_VFP_MASK2    0xEF000E10
#define THUMB_SIMD_VFP_CODE2    0xEE000A10
#define THUMB_SIMD_VFP_MASK3    0xEFE00E00
#define THUMB_SIMD_VFP_CODE3    0xEC400A00

#define IS_THUMB_VFP(op)        (       \
	(((op) & THUMB_SIMD_MASK0) == THUMB_SIMD_CODE0 )        \
	 || (((op) & THUMB_VFP_MASK0) == THUMB_VFP_CODE0 )      \
	 || (((op) & THUMB_SIMD_VFP_MASK0) == THUMB_SIMD_VFP_CODE0 )    \
	 || (((op) & THUMB_SIMD_VFP_MASK1) == THUMB_SIMD_VFP_CODE1 )    \
	 || (((op) & THUMB_SIMD_VFP_MASK2) == THUMB_SIMD_VFP_CODE2 )    \
	 || (((op) & THUMB_SIMD_VFP_MASK3) == THUMB_SIMD_VFP_CODE3))

extern boolean_t arm_force_fast_fault(ppnum_t, vm_prot_t, int, void *);
extern kern_return_t arm_fast_fault(pmap_t, vm_map_address_t, vm_prot_t, bool, bool);

/*
 * Determines if the aborted instruction is read or write operation
 */
#define arm_fault_type(op, spsr, vaddr) \
       (((((op)&ARM_CDX_MASK) == ARM_STC) || \
	 (((op)&ARM_STRH_MASK) == ARM_STRH) || \
	 (((op)&ARM_BDX_MASK) == ARM_STM) || \
	 (((op)&ARM_SDX_MASK) == ARM_STR)) ?  \
	                (VM_PROT_WRITE|VM_PROT_READ) : (VM_PROT_READ))

#define thumb_fault_type(op, spsr, vaddr) \
	(((((op)&THUMB_STR_1_MASK) == THUMB_STMIA) || \
	  (((op)&THUMB_STR_1_MASK) == THUMB_STR_1) || \
	  (((op)&THUMB_STR_2_MASK) == THUMB_STR_2) || \
	  (((op)&THUMB_STR_3_MASK) == THUMB_STR_3) || \
	  (((op)&THUMB_STR_1_MASK) == THUMB_STRB_1) || \
	  (((op)&THUMB_STR_2_MASK) == THUMB_STRB_2) || \
	  (((op)&THUMB_STR_1_MASK) == THUMB_STRH_1) || \
	  (((op)&THUMB_STR_2_MASK) == THUMB_STRH_2) || \
	  (((op)&THUMB_PUSH_MASK) == THUMB_PUSH)) ? \
	        (VM_PROT_WRITE|VM_PROT_READ) : (VM_PROT_READ))

typedef kern_return_t (*perfCallback)(
	int                     trapno,
	struct arm_saved_state  *ss,
	uintptr_t *,
	int);

#endif  /* !ASSEMBLER && MACH_KERNEL */

#endif  /* _ARM_TRAP_H_ */
