/*
 * Copyright (c) 2007-2013 Apple Inc. All rights reserved.
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
 * Processor registers for ARM64
 */
#ifndef _ARM64_PROC_REG_H_
#define _ARM64_PROC_REG_H_

#include <arm/proc_reg.h>

#if __ARM_KERNEL_PROTECT__
/*
 * __ARM_KERNEL_PROTECT__ is a feature intended to guard against potential
 * architectural or microarchitectural vulnerabilities that could allow cores to
 * read/access EL1-only mappings while in EL0 mode.  This is achieved by
 * removing as many mappings as possible when the core transitions to EL0 mode
 * from EL1 mode, and restoring those mappings when the core transitions to EL1
 * mode from EL0 mode.
 *
 * At the moment, this is achieved through use of ASIDs and TCR_EL1.  TCR_EL1 is
 * used to map and unmap the ordinary kernel mappings, by contracting and
 * expanding translation zone size for TTBR1 when exiting and entering EL1,
 * respectively:
 *
 * Kernel EL0 Mappings: TTBR1 mappings that must remain mapped while the core is
 *   is in EL0.
 * Kernel EL1 Mappings: TTBR1 mappings that must be mapped while the core is in
 *   EL1.
 *
 * T1SZ_USER: T1SZ_BOOT + 1
 * TTBR1_EL1_BASE_BOOT: (2^64) - (2^(64 - T1SZ_BOOT)
 * TTBR1_EL1_BASE_USER: (2^64) - (2^(64 - T1SZ_USER)
 * TTBR1_EL1_MAX: (2^64) - 1
 *
 * When in EL1, we program TCR_EL1 (specifically, TCR_EL1.T1SZ) to give the
 * the following TTBR1 layout:
 *
 *  TTBR1_EL1_BASE_BOOT   TTBR1_EL1_BASE_USER   TTBR1_EL1_MAX
 * +---------------------------------------------------------+
 * | Kernel EL0 Mappings |        Kernel EL1 Mappings        |
 * +---------------------------------------------------------+
 *
 * And when in EL0, we program TCR_EL1 to give the following TTBR1 layout:
 *
 *  TTBR1_EL1_BASE_USER                         TTBR1_EL1_MAX
 * +---------------------------------------------------------+
 * |                   Kernel EL0 Mappings                   |
 * +---------------------------------------------------------+
 *
 * With the current implementation, both the EL0 and EL1 mappings for the kernel
 * use otherwise empty translation tables for mapping the exception vectors (so
 * that we do not need to TLB flush the exception vector address when switching
 * between EL0 and EL1).  The rationale here is that the TLBI would require a
 * DSB, and DSBs can be extremely expensive.
 *
 * Each pmap is given two ASIDs: (n & ~1) as an EL0 ASID, and (n | 1) as an EL1
 * ASID.  The core switches between ASIDs on EL transitions, so that the TLB
 * does not need to be fully invalidated on an EL transition.
 *
 * Most kernel mappings will be marked non-global in this configuration, as
 * global mappings would be visible to userspace unless we invalidate them on
 * eret.
 */
#endif /* __ARM_KERNEL_PROTECT */

/*
 * 64-bit Program Status Register (PSR64)
 *
 *  31      27 23  22 21 20 19      10 9       5 4   0
 * +-+-+-+-+-----+---+--+--+----------+-+-+-+-+-+-----+
 * |N|Z|C|V|00000|PAN|SS|IL|0000000000|D|A|I|F|0|  M  |
 * +-+-+-+-+-+---+---+--+--+----------+-+-+-+-+-+-----+
 *
 * where:
 *   NZCV: Comparison flags
 *   PAN:  Privileged Access Never
 *   SS:   Single step
 *   IL:   Illegal state
 *   DAIF: Interrupt masks
 *   M:    Mode field
 */

#define PSR64_NZCV_SHIFT 28
#define PSR64_NZCV_MASK  (1 << PSR64_NZCV_SHIFT)

#define PSR64_N_SHIFT    31
#define PSR64_N          (1 << PSR64_N_SHIFT)

#define PSR64_Z_SHIFT    30
#define PSR64_Z          (1 << PSR64_Z_SHIFT)

#define PSR64_C_SHIFT    29
#define PSR64_C          (1 << PSR64_C_SHIFT)

#define PSR64_V_SHIFT    28
#define PSR64_V          (1 << PSR64_V_SHIFT)

#define PSR64_PAN_SHIFT  22
#define PSR64_PAN        (1 << PSR64_PAN_SHIFT)

#define PSR64_SS_SHIFT   21
#define PSR64_SS         (1 << PSR64_SS_SHIFT)

#define PSR64_IL_SHIFT   20
#define PSR64_IL         (1 << PSR64_IL_SHIFT)

/*
 * msr DAIF, Xn and mrs Xn, DAIF transfer into
 * and out of bits 9:6
 */
#define DAIF_DEBUG_SHIFT      9
#define DAIF_DEBUGF           (1 << DAIF_DEBUG_SHIFT)

#define DAIF_ASYNC_SHIFT      8
#define DAIF_ASYNCF           (1 << DAIF_ASYNC_SHIFT)

#define DAIF_IRQF_SHIFT       7
#define DAIF_IRQF             (1 << DAIF_IRQF_SHIFT)

#define DAIF_FIQF_SHIFT       6
#define DAIF_FIQF             (1 << DAIF_FIQF_SHIFT)

#define DAIF_ALL              (DAIF_DEBUGF | DAIF_ASYNCF | DAIF_IRQF | DAIF_FIQF)
#define DAIF_STANDARD_DISABLE (DAIF_ASYNCF | DAIF_IRQF | DAIF_FIQF)

#define SPSR_INTERRUPTS_ENABLED(x) (!(x & DAIF_FIQF))

/*
 * msr DAIFSet, Xn, and msr DAIFClr, Xn transfer
 * from bits 3:0.
 */
#define DAIFSC_DEBUGF           (1 << 3)
#define DAIFSC_ASYNCF           (1 << 2)
#define DAIFSC_IRQF             (1 << 1)
#define DAIFSC_FIQF             (1 << 0)
#define DAIFSC_ALL              (DAIFSC_DEBUGF | DAIFSC_ASYNCF | DAIFSC_IRQF | DAIFSC_FIQF)
#define DAIFSC_STANDARD_DISABLE (DAIFSC_ASYNCF | DAIFSC_IRQF | DAIFSC_FIQF)

/*
 * ARM64_TODO: unify with ARM?
 */
#define PSR64_CF         0x20000000 /* Carry/Borrow/Extend */

#define PSR64_MODE_MASK         0x1F

#define PSR64_MODE_USER32_THUMB 0x20

#define PSR64_MODE_RW_SHIFT     4
#define PSR64_MODE_RW_64        0
#define PSR64_MODE_RW_32        (0x1 << PSR64_MODE_RW_SHIFT)

#define PSR64_MODE_EL_SHIFT     2
#define PSR64_MODE_EL_MASK      (0x3 << PSR64_MODE_EL_SHIFT)
#define PSR64_MODE_EL3          (0x3 << PSR64_MODE_EL_SHIFT)
#define PSR64_MODE_EL2          (0x2 << PSR64_MODE_EL_SHIFT)
#define PSR64_MODE_EL1          (0x1 << PSR64_MODE_EL_SHIFT)
#define PSR64_MODE_EL0          0

#define PSR64_MODE_SPX          0x1
#define PSR64_MODE_SP0          0

#define PSR64_USER32_DEFAULT    (PSR64_MODE_RW_32 | PSR64_MODE_EL0 | PSR64_MODE_SP0)
#define PSR64_USER64_DEFAULT    (PSR64_MODE_RW_64 | PSR64_MODE_EL0 | PSR64_MODE_SP0)
#define PSR64_KERNEL_STANDARD   (DAIF_STANDARD_DISABLE | PSR64_MODE_RW_64 | PSR64_MODE_EL1 | PSR64_MODE_SP0)
#if __ARM_PAN_AVAILABLE__
#define PSR64_KERNEL_DEFAULT    (PSR64_KERNEL_STANDARD | PSR64_PAN)
#else
#define PSR64_KERNEL_DEFAULT    PSR64_KERNEL_STANDARD
#endif

#define PSR64_IS_KERNEL(x)      ((x & PSR64_MODE_EL_MASK) > PSR64_MODE_EL0)
#define PSR64_IS_USER(x)        ((x & PSR64_MODE_EL_MASK) == PSR64_MODE_EL0)

#define PSR64_IS_USER32(x)      (PSR64_IS_USER(x) && (x & PSR64_MODE_RW_32))
#define PSR64_IS_USER64(x)      (PSR64_IS_USER(x) && !(x & PSR64_MODE_RW_32))



/*
 * System Control Register (SCTLR)
 */

#define SCTLR_RESERVED     ((3ULL << 28) | (1ULL << 22) | (1ULL << 20) | (1ULL << 11))
#if defined(HAS_APPLE_PAC)

// 31    PACIA_ENABLED AddPACIA and AuthIA functions enabled
#define SCTLR_PACIA_ENABLED_SHIFT 31
#define SCTLR_PACIA_ENABLED       (1ULL << SCTLR_PACIA_ENABLED_SHIFT)
// 30    PACIB_ENABLED AddPACIB and AuthIB functions enabled
#define SCTLR_PACIB_ENABLED       (1ULL << 30)
// 29:28 RES1 11
// 27    PACDA_ENABLED AddPACDA and AuthDA functions enabled
#define SCTLR_PACDA_ENABLED       (1ULL << 27)
// 13    PACDB_ENABLED  AddPACDB and AuthDB functions enabled
#define SCTLR_PACDB_ENABLED       (1ULL << 13)

#define SCTLR_JOP_KEYS_ENABLED (SCTLR_PACIA_ENABLED | SCTLR_PACDA_ENABLED | SCTLR_PACDB_ENABLED)
#endif /* defined(HAS_APPLE_PAC) */

// 26    UCI User Cache Instructions
#define SCTLR_UCI_ENABLED         (1ULL << 26)

// 25    EE             Exception Endianness
#define SCTLR_EE_BIG_ENDIAN       (1ULL << 25)

// 24    E0E            EL0 Endianness
#define SCTLR_E0E_BIG_ENDIAN      (1ULL << 24)

// 23    SPAN           Set PAN
#define SCTLR_PAN_UNCHANGED       (1ULL << 23)

// 22    RES1           1
// 21    RES0           0
// 20    RES1           1

// 19    WXN            Writeable implies eXecute Never
#define SCTLR_WXN_ENABLED         (1ULL << 19)

// 18    nTWE           Not trap WFE from EL0
#define SCTLR_nTWE_WFE_ENABLED    (1ULL << 18)

// 17    RES0           0

// 16    nTWI           Not trap WFI from EL0
#define SCTRL_nTWI_WFI_ENABLED    (1ULL << 16)

// 15    UCT            User Cache Type register (CTR_EL0)
#define SCTLR_UCT_ENABLED         (1ULL << 15)

// 14    DZE            User Data Cache Zero (DC ZVA)
#define SCTLR_DZE_ENABLED         (1ULL << 14)

// 12    I              Instruction cache enable
#define SCTLR_I_ENABLED           (1ULL << 12)

// 11    RES1           1
// 10    RES0           0

// 9     UMA            User Mask Access
#define SCTLR_UMA_ENABLED         (1ULL << 9)

// 8     SED            SETEND Disable
#define SCTLR_SED_DISABLED        (1ULL << 8)

// 7     ITD            IT Disable
#define SCTLR_ITD_DISABLED        (1ULL << 7)

// 6     RES0           0

// 5     CP15BEN        CP15 Barrier ENable
#define SCTLR_CP15BEN_ENABLED     (1ULL << 5)

// 4     SA0            Stack Alignment check for EL0
#define SCTLR_SA0_ENABLED         (1ULL << 4)

// 3     SA             Stack Alignment check
#define SCTLR_SA_ENABLED          (1ULL << 3)

// 2     C              Cache enable
#define SCTLR_C_ENABLED           (1ULL << 2)

// 1     A              Alignment check
#define SCTLR_A_ENABLED           (1ULL << 1)

// 0     M              MMU enable
#define SCTLR_M_ENABLED           (1ULL << 0)

#define SCTLR_EL1_DEFAULT \
	(SCTLR_RESERVED | SCTLR_UCI_ENABLED | SCTLR_nTWE_WFE_ENABLED | SCTLR_DZE_ENABLED | \
	 SCTLR_I_ENABLED | SCTLR_SED_DISABLED | SCTLR_CP15BEN_ENABLED |                    \
	 SCTLR_SA0_ENABLED | SCTLR_SA_ENABLED | SCTLR_C_ENABLED | SCTLR_M_ENABLED)

/*
 * Coprocessor Access Control Register (CPACR)
 *
 *  31  28  27  22 21  20 19                 0
 * +---+---+------+------+--------------------+
 * |000|TTA|000000| FPEN |00000000000000000000|
 * +---+---+------+------+--------------------+
 *
 * where:
 *   TTA:  Trace trap
 *   FPEN: Floating point enable
 */
#define CPACR_TTA_SHIFT     28
#define CPACR_TTA           (1 << CPACR_TTA_SHIFT)

#define CPACR_FPEN_SHIFT    20
#define CPACR_FPEN_EL0_TRAP (0x1 << CPACR_FPEN_SHIFT)
#define CPACR_FPEN_ENABLE   (0x3 << CPACR_FPEN_SHIFT)

/*
 *  FPSR: Floating Point Status Register
 *
 *  31 30 29 28 27 26                  7   6  4   3   2   1   0
 * +--+--+--+--+--+-------------------+---+--+---+---+---+---+---+
 * | N| Z| C| V|QC|0000000000000000000|IDC|00|IXC|UFC|OFC|DZC|IOC|
 * +--+--+--+--+--+-------------------+---+--+---+---+---+---+---+
 */

#define FPSR_N_SHIFT   31
#define FPSR_Z_SHIFT   30
#define FPSR_C_SHIFT   29
#define FPSR_V_SHIFT   28
#define FPSR_QC_SHIFT  27
#define FPSR_IDC_SHIFT 7
#define FPSR_IXC_SHIFT 4
#define FPSR_UFC_SHIFT 3
#define FPSR_OFC_SHIFT 2
#define FPSR_DZC_SHIFT 1
#define FPSR_IOC_SHIFT 0
#define FPSR_N         (1 << FPSR_N_SHIFT)
#define FPSR_Z         (1 << FPSR_Z_SHIFT)
#define FPSR_C         (1 << FPSR_C_SHIFT)
#define FPSR_V         (1 << FPSR_V_SHIFT)
#define FPSR_QC        (1 << FPSR_QC_SHIFT)
#define FPSR_IDC       (1 << FPSR_IDC_SHIFT)
#define FPSR_IXC       (1 << FPSR_IXC_SHIFT)
#define FPSR_UFC       (1 << FPSR_UFC_SHIFT)
#define FPSR_OFC       (1 << FPSR_OFC_SHIFT)
#define FPSR_DZC       (1 << FPSR_DZC_SHIFT)
#define FPSR_IOC       (1 << FPSR_IOC_SHIFT)

/*
 * A mask for all for all of the bits that are not RAZ for FPSR; this
 * is primarily for converting between a 32-bit view of NEON state
 * (FPSCR) and a 64-bit view of NEON state (FPSR, FPCR).
 */
#define FPSR_MASK \
	(FPSR_N | FPSR_Z | FPSR_C | FPSR_V | FPSR_QC | FPSR_IDC | FPSR_IXC | \
	 FPSR_UFC | FPSR_OFC | FPSR_DZC | FPSR_IOC)

/*
 *  FPCR: Floating Point Control Register
 *
 *  31    26  25 24 23    21     19 18  15  14 12  11  10  9   8   7      0
 * +-----+---+--+--+-----+------+--+---+---+--+---+---+---+---+---+--------+
 * |00000|AHP|DN|FZ|RMODE|STRIDE| 0|LEN|IDE|00|IXE|UFE|OFE|DZE|IOE|00000000|
 * +-----+---+--+--+-----+------+--+---+---+--+---+---+---+---+---+--------+
 */

#define FPCR_AHP_SHIFT    26
#define FPCR_DN_SHIFT     25
#define FPCR_FZ_SHIFT     24
#define FPCR_RMODE_SHIFT  22
#define FPCR_STRIDE_SHIFT 20
#define FPCR_LEN_SHIFT    16
#define FPCR_IDE_SHIFT    15
#define FPCR_IXE_SHIFT    12
#define FPCR_UFE_SHIFT    11
#define FPCR_OFE_SHIFT    10
#define FPCR_DZE_SHIFT    9
#define FPCR_IOE_SHIFT    8
#define FPCR_AHP          (1 << FPCR_AHP_SHIFT)
#define FPCR_DN           (1 << FPCR_DN_SHIFT)
#define FPCR_FZ           (1 << FPCR_FZ_SHIFT)
#define FPCR_RMODE        (0x3 << FPCR_RMODE_SHIFT)
#define FPCR_STRIDE       (0x3 << FPCR_STRIDE_SHIFT)
#define FPCR_LEN          (0x7 << FPCR_LEN_SHIFT)
#define FPCR_IDE          (1 << FPCR_IDE_SHIFT)
#define FPCR_IXE          (1 << FPCR_IXE_SHIFT)
#define FPCR_UFE          (1 << FPCR_UFE_SHIFT)
#define FPCR_OFE          (1 << FPCR_OFE_SHIFT)
#define FPCR_DZE          (1 << FPCR_DZE_SHIFT)
#define FPCR_IOE          (1 << FPCR_IOE_SHIFT)
#define FPCR_DEFAULT      (FPCR_DN)
#define FPCR_DEFAULT_32   (FPCR_DN|FPCR_FZ)

/*
 * A mask for all for all of the bits that are not RAZ for FPCR; this
 * is primarily for converting between a 32-bit view of NEON state
 * (FPSCR) and a 64-bit view of NEON state (FPSR, FPCR).
 */
#define FPCR_MASK \
	(FPCR_AHP | FPCR_DN | FPCR_FZ | FPCR_RMODE | FPCR_STRIDE | FPCR_LEN | \
	 FPCR_IDE | FPCR_IXE | FPCR_UFE | FPCR_OFE | FPCR_DZE | FPCR_IOE)

/*
 * Translation Control Register (TCR)
 *
 * Legacy:
 *
 *  63  39   38   37 36   34 32    30 29 28 27 26 25 24   23 22 21  16    14 13 12 11 10 9   8    7   5  0
 * +------+----+----+--+-+-----+-+---+-----+-----+-----+----+--+------+-+---+-----+-----+-----+----+-+----+
 * | zero |TBI1|TBI0|AS|z| IPS |z|TG1| SH1 |ORGN1|IRGN1|EPD1|A1| T1SZ |z|TG0| SH0 |ORGN0|IRGN0|EPD0|z|T0SZ|
 * +------+----+----+--+-+-----+-+---+-----+-----+-----+----+--+------+-+---+-----+-----+-----+----+-+----+
 *
 * Current (with 16KB granule support):
 *
 *  63  39   38   37 36   34 32    30 29 28 27 26 25 24   23 22 21  16    14 13 12 11 10 9   8    7   5  0
 * +------+----+----+--+-+-----+-----+-----+-----+-----+----+--+------+-----+-----+-----+-----+----+-+----+
 * | zero |TBI1|TBI0|AS|z| IPS | TG1 | SH1 |ORGN1|IRGN1|EPD1|A1| T1SZ | TG0 | SH0 |ORGN0|IRGN0|EPD0|z|T0SZ|
 * +------+----+----+--+-+-----+-----+-----+-----+-----+----+--+------+-----+-----+-----+-----+----+-+----+
 *
 * TBI1:  Top Byte Ignored for TTBR1 region
 * TBI0:  Top Byte Ignored for TTBR0 region
 * AS:    ASID Size
 * IPS:   Physical Address Size limit
 * TG1:   Granule Size for TTBR1 region
 * SH1:   Shareability for TTBR1 region
 * ORGN1: Outer Cacheability for TTBR1 region
 * IRGN1: Inner Cacheability for TTBR1 region
 * EPD1:  Translation table walk disable for TTBR1
 * A1:    ASID selection from TTBR1 enable
 * T1SZ:  Virtual address size for TTBR1
 * TG0:   Granule Size for TTBR0 region
 * SH0:   Shareability for TTBR0 region
 * ORGN0: Outer Cacheability for TTBR0 region
 * IRGN0: Inner Cacheability for TTBR0 region
 * T0SZ:  Virtual address size for TTBR0
 */

#define TCR_T0SZ_SHIFT          0ULL
#define TCR_TSZ_BITS            6ULL
#define TCR_TSZ_MASK            ((1ULL << TCR_TSZ_BITS) - 1ULL)

#define TCR_IRGN0_SHIFT         8ULL
#define TCR_IRGN0_DISABLED      (0ULL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WRITEBACK     (1ULL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WRITETHRU     (2ULL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WRITEBACKNO   (3ULL << TCR_IRGN0_SHIFT)

#define TCR_ORGN0_SHIFT         10ULL
#define TCR_ORGN0_DISABLED      (0ULL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WRITEBACK     (1ULL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WRITETHRU     (2ULL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WRITEBACKNO   (3ULL << TCR_ORGN0_SHIFT)

#define TCR_SH0_SHIFT           12ULL
#define TCR_SH0_NONE            (0ULL << TCR_SH0_SHIFT)
#define TCR_SH0_OUTER           (2ULL << TCR_SH0_SHIFT)
#define TCR_SH0_INNER           (3ULL << TCR_SH0_SHIFT)

#define TCR_TG0_GRANULE_SHIFT   (14ULL)

#define TCR_TG0_GRANULE_4KB     (0ULL << TCR_TG0_GRANULE_SHIFT)
#define TCR_TG0_GRANULE_64KB    (1ULL << TCR_TG0_GRANULE_SHIFT)
#define TCR_TG0_GRANULE_16KB    (2ULL << TCR_TG0_GRANULE_SHIFT)

#if __ARM_16K_PG__
#define TCR_TG0_GRANULE_SIZE    (TCR_TG0_GRANULE_16KB)
#else
#define TCR_TG0_GRANULE_SIZE    (TCR_TG0_GRANULE_4KB)
#endif

#define TCR_T1SZ_SHIFT          16ULL

#define TCR_A1_ASID1            (1ULL << 22ULL)
#define TCR_EPD1_TTBR1_DISABLED (1ULL << 23ULL)

#define TCR_IRGN1_SHIFT          24ULL
#define TCR_IRGN1_DISABLED       (0ULL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WRITEBACK      (1ULL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WRITETHRU      (2ULL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WRITEBACKNO    (3ULL << TCR_IRGN1_SHIFT)

#define TCR_ORGN1_SHIFT          26ULL
#define TCR_ORGN1_DISABLED       (0ULL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WRITEBACK      (1ULL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WRITETHRU      (2ULL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WRITEBACKNO    (3ULL << TCR_ORGN1_SHIFT)

#define TCR_SH1_SHIFT            28ULL
#define TCR_SH1_NONE             (0ULL << TCR_SH1_SHIFT)
#define TCR_SH1_OUTER            (2ULL << TCR_SH1_SHIFT)
#define TCR_SH1_INNER            (3ULL << TCR_SH1_SHIFT)

#define TCR_TG1_GRANULE_SHIFT    30ULL

#define TCR_TG1_GRANULE_16KB     (1ULL << TCR_TG1_GRANULE_SHIFT)
#define TCR_TG1_GRANULE_4KB      (2ULL << TCR_TG1_GRANULE_SHIFT)
#define TCR_TG1_GRANULE_64KB     (3ULL << TCR_TG1_GRANULE_SHIFT)

#if __ARM_16K_PG__
#define TCR_TG1_GRANULE_SIZE     (TCR_TG1_GRANULE_16KB)
#else
#define TCR_TG1_GRANULE_SIZE     (TCR_TG1_GRANULE_4KB)
#endif

#define TCR_IPS_SHIFT            32ULL
#define TCR_IPS_32BITS           (0ULL << TCR_IPS_SHIFT)
#define TCR_IPS_36BITS           (1ULL << TCR_IPS_SHIFT)
#define TCR_IPS_40BITS           (2ULL << TCR_IPS_SHIFT)
#define TCR_IPS_42BITS           (3ULL << TCR_IPS_SHIFT)
#define TCR_IPS_44BITS           (4ULL << TCR_IPS_SHIFT)
#define TCR_IPS_48BITS           (5ULL << TCR_IPS_SHIFT)

#define TCR_AS_16BIT_ASID        (1ULL << 36)
#define TCR_TBI0_TOPBYTE_IGNORED (1ULL << 37)
#define TCR_TBI1_TOPBYTE_IGNORED (1ULL << 38)
#define TCR_TBID0_TBI_DATA_ONLY  (1ULL << 51)
#define TCR_TBID1_TBI_DATA_ONLY  (1ULL << 52)

#if defined(HAS_APPLE_PAC)
#define TCR_TBID0_ENABLE         TCR_TBID0_TBI_DATA_ONLY
#else
#define TCR_TBID0_ENABLE         0
#endif

/*
 * Multiprocessor Affinity Register (MPIDR_EL1)
 *
 * +64-----------------------------31+30+29-25+24+23-16+15-8+7--0+
 * |000000000000000000000000000000001| U|00000|MT| Aff2|Aff1|Aff0|
 * +---------------------------------+--+-----+--+-----+----+----+
 *
 * where
 *   U:    Uniprocessor
 *   MT:   Multi-threading at lowest affinity level
 *   Aff2: "1" - PCORE, "0" - ECORE
 *   Aff1: Cluster ID
 *   Aff0: CPU ID
 */
#define MPIDR_AFF0_SHIFT 0
#define MPIDR_AFF0_WIDTH 8
#define MPIDR_AFF0_MASK  (((1 << MPIDR_AFF0_WIDTH) - 1) << MPIDR_AFF0_SHIFT)
#define MPIDR_AFF1_SHIFT 8
#define MPIDR_AFF1_WIDTH 8
#define MPIDR_AFF1_MASK  (((1 << MPIDR_AFF1_WIDTH) - 1) << MPIDR_AFF1_SHIFT)
#define MPIDR_AFF2_SHIFT 16
#define MPIDR_AFF2_WIDTH 8
#define MPIDR_AFF2_MASK  (((1 << MPIDR_AFF2_WIDTH) - 1) << MPIDR_AFF2_SHIFT)

/*
 * TXSZ indicates the size of the range a TTBR covers.  Currently,
 * we support the following:
 *
 * 4KB pages, full page L1: 39 bit range.
 * 4KB pages, sub-page L1: 38 bit range.
 * 16KB pages, full page L1: 47 bit range.
 * 16KB pages, sub-page L1: 39 bit range.
 * 16KB pages, two level page tables: 36 bit range.
 */
#if __ARM_KERNEL_PROTECT__
/*
 * If we are configured to use __ARM_KERNEL_PROTECT__, the first half of the
 * address space is used for the mappings that will remain in place when in EL0.
 * As a result, 1 bit less of address space is available to the rest of the
 * the kernel.
 */
#endif /* __ARM_KERNEL_PROTECT__ */
#ifdef __ARM_16K_PG__
#if __ARM64_PMAP_SUBPAGE_L1__
#define T0SZ_BOOT 25ULL
#else /* !__ARM64_PMAP_SUBPAGE_L1__ */
#define T0SZ_BOOT 17ULL
#endif /* !__ARM64_PMAP_SUBPAGE_L1__ */
#else /* __ARM_16K_PG__ */
#if __ARM64_PMAP_SUBPAGE_L1__
#define T0SZ_BOOT 26ULL
#else /* __ARM64_PMAP_SUBPAGE_L1__ */
#define T0SZ_BOOT 25ULL
#endif /* __ARM64_PMAP_SUBPAGE_L1__ */
#endif /* __ARM_16K_PG__ */

#if defined(APPLE_ARM64_ARCH_FAMILY)
/* T0SZ must be the same as T1SZ */
#define T1SZ_BOOT T0SZ_BOOT
#else /* defined(APPLE_ARM64_ARCH_FAMILY) */
#ifdef __ARM_16K_PG__
#if __ARM64_PMAP_SUBPAGE_L1__
#define T1SZ_BOOT 25ULL
#else /* !__ARM64_PMAP_SUBPAGE_L1__ */
#define T1SZ_BOOT 17ULL
#endif /* !__ARM64_PMAP_SUBPAGE_L1__ */
#else /* __ARM_16K_PG__ */
#if __ARM64_PMAP_SUBPAGE_L1__
#define T1SZ_BOOT 26ULL
#else /* __ARM64_PMAP_SUBPAGE_L1__ */
#define T1SZ_BOOT 25ULL
#endif /*__ARM64_PMAP_SUBPAGE_L1__*/
#endif /* __ARM_16K_PG__ */
#endif /* defined(APPLE_ARM64_ARCH_FAMILY) */

#if __ARM_42BIT_PA_SPACE__
#define TCR_IPS_VALUE TCR_IPS_42BITS
#else /* !__ARM_42BIT_PA_SPACE__ */
#define TCR_IPS_VALUE TCR_IPS_40BITS
#endif /* !__ARM_42BIT_PA_SPACE__ */

#define TCR_EL1_BASE \
	(TCR_IPS_VALUE | TCR_SH0_OUTER | TCR_ORGN0_WRITEBACK |         \
	 TCR_IRGN0_WRITEBACK | (T0SZ_BOOT << TCR_T0SZ_SHIFT) |          \
	 (TCR_TG0_GRANULE_SIZE) | TCR_SH1_OUTER | TCR_ORGN1_WRITEBACK | \
	 TCR_IRGN1_WRITEBACK | (TCR_TG1_GRANULE_SIZE) |                 \
	 TCR_TBI0_TOPBYTE_IGNORED | (TCR_TBID0_ENABLE))

#if __ARM_KERNEL_PROTECT__
#define TCR_EL1_BOOT (TCR_EL1_BASE | (T1SZ_BOOT << TCR_T1SZ_SHIFT))
#define T1SZ_USER (T1SZ_BOOT + 1)
#define TCR_EL1_USER (TCR_EL1_BASE | (T1SZ_USER << TCR_T1SZ_SHIFT))
#else
#define TCR_EL1_BOOT (TCR_EL1_BASE | (T1SZ_BOOT << TCR_T1SZ_SHIFT))
#endif /* __ARM_KERNEL_PROTECT__ */


/*
 * Translation Table Base Register (TTBR)
 *
 *  63    48 47               x x-1  0
 * +--------+------------------+------+
 * |  ASID  |   Base Address   | zero |
 * +--------+------------------+------+
 *
 */
#define TTBR_ASID_SHIFT 48
#define TTBR_ASID_MASK  0xffff000000000000

#define TTBR_BADDR_MASK 0x0000ffffffffffff

/*
 * Memory Attribute Indirection Register
 *
 *  63   56 55   48 47   40 39   32 31   24 23   16 15    8 7     0
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 * | Attr7 | Attr6 | Attr5 | Attr4 | Attr3 | Attr2 | Attr1 | Attr0 |
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 *
 */

#define MAIR_ATTR_SHIFT(x)          (8*(x))

/* Strongly ordered or device memory attributes */
#define MAIR_OUTER_STRONGLY_ORDERED 0x0
#define MAIR_OUTER_DEVICE           0x0

#define MAIR_INNER_STRONGLY_ORDERED 0x0
#define MAIR_INNER_DEVICE           0x4

/* Normal memory attributes */
#define MAIR_OUTER_NON_CACHEABLE    0x40
#define MAIR_OUTER_WRITE_THROUGH    0x80
#define MAIR_OUTER_WRITE_BACK       0xc0

#define MAIR_INNER_NON_CACHEABLE    0x4
#define MAIR_INNER_WRITE_THROUGH    0x8
#define MAIR_INNER_WRITE_BACK       0xc

/* Allocate policy for cacheable memory */
#define MAIR_OUTER_WRITE_ALLOCATE   0x10
#define MAIR_OUTER_READ_ALLOCATE    0x20

#define MAIR_INNER_WRITE_ALLOCATE   0x1
#define MAIR_INNER_READ_ALLOCATE    0x2

/* Memory Atribute Encoding */

/*
 * Device memory types:
 * G (gathering): multiple reads/writes can be combined
 * R (reordering): reads or writes may reach device out of program order
 * E (early-acknowledge): writes may return immediately (e.g. PCIe posted writes)
 */
#define MAIR_DISABLE                   0x00 /* Device Memory, nGnRnE (strongly ordered) */
#define MAIR_POSTED                    0x04 /* Device Memory, nGnRE (strongly ordered, posted writes) */
#define MAIR_POSTED_REORDERED          0x08 /* Device Memory, nGRE (reorderable, posted writes) */
#define MAIR_POSTED_COMBINED_REORDERED 0x0C /* Device Memory, GRE (reorderable, gathered writes, posted writes) */
#define MAIR_WRITECOMB                 0x44 /* Normal Memory, Outer Non-Cacheable, Inner Non-Cacheable */
#define MAIR_WRITETHRU                 0xBB /* Normal Memory, Outer Write-through, Inner Write-through */
#define MAIR_WRITEBACK                 0xFF /* Normal Memory, Outer Write-back, Inner Write-back */
#define MAIR_INNERWRITEBACK            0x4F /* Normal Memory, Outer Non-Cacheable, Inner Write-back */


/*
 * ARM 4-level Page Table support - 2*1024TB (2^48) of address space
 */


/*
 *  Memory Attribute Index
 */
#define CACHE_ATTRINDX_WRITEBACK                 0x0 /* cache enabled, buffer enabled  (normal memory) */
#define CACHE_ATTRINDX_WRITECOMB                 0x1 /* no cache, buffered writes (normal memory) */
#define CACHE_ATTRINDX_WRITETHRU                 0x2 /* cache enabled, buffer disabled (normal memory) */
#define CACHE_ATTRINDX_DISABLE                   0x3 /* no cache, no buffer (device memory) */
#define CACHE_ATTRINDX_INNERWRITEBACK            0x4 /* inner cache enabled, buffer enabled, write allocate (normal memory) */
#define CACHE_ATTRINDX_POSTED                    0x5 /* no cache, no buffer, posted writes (device memory) */
#define CACHE_ATTRINDX_POSTED_REORDERED          0x6 /* no cache, reorderable access, posted writes (device memory) */
#define CACHE_ATTRINDX_POSTED_COMBINED_REORDERED 0x7 /* no cache, write gathering, reorderable access, posted writes (device memory) */
#define CACHE_ATTRINDX_DEFAULT                   CACHE_ATTRINDX_WRITEBACK


/*
 * Access protection bit values (TTEs and PTEs), stage 1
 *
 * Bit 1 controls access type (1=RO, 0=RW), bit 0 controls user (1=access, 0=no access)
 */
#define AP_RWNA 0x0 /* priv=read-write, user=no-access */
#define AP_RWRW 0x1 /* priv=read-write, user=read-write */
#define AP_RONA 0x2 /* priv=read-only, user=no-access */
#define AP_RORO 0x3 /* priv=read-only, user=read-only */
#define AP_MASK 0x3 /* mask to find ap bits */

/*
 * Shareability attributes
 */
#define SH_NONE         0x0 /* Non shareable  */
#define SH_NONE         0x0 /* Device shareable */
#define SH_DEVICE       0x2 /* Normal memory Inner non shareable - Outer non shareable */
#define SH_OUTER_MEMORY 0x2 /* Normal memory Inner shareable - Outer shareable */
#define SH_INNER_MEMORY 0x3 /* Normal memory Inner shareable - Outer non shareable */


/*
 * ARM Page Granule
 */
#ifdef __ARM_16K_PG__
#define ARM_PGSHIFT 14
#else
#define ARM_PGSHIFT 12
#endif
#define ARM_PGBYTES (1 << ARM_PGSHIFT)
#define ARM_PGMASK  (ARM_PGBYTES-1)

/*
 *  L0 Translation table
 *
 *  4KB granule size:
 *    Each translation table is 4KB
 *    512 64-bit entries of 512GB (2^39) of address space.
 *    Covers 256TB (2^48) of address space.
 *
 *  16KB granule size:
 *    Each translation table is 16KB
 *    2 64-bit entries of 128TB (2^47) of address space.
 *    Covers 256TB (2^48) of address space.
 */

/* 16K L0 */
#define ARM_16K_TT_L0_SIZE       0x0000800000000000ULL /* size of area covered by a tte */
#define ARM_16K_TT_L0_OFFMASK    0x00007fffffffffffULL /* offset within an L0 entry */
#define ARM_16K_TT_L0_SHIFT      47                    /* page descriptor shift */
#define ARM_16K_TT_L0_INDEX_MASK 0x0000800000000000ULL /* mask for getting index in L0 table from virtual address */

/* 4K L0 */
#define ARM_4K_TT_L0_SIZE       0x0000008000000000ULL /* size of area covered by a tte */
#define ARM_4K_TT_L0_OFFMASK    0x0000007fffffffffULL /* offset within an L0 entry */
#define ARM_4K_TT_L0_SHIFT      39                    /* page descriptor shift */
#define ARM_4K_TT_L0_INDEX_MASK 0x0000ff8000000000ULL /* mask for getting index in L0 table from virtual address */

/*
 *  L1 Translation table
 *
 *  4KB granule size:
 *    Each translation table is 4KB
 *    512 64-bit entries of 1GB (2^30) of address space.
 *    Covers 512GB (2^39) of address space.
 *
 *  16KB granule size:
 *    Each translation table is 16KB
 *    2048 64-bit entries of 64GB (2^36) of address space.
 *    Covers 128TB (2^47) of address space.
 */

/* 16K L1 */
#define ARM_16K_TT_L1_SIZE       0x0000001000000000ULL /* size of area covered by a tte */
#define ARM_16K_TT_L1_OFFMASK    0x0000000fffffffffULL /* offset within an L1 entry */
#define ARM_16K_TT_L1_SHIFT      36                    /* page descriptor shift */
#ifdef __ARM64_PMAP_SUBPAGE_L1__
/* This config supports 512GB per TTBR. */
#define ARM_16K_TT_L1_INDEX_MASK 0x0000007000000000ULL /* mask for getting index into L1 table from virtual address */
#else /* __ARM64_PMAP_SUBPAGE_L1__ */
#define ARM_16K_TT_L1_INDEX_MASK 0x00007ff000000000ULL /* mask for getting index into L1 table from virtual address */
#endif /* __ARM64_PMAP_SUBPAGE_L1__ */

/* 4K L1 */
#define ARM_4K_TT_L1_SIZE       0x0000000040000000ULL /* size of area covered by a tte */
#define ARM_4K_TT_L1_OFFMASK    0x000000003fffffffULL /* offset within an L1 entry */
#define ARM_4K_TT_L1_SHIFT      30                    /* page descriptor shift */
#ifdef __ARM64_PMAP_SUBPAGE_L1__
/* This config supports 256GB per TTBR. */
#define ARM_4K_TT_L1_INDEX_MASK 0x0000003fc0000000ULL /* mask for getting index into L1 table from virtual address */
#else /* __ARM64_PMAP_SUBPAGE_L1__ */
#define ARM_4K_TT_L1_INDEX_MASK 0x0000007fc0000000ULL /* mask for getting index into L1 table from virtual address */
#endif /* __ARM64_PMAP_SUBPAGE_L1__ */

/* some sugar for getting pointers to page tables and entries */

#define L1_TABLE_INDEX(va) (((va) & ARM_TT_L1_INDEX_MASK) >> ARM_TT_L1_SHIFT)
#define L2_TABLE_INDEX(va) (((va) & ARM_TT_L2_INDEX_MASK) >> ARM_TT_L2_SHIFT)
#define L3_TABLE_INDEX(va) (((va) & ARM_TT_L3_INDEX_MASK) >> ARM_TT_L3_SHIFT)

#define L2_TABLE_VA(tte)  ((tt_entry_t*) phystokv((*(tte)) & ARM_TTE_TABLE_MASK))
#define L3_TABLE_VA(tte2) ((pt_entry_t*) phystokv((*(tte2)) & ARM_TTE_TABLE_MASK))

/*
 *  L2 Translation table
 *
 *  4KB granule size:
 *    Each translation table is 4KB
 *    512 64-bit entries of 2MB (2^21) of address space.
 *    Covers 1GB (2^30) of address space.
 *
 *  16KB granule size:
 *    Each translation table is 16KB
 *    2048 64-bit entries of 32MB (2^25) of address space.
 *    Covers 64GB (2^36) of address space.
 */

/* 16K L2 */
#define ARM_16K_TT_L2_SIZE       0x0000000002000000ULL /* size of area covered by a tte */
#define ARM_16K_TT_L2_OFFMASK    0x0000000001ffffffULL /* offset within an L2 entry */
#define ARM_16K_TT_L2_SHIFT      25                    /* page descriptor shift */
#define ARM_16K_TT_L2_INDEX_MASK 0x0000000ffe000000ULL /* mask for getting index in L2 table from virtual address */

/* 4K L2 */
#define ARM_4K_TT_L2_SIZE       0x0000000000200000ULL /* size of area covered by a tte */
#define ARM_4K_TT_L2_OFFMASK    0x00000000001fffffULL /* offset within an L2 entry */
#define ARM_4K_TT_L2_SHIFT      21                    /* page descriptor shift */
#define ARM_4K_TT_L2_INDEX_MASK 0x000000003fe00000ULL /* mask for getting index in L2 table from virtual address */

/*
 *  L3 Translation table
 *
 *  4KB granule size:
 *    Each translation table is 4KB
 *    512 64-bit entries of 4KB (2^12) of address space.
 *    Covers 2MB (2^21) of address space.
 *
 *  16KB granule size:
 *    Each translation table is 16KB
 *    2048 64-bit entries of 16KB (2^14) of address space.
 *    Covers 32MB (2^25) of address space.
 */

/* 16K L3 */
#define ARM_16K_TT_L3_SIZE       0x0000000000004000ULL /* size of area covered by a tte */
#define ARM_16K_TT_L3_OFFMASK    0x0000000000003fffULL /* offset within L3 PTE */
#define ARM_16K_TT_L3_SHIFT      14                    /* page descriptor shift */
#define ARM_16K_TT_L3_INDEX_MASK 0x0000000001ffc000ULL /* mask for page descriptor index */

/* 4K L3 */
#define ARM_4K_TT_L3_SIZE       0x0000000000001000ULL /* size of area covered by a tte */
#define ARM_4K_TT_L3_OFFMASK    0x0000000000000fffULL /* offset within L3 PTE */
#define ARM_4K_TT_L3_SHIFT      12                    /* page descriptor shift */
#define ARM_4K_TT_L3_INDEX_MASK 0x00000000001ff000ULL /* mask for page descriptor index */

#ifdef __ARM_16K_PG__

/* Native L0 defines */
#define ARM_TT_L0_SIZE       ARM_16K_TT_L0_SIZE
#define ARM_TT_L0_OFFMASK    ARM_16K_TT_L0_OFFMASK
#define ARM_TT_L0_SHIFT      ARM_16K_TT_L0_SHIFT
#define ARM_TT_L0_INDEX_MASK ARM_16K_TT_L0_INDEX_MASK

/* Native L1 defines */
#define ARM_TT_L1_SIZE       ARM_16K_TT_L1_SIZE
#define ARM_TT_L1_OFFMASK    ARM_16K_TT_L1_OFFMASK
#define ARM_TT_L1_SHIFT      ARM_16K_TT_L1_SHIFT
#define ARM_TT_L1_INDEX_MASK ARM_16K_TT_L1_INDEX_MASK

/* Native L2 defines */
#define ARM_TT_L2_SIZE       ARM_16K_TT_L2_SIZE
#define ARM_TT_L2_OFFMASK    ARM_16K_TT_L2_OFFMASK
#define ARM_TT_L2_SHIFT      ARM_16K_TT_L2_SHIFT
#define ARM_TT_L2_INDEX_MASK ARM_16K_TT_L2_INDEX_MASK

/* Native L3 defines */
#define ARM_TT_L3_SIZE       ARM_16K_TT_L3_SIZE
#define ARM_TT_L3_OFFMASK    ARM_16K_TT_L3_OFFMASK
#define ARM_TT_L3_SHIFT      ARM_16K_TT_L3_SHIFT
#define ARM_TT_L3_INDEX_MASK ARM_16K_TT_L3_INDEX_MASK

#else /* !__ARM_16K_PG__ */

/* Native L0 defines */
#define ARM_TT_L0_SIZE       ARM_4K_TT_L0_SIZE
#define ARM_TT_L0_OFFMASK    ARM_4K_TT_L0_OFFMASK
#define ARM_TT_L0_SHIFT      ARM_4K_TT_L0_SHIFT
#define ARM_TT_L0_INDEX_MASK ARM_4K_TT_L0_INDEX_MASK

/* Native L1 defines */
#define ARM_TT_L1_SIZE       ARM_4K_TT_L1_SIZE
#define ARM_TT_L1_OFFMASK    ARM_4K_TT_L1_OFFMASK
#define ARM_TT_L1_SHIFT      ARM_4K_TT_L1_SHIFT
#define ARM_TT_L1_INDEX_MASK ARM_4K_TT_L1_INDEX_MASK

/* Native L2 defines */
#define ARM_TT_L2_SIZE       ARM_4K_TT_L2_SIZE
#define ARM_TT_L2_OFFMASK    ARM_4K_TT_L2_OFFMASK
#define ARM_TT_L2_SHIFT      ARM_4K_TT_L2_SHIFT
#define ARM_TT_L2_INDEX_MASK ARM_4K_TT_L2_INDEX_MASK

/* Native L3 defines */
#define ARM_TT_L3_SIZE       ARM_4K_TT_L3_SIZE
#define ARM_TT_L3_OFFMASK    ARM_4K_TT_L3_OFFMASK
#define ARM_TT_L3_SHIFT      ARM_4K_TT_L3_SHIFT
#define ARM_TT_L3_INDEX_MASK ARM_4K_TT_L3_INDEX_MASK

#endif /* !__ARM_16K_PG__ */

/*
 * Convenience definitions for:
 *   ARM_TT_LEAF: The last level of the configured page table format.
 *   ARM_TT_TWIG: The second to last level of the configured page table format.
 *   ARM_TT_ROOT: The first level of the configured page table format.
 *
 *   My apologies to any botanists who may be reading this.
 */
#define ARM_TT_LEAF_SIZE       ARM_TT_L3_SIZE
#define ARM_TT_LEAF_OFFMASK    ARM_TT_L3_OFFMASK
#define ARM_TT_LEAF_SHIFT      ARM_TT_L3_SHIFT
#define ARM_TT_LEAF_INDEX_MASK ARM_TT_L3_INDEX_MASK

#define ARM_TT_TWIG_SIZE       ARM_TT_L2_SIZE
#define ARM_TT_TWIG_OFFMASK    ARM_TT_L2_OFFMASK
#define ARM_TT_TWIG_SHIFT      ARM_TT_L2_SHIFT
#define ARM_TT_TWIG_INDEX_MASK ARM_TT_L2_INDEX_MASK

#define ARM_TT_ROOT_SIZE       ARM_TT_L1_SIZE
#define ARM_TT_ROOT_OFFMASK    ARM_TT_L1_OFFMASK
#define ARM_TT_ROOT_SHIFT      ARM_TT_L1_SHIFT
#define ARM_TT_ROOT_INDEX_MASK ARM_TT_L1_INDEX_MASK

/*
 * 4KB granule size:
 *
 * Level 0 Translation Table Entry
 *
 *  63 62 61 60  59 58   52 51  48 47                  12 11    2 1 0
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 * |NS|  AP |XN|PXN|ignored| zero | L1TableOutputAddress |ignored|1|V|
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 *
 * Level 1 Translation Table Entry
 *
 *  63 62 61 60  59 58   52 51  48 47                  12 11    2 1 0
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 * |NS|  AP |XN|PXN|ignored| zero | L2TableOutputAddress |ignored|1|V|
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 *
 * Level 1 Translation Block Entry
 *
 *  63 59 58  55 54  53   52 51  48 47                  30 29  12 11 10 9  8 7  6  5 4     2 1 0
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 * | ign |sw use|XN|PXN|HINT| zero | OutputAddress[47:30] | zero |nG|AF| SH | AP |NS|AttrIdx|0|V|
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 *
 * Level 2 Translation Table Entry
 *
 *  63 62 61 60  59 58   52 51  48 47                  12 11    2 1 0
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 * |NS|  AP |XN|PXN|ignored| zero | L3TableOutputAddress |ignored|1|V|
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 *
 * Level 2 Translation Block Entry
 *
 *  63 59 58  55 54  53   52 51  48 47                  21 20  12 11 10 9  8 7  6  5 4     2 1 0
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 * | ign |sw use|XN|PXN|HINT| zero | OutputAddress[47:21] | zero |nG|AF| SH | AP |NS|AttrIdx|0|V|
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 *
 * 16KB granule size:
 *
 * Level 0 Translation Table Entry
 *
 *  63 62 61 60  59 58   52 51  48 47                  14 13    2 1 0
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 * |NS|  AP |XN|PXN|ignored| zero | L1TableOutputAddress |ignored|1|V|
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 *
 * Level 1 Translation Table Entry
 *
 *  63 62 61 60  59 58   52 51  48 47                  14 13    2 1 0
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 * |NS|  AP |XN|PXN|ignored| zero | L2TableOutputAddress |ignored|1|V|
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 *
 * Level 2 Translation Table Entry
 *
 *  63 62 61 60  59 58   52 51  48 47                  14 13    2 1 0
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 * |NS|  AP |XN|PXN|ignored| zero | L3TableOutputAddress |ignored|1|V|
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 *
 * Level 2 Translation Block Entry
 *
 *  63 59 58  55 54  53   52 51  48 47                  25 24  12 11 10 9  8 7  6  5 4     2 1 0
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 * | ign |sw use|XN|PXN|HINT| zero | OutputAddress[47:25] | zero |nG|AF| SH | AP |NS|AttrIdx|0|V|
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 *
 * where:
 *   nG:      notGlobal bit
 *   SH:      Shareability field
 *   AP:      access protection
 *   XN:      eXecute Never bit
 *   PXN:     Privilege eXecute Never bit
 *   NS:      Non-Secure bit
 *   HINT:    16 entry continuguous output hint
 *   AttrIdx: Memory Attribute Index
 */

#define TTE_SHIFT                   3                              /* shift width of a tte (sizeof(tte) == (1 << TTE_SHIFT)) */
#ifdef __ARM_16K_PG__
#define TTE_PGENTRIES               (16384 >> TTE_SHIFT)           /* number of ttes per page */
#else
#define TTE_PGENTRIES               (4096 >> TTE_SHIFT)            /* number of ttes per page */
#endif

#define ARM_TTE_MAX                 (TTE_PGENTRIES)

#define ARM_TTE_EMPTY               0x0000000000000000ULL          /* unasigned - invalid entry */
#define ARM_TTE_TYPE_FAULT          0x0000000000000000ULL          /* unasigned - invalid entry */

#define ARM_TTE_VALID               0x0000000000000001ULL          /* valid entry */

#define ARM_TTE_TYPE_MASK           0x0000000000000002ULL          /* mask for extracting the type */
#define ARM_TTE_TYPE_TABLE          0x0000000000000002ULL          /* page table type */
#define ARM_TTE_TYPE_BLOCK          0x0000000000000000ULL          /* block entry type */
#define ARM_TTE_TYPE_L3BLOCK        0x0000000000000002ULL
#define ARM_TTE_TYPE_MASK           0x0000000000000002ULL          /* mask for extracting the type */

#ifdef __ARM_16K_PG__
/*
 * Note that L0/L1 block entries are disallowed for the 16KB granule size; what
 * are we doing with these?
 */
#define ARM_TTE_BLOCK_SHIFT         12                             /* entry shift for a 16KB L3 TTE entry */
#define ARM_TTE_BLOCK_L0_SHIFT      ARM_TT_L0_SHIFT                /* block shift for 128TB section */
#define ARM_TTE_BLOCK_L1_MASK       0x0000fff000000000ULL          /* mask to extract phys address from L1 block entry */
#define ARM_TTE_BLOCK_L1_SHIFT      ARM_TT_L1_SHIFT                /* block shift for 64GB section */
#define ARM_TTE_BLOCK_L2_MASK       0x0000fffffe000000ULL          /* mask to extract phys address from Level 2 Translation Block entry */
#define ARM_TTE_BLOCK_L2_SHIFT      ARM_TT_L2_SHIFT                /* block shift for 32MB section */
#else
#define ARM_TTE_BLOCK_SHIFT         12                             /* entry shift for a 4KB L3 TTE entry */
#define ARM_TTE_BLOCK_L0_SHIFT      ARM_TT_L0_SHIFT                /* block shift for 2048GB section */
#define ARM_TTE_BLOCK_L1_MASK       0x0000ffffc0000000ULL          /* mask to extract phys address from L1 block entry */
#define ARM_TTE_BLOCK_L1_SHIFT      ARM_TT_L1_SHIFT                /* block shift for 1GB section */
#define ARM_TTE_BLOCK_L2_MASK       0x0000ffffffe00000ULL          /* mask to extract phys address from Level 2 Translation Block entry */
#define ARM_TTE_BLOCK_L2_SHIFT      ARM_TT_L2_SHIFT                /* block shift for 2MB section */
#endif

#define ARM_TTE_BLOCK_APSHIFT       6
#define ARM_TTE_BLOCK_AP(x)         ((x)<<ARM_TTE_BLOCK_APSHIFT)   /* access protection */
#define ARM_TTE_BLOCK_APMASK        (0x3 << ARM_TTE_BLOCK_APSHIFT)

#define ARM_TTE_BLOCK_ATTRINDX(x)   ((x) << 2)                     /* memory attributes index */
#define ARM_TTE_BLOCK_ATTRINDXMASK  (0x7ULL << 2)                  /* mask memory attributes index */

#define ARM_TTE_BLOCK_SH(x)         ((x) << 8)                     /* access shared */
#define ARM_TTE_BLOCK_SHMASK        (0x3ULL << 8)                  /* mask access shared */

#define ARM_TTE_BLOCK_AF            0x0000000000000400ULL          /* value for access */
#define ARM_TTE_BLOCK_AFMASK        0x0000000000000400ULL          /* access mask */

#define ARM_TTE_BLOCK_NG            0x0000000000000800ULL          /* value for a global mapping */
#define ARM_TTE_BLOCK_NG_MASK       0x0000000000000800ULL          /* notGlobal mapping mask */

#define ARM_TTE_BLOCK_NS            0x0000000000000020ULL          /* value for a secure mapping */
#define ARM_TTE_BLOCK_NS_MASK       0x0000000000000020ULL          /* notSecure mapping mask */

#define ARM_TTE_BLOCK_PNX           0x0020000000000000ULL          /* value for privilege no execute bit */
#define ARM_TTE_BLOCK_PNXMASK       0x0020000000000000ULL          /* privilege no execute mask */

#define ARM_TTE_BLOCK_NX            0x0040000000000000ULL          /* value for no execute */
#define ARM_TTE_BLOCK_NXMASK        0x0040000000000000ULL          /* no execute mask */

#define ARM_TTE_BLOCK_WIRED         0x0400000000000000ULL          /* value for software wired bit */
#define ARM_TTE_BLOCK_WIREDMASK     0x0400000000000000ULL          /* software wired mask */

#define ARM_TTE_BLOCK_WRITEABLE     0x0800000000000000ULL          /* value for software writeable bit */
#define ARM_TTE_BLOCK_WRITEABLEMASK 0x0800000000000000ULL          /* software writeable mask */

#ifdef __ARM_16K_PG__
/*
 * TODO: Do we care about the low bits being unused?  It should technically
 * work either way, but masking them out should be future proof; it is only a
 * matter of time before someone wants to shove something into the free bits.
 */
#define ARM_TTE_TABLE_MASK          (0x0000ffffffffc000ULL)        /* mask for extracting pointer to next table (works at any level) */
#else
#define ARM_TTE_TABLE_MASK          (0x0000fffffffff000ULL)        /* mask for extracting pointer to next table (works at any level) */
#endif

#define ARM_TTE_TABLE_APSHIFT       61
#define ARM_TTE_TABLE_AP(x)         ((x)<<TTE_BLOCK_APSHIFT)       /* access protection */

#define ARM_TTE_TABLE_NS            0x8000000000000020ULL          /* value for a secure mapping */
#define ARM_TTE_TABLE_NS_MASK       0x8000000000000020ULL          /* notSecure mapping mask */

#define ARM_TTE_TABLE_XN            0x1000000000000000ULL          /* value for no execute */
#define ARM_TTE_TABLE_XNMASK        0x1000000000000000ULL          /* no execute mask */

#define ARM_TTE_TABLE_PXN           0x0800000000000000ULL          /* value for privilege no execute bit */
#define ARM_TTE_TABLE_PXNMASK       0x0800000000000000ULL          /* privilege execute mask */

#if __ARM_KERNEL_PROTECT__
#define ARM_TTE_BOOT_BLOCK \
	(ARM_TTE_TYPE_BLOCK | ARM_TTE_VALID | ARM_TTE_BLOCK_SH(SH_OUTER_MEMORY) | \
	 ARM_TTE_BLOCK_ATTRINDX(CACHE_ATTRINDX_WRITEBACK) | ARM_TTE_BLOCK_AF | ARM_TTE_BLOCK_NG)
#else /* __ARM_KERNEL_PROTECT__ */
#define ARM_TTE_BOOT_BLOCK \
	(ARM_TTE_TYPE_BLOCK | ARM_TTE_VALID | ARM_TTE_BLOCK_SH(SH_OUTER_MEMORY) | \
	 ARM_TTE_BLOCK_ATTRINDX(CACHE_ATTRINDX_WRITEBACK) | ARM_TTE_BLOCK_AF)
#endif /* __ARM_KERNEL_PROTECT__ */

#define ARM_TTE_BOOT_TABLE (ARM_TTE_TYPE_TABLE | ARM_TTE_VALID )
/*
 *  L3 Translation table
 *
 *  4KB granule size:
 *    Each translation table is 4KB
 *    512 64-bit entries of 4KB (2^12) of address space.
 *    Covers 2MB (2^21) of address space.
 *
 *  16KB granule size:
 *    Each translation table is 16KB
 *    2048 64-bit entries of 16KB (2^14) of address space.
 *    Covers 32MB (2^25) of address space.
 */

#ifdef __ARM_16K_PG__
#define ARM_PTE_SIZE    0x0000000000004000ULL /* size of area covered by a tte */
#define ARM_PTE_OFFMASK 0x0000000000003fffULL /* offset within pte area */
#define ARM_PTE_SHIFT   14                    /* page descriptor shift */
#define ARM_PTE_MASK    0x0000ffffffffc000ULL /* mask for output address in PTE */
#else
#define ARM_PTE_SIZE    0x0000000000001000ULL /* size of area covered by a tte */
#define ARM_PTE_OFFMASK 0x0000000000000fffULL /* offset within pte area */
#define ARM_PTE_SHIFT   12                    /* page descriptor shift */
#define ARM_PTE_MASK    0x0000fffffffff000ULL /* mask for output address in PTE */
#endif

/*
 * L3 Page table entries
 *
 * The following page table entry types are possible:
 *
 * fault page entry
 *  63                            2  0
 * +------------------------------+--+
 * |    ignored                   |00|
 * +------------------------------+--+
 *
 *
 *  63 59 58  55 54  53   52 51  48 47                  12 11 10 9  8 7  6  5 4     2 1 0
 * +-----+------+--+---+----+------+----------------------+--+--+----+----+--+-------+-+-+
 * | ign |sw use|XN|PXN|HINT| zero | OutputAddress[47:12] |nG|AF| SH | AP |NS|AttrIdx|1|V|
 * +-----+------+--+---+----+------+----------------------+--+--+----+----+--+-------+-+-+
 *
 * where:
 *   nG:      notGlobal bit
 *   SH:      Shareability field
 *   AP:      access protection
 *   XN:      eXecute Never bit
 *   PXN:     Privilege eXecute Never bit
 *   NS:      Non-Secure bit
 *   HINT:    16 entry continuguous output hint
 *   AttrIdx: Memory Attribute Index
 */

#define PTE_SHIFT               3                     /* shift width of a pte (sizeof(pte) == (1 << PTE_SHIFT)) */
#ifdef __ARM_16K_PG__
#define PTE_PGENTRIES           (16384 >> PTE_SHIFT)  /* number of ptes per page */
#else
#define PTE_PGENTRIES           (4096 >> PTE_SHIFT)   /* number of ptes per page */
#endif

#define ARM_PTE_EMPTY           0x0000000000000000ULL /* unassigned - invalid entry */

/* markers for (invalid) PTE for a page sent to compressor */
#define ARM_PTE_COMPRESSED      0x8000000000000000ULL /* compressed... */
#define ARM_PTE_COMPRESSED_ALT  0x4000000000000000ULL /* ... and was "alt_acct" */
#define ARM_PTE_COMPRESSED_MASK 0xC000000000000000ULL

#define ARM_PTE_IS_COMPRESSED(x, p) \
	((((x) & 0x3) == 0) && /* PTE is not valid... */                      \
	 ((x) & ARM_PTE_COMPRESSED) && /* ...has "compressed" marker" */      \
	 ((!((x) & ~ARM_PTE_COMPRESSED_MASK)) || /* ...no other bits */       \
	 (panic("compressed PTE %p 0x%llx has extra bits 0x%llx: corrupted?", \
	        (p), (x), (x) & ~ARM_PTE_COMPRESSED_MASK), FALSE)))

#define ARM_PTE_TYPE               0x0000000000000003ULL /* valid L3 entry: includes bit #1 (counterintuitively) */
#define ARM_PTE_TYPE_VALID         0x0000000000000003ULL /* valid L3 entry: includes bit #1 (counterintuitively) */
#define ARM_PTE_TYPE_FAULT         0x0000000000000000ULL /* invalid L3 entry */
#define ARM_PTE_TYPE_MASK          0x0000000000000002ULL /* mask to get pte type */

#ifdef __ARM_16K_PG__
/* TODO: What does the shift mean here? */
#define ARM_PTE_PAGE_MASK          0x0000FFFFFFFFC000ULL /* mask for 16KB page */
#else
#define ARM_PTE_PAGE_MASK          0x0000FFFFFFFFF000ULL /* mask for  4KB page */
#define ARM_PTE_PAGE_SHIFT         12                    /* page shift for 4KB page */
#endif

#define ARM_PTE_AP(x)              ((x) << 6)            /* access protections */
#define ARM_PTE_APMASK             (0x3ULL << 6)         /* mask access protections */
#define ARM_PTE_EXTRACT_AP(x)      (((x) >> 6) & 0x3ULL) /* extract access protections from PTE */

#define ARM_PTE_ATTRINDX(x)        ((x) << 2)            /* memory attributes index */
#define ARM_PTE_ATTRINDXMASK       (0x7ULL << 2)         /* mask memory attributes index */

#define ARM_PTE_SH(x)              ((x) << 8)            /* access shared */
#define ARM_PTE_SHMASK             (0x3ULL << 8)         /* mask access shared */

#define ARM_PTE_AF                 0x0000000000000400ULL /* value for access */
#define ARM_PTE_AFMASK             0x0000000000000400ULL /* access mask */

#define ARM_PTE_NG                 0x0000000000000800ULL /* value for a global mapping */
#define ARM_PTE_NG_MASK            0x0000000000000800ULL /* notGlobal mapping mask */

#define ARM_PTE_NS                 0x0000000000000020ULL /* value for a secure mapping */
#define ARM_PTE_NS_MASK            0x0000000000000020ULL /* notSecure mapping mask */

#define ARM_PTE_HINT               0x0010000000000000ULL /* value for contiguous entries hint */
#define ARM_PTE_HINT_MASK          0x0010000000000000ULL /* mask for contiguous entries hint */

#if __ARM_16K_PG__
#define ARM_PTE_HINT_ENTRIES       128ULL                /* number of entries the hint covers */
#define ARM_PTE_HINT_ENTRIES_SHIFT 7ULL                  /* shift to construct the number of entries */
#define ARM_PTE_HINT_ADDR_MASK     0x0000FFFFFFE00000ULL /* mask to extract the starting hint address */
#define ARM_PTE_HINT_ADDR_SHIFT    21                    /* shift for the hint address */
#define ARM_KVA_HINT_ADDR_MASK     0xFFFFFFFFFFE00000ULL /* mask to extract the starting hint address */
#else
#define ARM_PTE_HINT_ENTRIES       16ULL                 /* number of entries the hint covers */
#define ARM_PTE_HINT_ENTRIES_SHIFT 4ULL                  /* shift to construct the number of entries */
#define ARM_PTE_HINT_ADDR_MASK     0x0000FFFFFFFF0000ULL /* mask to extract the starting hint address */
#define ARM_PTE_HINT_ADDR_SHIFT    16                    /* shift for the hint address */
#define ARM_KVA_HINT_ADDR_MASK     0xFFFFFFFFFFFF0000ULL /* mask to extract the starting hint address */
#endif

#define ARM_PTE_PNX                0x0020000000000000ULL /* value for privilege no execute bit */
#define ARM_PTE_PNXMASK            0x0020000000000000ULL /* privilege no execute mask */

#define ARM_PTE_NX                 0x0040000000000000ULL /* value for no execute bit */
#define ARM_PTE_NXMASK             0x0040000000000000ULL /* no execute mask */

#define ARM_PTE_WIRED              0x0400000000000000ULL /* value for software wired bit */
#define ARM_PTE_WIRED_MASK         0x0400000000000000ULL /* software wired mask */

#define ARM_PTE_WRITEABLE          0x0800000000000000ULL /* value for software writeable bit */
#define ARM_PTE_WRITEABLE_MASK     0x0800000000000000ULL /* software writeable mask */

#if CONFIG_PGTRACE
#define ARM_PTE_PGTRACE            0x0200000000000000ULL /* value for software trace bit */
#define ARM_PTE_PGTRACE_MASK       0x0200000000000000ULL /* software trace mask */
#endif

#define ARM_PTE_BOOT_PAGE_BASE \
	(ARM_PTE_TYPE_VALID | ARM_PTE_SH(SH_OUTER_MEMORY) |       \
	 ARM_PTE_ATTRINDX(CACHE_ATTRINDX_WRITEBACK) | ARM_PTE_AF)

#if __ARM_KERNEL_PROTECT__
#define ARM_PTE_BOOT_PAGE (ARM_PTE_BOOT_PAGE_BASE | ARM_PTE_NG)
#else /* __ARM_KERNEL_PROTECT__ */
#define ARM_PTE_BOOT_PAGE (ARM_PTE_BOOT_PAGE_BASE)
#endif /* __ARM_KERNEL_PROTECT__ */

/*
 * TLBI appers to only deal in 4KB page addresses, so give
 * it an explicit shift of 12.
 */
#define TLBI_ADDR_SHIFT (0)
#define TLBI_ADDR_SIZE  (44)
#define TLBI_ADDR_MASK  ((1ULL << TLBI_ADDR_SIZE) - 1)
#define TLBI_ASID_SHIFT (48)
#define TLBI_ASID_SIZE  (16)
#define TLBI_ASID_MASK  (((1ULL << TLBI_ASID_SIZE) - 1))

#define RTLBI_ADDR_SIZE (37)
#define RTLBI_ADDR_MASK ((1ULL << RTLBI_ADDR_SIZE) - 1)
#define RTLBI_ADDR_SHIFT ARM_TT_L3_SHIFT
#define RTLBI_TG ((uint64_t)(((ARM_TT_L3_SHIFT - 12) >> 1) + 1) << 46)
#define RTLBI_SCALE_SHIFT (44)
#define RTLBI_NUM_SHIFT (39)

/*
 * Exception Syndrome Register
 *
 *  31  26 25 24               0
 * +------+--+------------------+
 * |  EC  |IL|       ISS        |
 * +------+--+------------------+
 *
 * EC  - Exception Class
 * IL  - Instruction Length
 * ISS - Instruction Specific Syndrome
 *
 * Note: The ISS can have many forms. These are defined separately below.
 */

#define ESR_EC_SHIFT           26
#define ESR_EC_MASK            (0x3FULL << ESR_EC_SHIFT)
#define ESR_EC(x)              ((x & ESR_EC_MASK) >> ESR_EC_SHIFT)

#define ESR_IL_SHIFT           25
#define ESR_IL                 (1 << ESR_IL_SHIFT)

#define ESR_INSTR_IS_2BYTES(x) (!(x & ESR_IL))

#define ESR_ISS_MASK           0x01FFFFFF
#define ESR_ISS(x)             (x & ESR_ISS_MASK)

#ifdef __ASSEMBLER__
/* Define only the classes we need to test in the exception vectors. */
#define ESR_EC_IABORT_EL1      0x21
#define ESR_EC_DABORT_EL1      0x25
#define ESR_EC_SP_ALIGN        0x26
#else
typedef enum {
	ESR_EC_UNCATEGORIZED       = 0x00,
	ESR_EC_WFI_WFE             = 0x01,
	ESR_EC_MCR_MRC_CP15_TRAP   = 0x03,
	ESR_EC_MCRR_MRRC_CP15_TRAP = 0x04,
	ESR_EC_MCR_MRC_CP14_TRAP   = 0x05,
	ESR_EC_LDC_STC_CP14_TRAP   = 0x06,
	ESR_EC_TRAP_SIMD_FP        = 0x07,
	ESR_EC_MCRR_MRRC_CP14_TRAP = 0x0c,
	ESR_EC_ILLEGAL_INSTR_SET   = 0x0e,
	ESR_EC_SVC_32              = 0x11,
	ESR_EC_SVC_64              = 0x15,
	ESR_EC_MSR_TRAP            = 0x18,
	ESR_EC_IABORT_EL0          = 0x20,
	ESR_EC_IABORT_EL1          = 0x21,
	ESR_EC_PC_ALIGN            = 0x22,
	ESR_EC_DABORT_EL0          = 0x24,
	ESR_EC_DABORT_EL1          = 0x25,
	ESR_EC_SP_ALIGN            = 0x26,
	ESR_EC_FLOATING_POINT_32   = 0x28,
	ESR_EC_FLOATING_POINT_64   = 0x2C,
	ESR_EC_BKPT_REG_MATCH_EL0  = 0x30, // Breakpoint Debug event taken to the EL from a lower EL.
	ESR_EC_BKPT_REG_MATCH_EL1  = 0x31, // Breakpoint Debug event taken to the EL from the EL.
	ESR_EC_SW_STEP_DEBUG_EL0   = 0x32, // Software Step Debug event taken to the EL from a lower EL.
	ESR_EC_SW_STEP_DEBUG_EL1   = 0x33, // Software Step Debug event taken to the EL from the EL.
	ESR_EC_WATCHPT_MATCH_EL0   = 0x34, // Watchpoint Debug event taken to the EL from a lower EL.
	ESR_EC_WATCHPT_MATCH_EL1   = 0x35, // Watchpoint Debug event taken to the EL from the EL.
	ESR_EC_BKPT_AARCH32        = 0x38,
	ESR_EC_BRK_AARCH64         = 0x3C,
} esr_exception_class_t;

typedef enum {
	FSC_TRANSLATION_FAULT_L0   = 0x04,
	FSC_TRANSLATION_FAULT_L1   = 0x05,
	FSC_TRANSLATION_FAULT_L2   = 0x06,
	FSC_TRANSLATION_FAULT_L3   = 0x07,
	FSC_ACCESS_FLAG_FAULT_L1   = 0x09,
	FSC_ACCESS_FLAG_FAULT_L2   = 0x0A,
	FSC_ACCESS_FLAG_FAULT_L3   = 0x0B,
	FSC_PERMISSION_FAULT_L1    = 0x0D,
	FSC_PERMISSION_FAULT_L2    = 0x0E,
	FSC_PERMISSION_FAULT_L3    = 0x0F,
	FSC_SYNC_EXT_ABORT         = 0x10,
	FSC_ASYNC_EXT_ABORT        = 0x11,
	FSC_SYNC_EXT_ABORT_TT_L1   = 0x15,
	FSC_SYNC_EXT_ABORT_TT_L2   = 0x16,
	FSC_SYNC_EXT_ABORT_TT_L3   = 0x17,
	FSC_SYNC_PARITY            = 0x18,
	FSC_ASYNC_PARITY           = 0x19,
	FSC_SYNC_PARITY_TT_L1      = 0x1D,
	FSC_SYNC_PARITY_TT_L2      = 0x1E,
	FSC_SYNC_PARITY_TT_L3      = 0x1F,
	FSC_ALIGNMENT_FAULT        = 0x21,
	FSC_DEBUG_FAULT            = 0x22
} fault_status_t;
#endif /* ASSEMBLER */

/*
 * Software step debug event ISS (EL1)
 *  24  23                6  5    0
 * +---+-----------------+--+------+
 * |ISV|00000000000000000|EX| IFSC |
 * +---+-----------------+--+------+
 *
 * where:
 *   ISV:  Instruction syndrome valid
 *   EX:   Exclusive access
 *   IFSC: Instruction Fault Status Code
 */

#define ISS_SSDE_ISV_SHIFT 24
#define ISS_SSDE_ISV       (0x1 << ISS_SSDE_ISV_SHIFT)

#define ISS_SSDE_EX_SHIFT  6
#define ISS_SSDE_EX        (0x1 << ISS_SSDE_EX_SHIFT)

#define ISS_SSDE_FSC_MASK  0x3F
#define ISS_SSDE_FSC(x)    (x & ISS_SSDE_FSC_MASK)

/*
 * Instruction Abort ISS (EL1)
 *  24           10 9      5    0
 * +---------------+--+---+------+
 * |000000000000000|EA|000| IFSC |
 * +---------------+--+---+------+
 *
 * where:
 *   EA:   External Abort type
 *   IFSC: Instruction Fault Status Code
 */

#define ISS_IA_EA_SHIFT 9
#define ISS_IA_EA       (0x1 << ISS_IA_EA_SHIFT)

#define ISS_IA_FSC_MASK 0x3F
#define ISS_IA_FSC(x)   (x & ISS_IA_FSC_MASK)


/*
 * Data Abort ISS (EL1)
 *
 *  24              9  8  7  6  5  0
 * +---------------+--+--+-+---+----+
 * |000000000000000|EA|CM|0|WnR|DFSC|
 * +---------------+--+--+-+---+----+
 *
 * where:
 *   EA:   External Abort type
 *   CM:   Cache Maintenance operation
 *   WnR:  Write not Read
 *   DFSC: Data Fault Status Code
 */
#define ISS_DA_EA_SHIFT  9
#define ISS_DA_EA        (0x1 << ISS_DA_EA_SHIFT)

#define ISS_DA_CM_SHIFT  8
#define ISS_DA_CM        (0x1 << ISS_DA_CM_SHIFT)

#define ISS_DA_WNR_SHIFT 6
#define ISS_DA_WNR       (0x1 << ISS_DA_WNR_SHIFT)

#define ISS_DA_FSC_MASK  0x3F
#define ISS_DA_FSC(x)    (x & ISS_DA_FSC_MASK)

/*
 * Floating Point Exception ISS (EL1)
 *
 * 24  23 22            8  7      4   3   2   1   0
 * +-+---+---------------+---+--+---+---+---+---+---+
 * |0|TFV|000000000000000|IDF|00|IXF|UFF|OFF|DZF|IOF|
 * +-+---+---------------+---+--+---+---+---+---+---+
 *
 * where:
 *   TFV: Trapped Fault Valid
 *   IDF: Input Denormal Exception
 *   IXF: Input Inexact Exception
 *   UFF: Underflow Exception
 *   OFF: Overflow Exception
 *   DZF: Divide by Zero Exception
 *   IOF: Invalid Operation Exception
 */
#define ISS_FP_TFV_SHIFT 23
#define ISS_FP_TFV       (0x1 << ISS_FP_TFV_SHIFT)

#define ISS_FP_IDF_SHIFT 7
#define ISS_FP_IDF       (0x1 << ISS_FP_IDF_SHIFT)

#define ISS_FP_IXF_SHIFT 4
#define ISS_FP_IXF       (0x1 << ISS_FP_IXF_SHIFT)

#define ISS_FP_UFF_SHIFT 3
#define ISS_FP_UFF       (0x1 << ISS_FP_UFF_SHIFT)

#define ISS_FP_OFF_SHIFT 2
#define ISS_FP_OFF       (0x1 << ISS_FP_OFF_SHIFT)

#define ISS_FP_DZF_SHIFT 1
#define ISS_FP_DZF       (0x1 << ISS_FP_DZF_SHIFT)

#define ISS_FP_IOF_SHIFT 0
#define ISS_FP_IOF       (0x1 << ISS_FP_IOF_SHIFT)


/*
 * Physical Address Register (EL1)
 */
#define PAR_F_SHIFT 0
#define PAR_F       (0x1 << PAR_F_SHIFT)

#define PLATFORM_SYSCALL_TRAP_NO 0x80000000

#define ARM64_SYSCALL_CODE_REG_NUM (16)

#define ARM64_CLINE_SHIFT 6

#if defined(APPLE_ARM64_ARCH_FAMILY)
#define L2CERRSTS_DATSBEESV (1ULL << 2) /* L2C data single bit ECC error */
#define L2CERRSTS_DATDBEESV (1ULL << 4) /* L2C data double bit ECC error */
#endif

/*
 * Timer definitions.
 */
#define CNTKCTL_EL1_PL0PTEN      (0x1 << 9)           /* 1: EL0 access to physical timer regs permitted */
#define CNTKCTL_EL1_PL0VTEN      (0x1 << 8)           /* 1: EL0 access to virtual timer regs permitted */
#define CNTKCTL_EL1_EVENTI_MASK  (0x000000f0)         /* Mask for bits describing which bit to use for triggering event stream */
#define CNTKCTL_EL1_EVENTI_SHIFT (0x4)                /* Shift for same */
#define CNTKCTL_EL1_EVENTDIR     (0x1 << 3)           /* 1: one-to-zero transition of specified bit causes event */
#define CNTKCTL_EL1_EVNTEN       (0x1 << 2)           /* 1: enable event stream */
#define CNTKCTL_EL1_PL0VCTEN     (0x1 << 1)           /* 1: EL0 access to physical timebase + frequency reg enabled */
#define CNTKCTL_EL1_PL0PCTEN     (0x1 << 0)           /* 1: EL0 access to virtual timebase + frequency reg enabled */

#define CNTV_CTL_EL0_ISTATUS     (0x1 << 2)           /* (read only): whether interrupt asserted */
#define CNTV_CTL_EL0_IMASKED     (0x1 << 1)           /* 1: interrupt masked */
#define CNTV_CTL_EL0_ENABLE      (0x1 << 0)           /* 1: virtual timer enabled */

#define CNTP_CTL_EL0_ISTATUS     CNTV_CTL_EL0_ISTATUS
#define CNTP_CTL_EL0_IMASKED     CNTV_CTL_EL0_IMASKED
#define CNTP_CTL_EL0_ENABLE      CNTV_CTL_EL0_ENABLE

/*
 * At present all other uses of ARM_DBG_* are shared bit compatibly with the 32bit definitons.
 * (cf. osfmk/arm/proc_reg.h)
 */
#define ARM_DBG_VR_ADDRESS_MASK64 0xFFFFFFFFFFFFFFFCull /* BVR & WVR */

#define MIDR_EL1_REV_SHIFT  0
#define MIDR_EL1_REV_MASK   (0xf << MIDR_EL1_REV_SHIFT)
#define MIDR_EL1_PNUM_SHIFT 4
#define MIDR_EL1_PNUM_MASK  (0xfff << MIDR_EL1_PNUM_SHIFT)
#define MIDR_EL1_ARCH_SHIFT 16
#define MIDR_EL1_ARCH_MASK  (0xf << MIDR_EL1_ARCH_SHIFT)
#define MIDR_EL1_VAR_SHIFT  20
#define MIDR_EL1_VAR_MASK   (0xf << MIDR_EL1_VAR_SHIFT)
#define MIDR_EL1_IMP_SHIFT  24
#define MIDR_EL1_IMP_MASK   (0xff << MIDR_EL1_IMP_SHIFT)

/*
 * CoreSight debug registers
 */
#define CORESIGHT_ED  0
#define CORESIGHT_CTI 1
#define CORESIGHT_PMU 2
#define CORESIGHT_UTT 3 /* Not truly a coresight thing, but at a fixed convenient location right after the coresight region */

#define CORESIGHT_OFFSET(x) ((x) * 0x10000)
#define CORESIGHT_REGIONS   4
#define CORESIGHT_SIZE      0x1000










/*
 * ID_AA64ISAR0_EL1 - AArch64 Instruction Set Attribute Register 0
 *
 *  63      24 23    20 19  16 15  12 11   8 7   4 3    0
 * +----------+--------+------+------+------+-----+------+
 * | reserved | atomic |crc32 | sha2 | sha1 | aes | res0 |
 * +----------+--------+------+------+------+-----+------+
 */

#define ID_AA64ISAR0_EL1_FHM_OFFSET    48
#define ID_AA64ISAR0_EL1_FHM_MASK      (0xfull << ID_AA64ISAR0_EL1_FHM_OFFSET)
#define ID_AA64ISAR0_EL1_FHM_8_2       (1ull << ID_AA64ISAR0_EL1_FHM_OFFSET)

#define ID_AA64ISAR0_EL1_ATOMIC_OFFSET 20
#define ID_AA64ISAR0_EL1_ATOMIC_MASK   (0xfull << ID_AA64ISAR0_EL1_ATOMIC_OFFSET)
#define ID_AA64ISAR0_EL1_ATOMIC_8_1    (2ull << ID_AA64ISAR0_EL1_ATOMIC_OFFSET)

#define ID_AA64ISAR0_EL1_CRC32_OFFSET  16
#define ID_AA64ISAR0_EL1_CRC32_MASK    (0xfull << ID_AA64ISAR0_EL1_CRC32_OFFSET)
#define ID_AA64ISAR0_EL1_CRC32_EN      (1ull << ID_AA64ISAR0_EL1_CRC32_OFFSET)

#define ID_AA64ISAR0_EL1_SHA2_OFFSET   12
#define ID_AA64ISAR0_EL1_SHA2_MASK     (0xfull << ID_AA64ISAR0_EL1_SHA2_OFFSET)
#define ID_AA64ISAR0_EL1_SHA2_EN       (1ull << ID_AA64ISAR0_EL1_SHA2_OFFSET)

#define ID_AA64ISAR0_EL1_SHA1_OFFSET   8
#define ID_AA64ISAR0_EL1_SHA1_MASK     (0xfull << ID_AA64ISAR0_EL1_SHA1_OFFSET)
#define ID_AA64ISAR0_EL1_SHA1_EN       (1ull << ID_AA64ISAR0_EL1_SHA1_OFFSET)

#define ID_AA64ISAR0_EL1_AES_OFFSET    4
#define ID_AA64ISAR0_EL1_AES_MASK      (0xfull << ID_AA64ISAR0_EL1_AES_OFFSET)
#define ID_AA64ISAR0_EL1_AES_EN        (1ull << ID_AA64ISAR0_EL1_AES_OFFSET)
#define ID_AA64ISAR0_EL1_AES_PMULL_EN  (2ull << ID_AA64ISAR0_EL1_AES_OFFSET)


#if __APCFG_SUPPORTED__
/*
 * APCFG_EL1
 *
 *  63       2 1 0
 * +----------+-+-+
 * | reserved |K|R|
 * +----------+-+-+
 *
 * where:
 *   R: Reserved
 *   K: ElXEnKey - Enable ARMV8.3 defined {IA,IB,DA,DB} keys when CPU is
 *                 operating in EL1 (or higher) and when under Apple-Mode
 */

#define APCFG_EL1_ELXENKEY_OFFSET      1
#define APCFG_EL1_ELXENKEY_MASK        (0x1ULL << APCFG_EL1_ELXENKEY_OFFSET)
#define APCFG_EL1_ELXENKEY             APCFG_EL1_ELXENKEY_MASK
#endif /* __APCFG_SUPPORTED__ */

#define APSTATE_G_SHIFT  (0)
#define APSTATE_P_SHIFT  (1)
#define APSTATE_A_SHIFT  (2)

#ifdef __APSTS_SUPPORTED__
#define APCTL_EL1_AppleMode  (1ULL << 0)
#define APCTL_EL1_KernKeyEn  (1ULL << 1)
#define APCTL_EL1_EnAPKey0   (1ULL << 2)
#define APCTL_EL1_EnAPKey1   (1ULL << 3)
#define APSTS_EL1_MKEYVld    (1ULL << 0)
#else
#define APCTL_EL1_AppleMode  (1ULL << 0)
#define APCTL_EL1_MKEYVld    (1ULL << 1)
#define APCTL_EL1_KernKeyEn  (1ULL << 2)
#endif

#define ACTLR_EL1_DisHWP_OFFSET  3
#define ACTLR_EL1_DisHWP_MASK    (1ULL << ACTLR_EL1_DisHWP_OFFSET)
#define ACTLR_EL1_DisHWP         ACTLR_EL1_DisHWP_MASK


#if defined(HAS_APPLE_PAC)
// The value of ptrauth_string_discriminator("recover"), hardcoded so it can be used from assembly code
#define PAC_DISCRIMINATOR_RECOVER    0x1e02
#endif

#ifdef __ASSEMBLER__

/*
 * Compute CPU version:
 * Version is constructed as [4 bits of MIDR variant]:[4 bits of MIDR revision]
 *
 * Where the "variant" is the major number and the "revision" is the minor number.
 *
 * For example:
 *   Cyclone A0 is variant 0, revision 0, i.e. 0.
 *   Cyclone B0 is variant 1, revision 0, i.e. 0x10
 * $0 - register to place value in
 */
.macro GET_MIDR_CPU_VERSION
mrs  $0, MIDR_EL1                                  // Read MIDR_EL1 for CPUID
bfi  $0, $0, #(MIDR_EL1_VAR_SHIFT - 4), #4         // move bits 3:0 (revision) to 19:16 (below variant) to get values adjacent
ubfx $0, $0, #(MIDR_EL1_VAR_SHIFT - 4), #8         // And extract the concatenated bitstring to beginning of register
.endmacro

/*
 * To apply a workaround for CPU versions less than a given value
 * (e.g. earlier than when a fix arrived)
 *
 * $0 - scratch register1
 * $1 - version at which to stop applying workaround
 * $2 - label to branch to  (at end of workaround)
 */
.macro SKIP_IF_CPU_VERSION_GREATER_OR_EQUAL
GET_MIDR_CPU_VERSION $0
cmp  $0, $1
b.pl $2                         // Unsigned "greater or equal"
.endmacro

/*
 * To apply a workaround for CPU versions greater than a given value
 * (e.g. starting when a bug was introduced)
 *
 * $0 - scratch register1
 * $1 - version at which to stop applying workaround
 * $2 - label to branch to  (at end of workaround)
 */
.macro SKIP_IF_CPU_VERSION_LESS_THAN
GET_MIDR_CPU_VERSION $0
cmp  $0, $1
b.mi $2                         // Unsigned "strictly less than"
.endmacro

#endif /* __ASSEMBLER__ */

#define MSR(reg, src)  __asm__ volatile ("msr " reg ", %0" :: "r" (src))
#define MRS(dest, reg) __asm__ volatile ("mrs %0, " reg : "=r" (dest))


#endif /* _ARM64_PROC_REG_H_ */
