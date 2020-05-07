/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
 */
/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _FASTTRAP_ISA_H
#define _FASTTRAP_ISA_H

#include <sys/types.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef union {
	uint32_t instr32;
	struct {
		uint16_t instr1;
		uint16_t instr2;
	} instr16;
} fasttrap_instr_t;

typedef struct fasttrap_machtp {
	fasttrap_instr_t        ftmt_instr;     /* original instruction */

	uint8_t                 ftmt_fntype;    /* One of the FASTTRAP_FN* constants defined below */
	/* Once the tracepoint is initialized, fntype will be FN_DONE_INIT and thumb will be 0 for ARM, 1 for Thumb */
	uint8_t                 ftmt_thumb;

	uint8_t                 ftmt_type;
	uint8_t                 ftmt_installed:1;
	uint8_t                 ftmt_retired:1;
} fasttrap_machtp_t;

#define ftt_instr       ftt_mtp.ftmt_instr.instr32
#define ftt_instr1      ftt_mtp.ftmt_instr.instr16.instr1
#define ftt_instr2      ftt_mtp.ftmt_instr.instr16.instr2
#define ftt_fntype      ftt_mtp.ftmt_fntype
#define ftt_thumb       ftt_mtp.ftmt_thumb
#define ftt_type        ftt_mtp.ftmt_type
#define ftt_installed   ftt_mtp.ftmt_installed
#define ftt_retired     ftt_mtp.ftmt_retired

#define FASTTRAP_T_INV                          1
#define FASTTRAP_T_COMMON                       2
#define FASTTRAP_T_BLX                          3
#define FASTTRAP_T_B_COND                       4
#define FASTTRAP_T_B_UNCOND                     5
#define FASTTRAP_T_BX_REG                       6
#define FASTTRAP_T_PUSH_LR                      7
#define FASTTRAP_T_POP_PC                       8
#define FASTTRAP_T_STM_LR                       9
#define FASTTRAP_T_LDM_PC                       10
#define FASTTRAP_T_CPY_PC                       11
#define FASTTRAP_T_MOV_PC_REG                   12
#define FASTTRAP_T_LDR_PC_IMMED                 13
#define FASTTRAP_T_VLDR_PC_IMMED                14
#define FASTTRAP_T_CB_N_Z                       15
#if defined(__arm64__)
#define FASTTRAP_T_ARM64_STANDARD_FUNCTION_ENTRY 16     /* stp fp, lr, [sp, #-16]! */
#define FASTTRAP_T_ARM64_LDR_S_PC_REL           17
#define FASTTRAP_T_ARM64_LDR_W_PC_REL           18
#define FASTTRAP_T_ARM64_LDR_D_PC_REL           19
#define FASTTRAP_T_ARM64_LDR_X_PC_REL           20
#define FASTTRAP_T_ARM64_LDR_Q_PC_REL           21
#define FASTTRAP_T_ARM64_LDRSW_PC_REL           22
#define FASTTRAP_T_ARM64_B_COND                 23
#define FASTTRAP_T_ARM64_CBNZ_W                 24
#define FASTTRAP_T_ARM64_CBNZ_X                 25
#define FASTTRAP_T_ARM64_CBZ_W                  26
#define FASTTRAP_T_ARM64_CBZ_X                  27
#define FASTTRAP_T_ARM64_TBNZ                   28
#define FASTTRAP_T_ARM64_TBZ                    29
#define FASTTRAP_T_ARM64_B                      30
#define FASTTRAP_T_ARM64_BL                     31
#define FASTTRAP_T_ARM64_BLR                    32
#define FASTTRAP_T_ARM64_BR                     33
#define FASTTRAP_T_ARM64_RET                    34
#define FASTTRAP_T_ARM64_ADRP                   35
#define FASTTRAP_T_ARM64_ADR                    36
#define FASTTRAP_T_ARM64_PRFM                   37
#define FASTTRAP_T_ARM64_EXCLUSIVE_MEM          38
#define FASTTRAP_T_ARM64_RETAB                  39
#endif

#if defined (__arm__)
#define FASTTRAP_ARM_INSTR       0xe7ffdefc
#define FASTTRAP_THUMB_INSTR     0xdefc
#define FASTTRAP_ARM_RET_INSTR   0xe7ffdefb
#define FASTTRAP_THUMB_RET_INSTR 0xdefb

#elif defined (__arm64__)
#define FASTTRAP_ARM32_INSTR       0xe7ffdefc
#define FASTTRAP_THUMB32_INSTR     0xdefc
#define FASTTRAP_ARM64_INSTR       0xe7eeee7e

#define FASTTRAP_ARM32_RET_INSTR   0xe7ffdefb
#define FASTTRAP_THUMB32_RET_INSTR 0xdefb
#define FASTTRAP_ARM64_RET_INSTR   0xe7eeee7d
#endif

#define FASTTRAP_FN_DONE_INIT 255
#define FASTTRAP_FN_UNKNOWN 0
#define FASTTRAP_FN_ARM 1
#define FASTTRAP_FN_THUMB 2
#define FASTTRAP_FN_USDT 3
#define FASTTRAP_FN_ARM64 4
#define FASTTRAP_FN_ARM64_32 5

#define ARM_RM(x) ((x) & 0xF)
#define ARM_RS(x) (((x) >> 8) & 0xF)
#define ARM_RD(x) (((x) >> 12) & 0xF)
#define ARM_RN(x) (((x) >> 16) & 0xF)
#define ARM_CONDCODE(x) ((x) >> 28)

#define THUMB16_HRM(x) (((x) >> 3) & 0xF)
#define THUMB16_HRD(x) (((x) & 0x7) | ((((x) >> 4) & 0x8)))

#define THUMB32_RM(x, y) ((y) & 0xF)
#define THUMB32_RD(x, y) (((y) >> 8) & 0xF)
#define THUMB32_RT(x, y) (((y) >> 12) & 0xF)
#define THUMB32_RN(x, y) ((x) & 0xF)

#define REG_SP 13
#define REG_LR 14
#define REG_PC 15

#define FASTTRAP_RETURN_AFRAMES         6
#define FASTTRAP_ENTRY_AFRAMES          5
#define FASTTRAP_OFFSET_AFRAMES         5

#if defined(__arm64__)
#define FASTTRAP_ARM64_OP_VALUE_FUNC_ENTRY      0xa9bf7bfd /* stp fp, lr, [sp, #-16]! */

#define FASTTRAP_ARM64_OP_MASK_LDR_S_PC_REL     0xff000000 /* Bits to check for ldr St, label */
#define FASTTRAP_ARM64_OP_VALUE_LDR_S_PC_REL    0x1c000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_LDR_W_PC_REL     0xff000000 /* Bits to check for ldr Wt, label */
#define FASTTRAP_ARM64_OP_VALUE_LDR_W_PC_REL    0x18000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_LDR_D_PC_REL     0xff000000 /* Bits to check for ldr Dt, label */
#define FASTTRAP_ARM64_OP_VALUE_LDR_D_PC_REL    0x5c000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_LDR_X_PC_REL     0xff000000 /* Bits to check for ldr Xt, label */
#define FASTTRAP_ARM64_OP_VALUE_LDR_X_PC_REL    0x58000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_LDR_Q_PC_REL     0xff000000 /* Bits to check for ldr Qt, label */
#define FASTTRAP_ARM64_OP_VALUE_LDR_Q_PC_REL    0x9c000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_LRDSW_PC_REL     0xff000000 /* Bits to check for ldrsw <reg>, label */
#define FASTTRAP_ARM64_OP_VALUE_LRDSW_PC_REL    0x98000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_B_COND_PC_REL    0xff000010 /* Bits to check for b.cond label */
#define FASTTRAP_ARM64_OP_VALUE_B_COND_PC_REL   0x54000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_CBNZ_W_PC_REL    0xff000000 /* Bits to check for cbnz Wt, _label */
#define FASTTRAP_ARM64_OP_VALUE_CBNZ_W_PC_REL   0x35000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_CBNZ_X_PC_REL    0xff000000 /* Bits to check for cbnz Xt, _label */
#define FASTTRAP_ARM64_OP_VALUE_CBNZ_X_PC_REL   0xb5000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_CBZ_W_PC_REL     0xff000000 /* Bits to check for cbz Wt, _label */
#define FASTTRAP_ARM64_OP_VALUE_CBZ_W_PC_REL    0x34000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_CBZ_X_PC_REL     0xff000000 /* Bits to check for cbz Xt, _label */
#define FASTTRAP_ARM64_OP_VALUE_CBZ_X_PC_REL    0xb4000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_TBNZ_PC_REL      0x7f000000 /* Bits to check for tbnz Xt, _label */
#define FASTTRAP_ARM64_OP_VALUE_TBNZ_PC_REL     0x37000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_TBZ_PC_REL       0x7f000000 /* Bits to check for tbz Xt, _label */
#define FASTTRAP_ARM64_OP_VALUE_TBZ_PC_REL      0x36000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_B_PC_REL         0xfc000000 /* Bits to check for b _label */
#define FASTTRAP_ARM64_OP_VALUE_B_PC_REL        0x14000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_BL_PC_REL        0xfc000000 /* Bits to check for bl _label */
#define FASTTRAP_ARM64_OP_VALUE_BL_PC_REL       0x94000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_BLR              0xfffffe1f /* Bits to check for blr Xt */
#define FASTTRAP_ARM64_OP_VALUE_BLR             0xd63f0000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_BR               0xfffffe1f /* Bits to check for br Xt */
#define FASTTRAP_ARM64_OP_VALUE_BR              0xd61f0000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_RET              0xfffffc1f /* Bits to check for ret Rt */
#define FASTTRAP_ARM64_OP_VALUE_RET             0xd65f0000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_ADRP             0x9f000000 /* Bits to check for adrp Xt, label*/
#define FASTTRAP_ARM64_OP_VALUE_ADRP            0x90000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_ADR              0x9f000000 /* Bits to check for adr Xt, label*/
#define FASTTRAP_ARM64_OP_VALUE_ADR             0x10000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_PRFM             0xff000000 /* Bits to check for adr Xt, label*/
#define FASTTRAP_ARM64_OP_VALUE_PRFM            0xd8000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_EXCL_MEM         0x3f000000 /* Bits  to check for exclusive memory operation */
#define FASTTRAP_ARM64_OP_VALUE_EXCL_MEM        0x08000000 /* Value to find */

#define FASTTRAP_ARM64_OP_MASK_RETAB            0xfffffc1f /* Bits to check for retab Rt */
#define FASTTRAP_ARM64_OP_VALUE_RETAB           0xd65f0c1f /* Value to find */
#endif /* defined(__arm64__) */

#ifdef  __cplusplus
}
#endif

#endif  /* _FASTTRAP_ISA_H */
