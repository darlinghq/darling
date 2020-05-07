/*
 * Copyright (c) 2007-2018 Apple Inc. All rights reserved.
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
/* CMU_ENDHIST */
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

/*
 * Processor registers for ARM
 */
#ifndef _ARM_PROC_REG_H_
#define _ARM_PROC_REG_H_

#if defined (__arm64__)
#include <pexpert/arm64/board_config.h>
#elif defined (__arm__)
#include <pexpert/arm/board_config.h>
#endif

#if defined (ARMA7)
#define __ARM_ARCH__               7
#define __ARM_SUB_ARCH__           CPU_ARCH_ARMv7k
#define __ARM_VMSA__               7
#define __ARM_VFP__                3
#if defined(__XNU_UP__)
#define __ARM_SMP__                0
#else
#define __ARM_SMP__                1
/* For SMP kernels, force physical aperture to be mapped at PTE level so that its mappings
 * can be updated to reflect cache attribute changes on alias mappings.  This prevents
 * prefetched physical aperture cachelines from becoming dirty in L1 due to a write to
 * an uncached alias mapping on the same core.  Subsequent uncached writes from another
 * core may not snoop this line, and the dirty line may end up being evicted later to
 * effectively overwrite the uncached writes from other cores. */
#define __ARM_PTE_PHYSMAP__        1
#endif
/* __ARMA7_SMP__ controls whether we are consistent with the A7 MP_CORE spec; needed because entities other than
 * the xnu-managed processors may need to snoop our cache operations.
 */
#define __ARMA7_SMP__              1
#define __ARM_COHERENT_CACHE__     1
#define __ARM_DEBUG__              7
#define __ARM_USER_PROTECT__       1
#define __ARM_TIME_TIMEBASE_ONLY__ 1

#elif defined (APPLETYPHOON)
#define __ARM_ARCH__                 8
#define __ARM_VMSA__                 8
#define __ARM_SMP__                  1
#define __ARM_VFP__                  4
#define __ARM_COHERENT_CACHE__       1
#define __ARM_COHERENT_IO__          1
#define __ARM_IC_NOALIAS_ICACHE__    1
#define __ARM_DEBUG__                7
#define __ARM_ENABLE_SWAP__          1
#define __ARM_V8_CRYPTO_EXTENSIONS__ 1
#define __ARM64_PMAP_SUBPAGE_L1__    1
#define __ARM_KERNEL_PROTECT__       1

#elif defined (APPLETWISTER)
#define __ARM_ARCH__                 8
#define __ARM_VMSA__                 8
#define __ARM_SMP__                  1
#define __ARM_VFP__                  4
#define __ARM_COHERENT_CACHE__       1
#define __ARM_COHERENT_IO__          1
#define __ARM_IC_NOALIAS_ICACHE__    1
#define __ARM_DEBUG__                7
#define __ARM_ENABLE_SWAP__          1
#define __ARM_V8_CRYPTO_EXTENSIONS__ 1
#define __ARM_16K_PG__               1
#define __ARM64_PMAP_SUBPAGE_L1__    1
#define __ARM_KERNEL_PROTECT__       1

#elif defined (APPLEHURRICANE)
#define __ARM_ARCH__                 8
#define __ARM_VMSA__                 8
#define __ARM_SMP__                  1
#define __ARM_VFP__                  4
#define __ARM_COHERENT_CACHE__       1
#define __ARM_COHERENT_IO__          1
#define __ARM_IC_NOALIAS_ICACHE__    1
#define __ARM_DEBUG__                7
#define __ARM_ENABLE_SWAP__          1
#define __ARM_V8_CRYPTO_EXTENSIONS__ 1
#define __ARM_16K_PG__               1
#define __ARM64_PMAP_SUBPAGE_L1__    1
#define __ARM_KERNEL_PROTECT__       1
#define __ARM_GLOBAL_SLEEP_BIT__     1
#define __ARM_PAN_AVAILABLE__        1

#elif defined (APPLEMONSOON)
#define __ARM_ARCH__                         8
#define __ARM_VMSA__                         8
#define __ARM_SMP__                          1
#define __ARM_AMP__                          1
#define __ARM_VFP__                          4
#define __ARM_COHERENT_CACHE__               1
#define __ARM_COHERENT_IO__                  1
#define __ARM_IC_NOALIAS_ICACHE__            1
#define __ARM_DEBUG__                        7
#define __ARM_ENABLE_SWAP__                  1
#define __ARM_V8_CRYPTO_EXTENSIONS__         1
#define __ARM_16K_PG__                       1
#define __ARM64_PMAP_SUBPAGE_L1__            1
#define __ARM_KERNEL_PROTECT__               1
#define __ARM_GLOBAL_SLEEP_BIT__             1
#define __ARM_PAN_AVAILABLE__                1
#define __ARM_WKDM_ISA_AVAILABLE__           1
#define __PLATFORM_WKDM_ALIGNMENT_MASK__     (0x3FULL)
#define __PLATFORM_WKDM_ALIGNMENT_BOUNDARY__ (64)
#define __ARM_CLUSTER_COUNT__                2

#elif defined (BCM2837)
#define __ARM_ARCH__              8
#define __ARM_VMSA__              8
#define __ARM_SMP__               1
#define __ARM_VFP__               4
#define __ARM_COHERENT_CACHE__    1
#define __ARM_DEBUG__             7
#define __ARM64_PMAP_SUBPAGE_L1__ 1
#else
#error processor not supported
#endif

#if __ARM_42BIT_PA_SPACE__
/* For now, force the issue! */
#undef __ARM64_PMAP_SUBPAGE_L1__
#endif /* __ARM_42BIT_PA_SPACE__ */

#if __ARM_KERNEL_PROTECT__
/*
 * This feature is not currently implemented for 32-bit ARM CPU architectures.
 * A discussion of this feature for 64-bit ARM CPU architectures can be found
 * in the ARM64 version of this file.
 */
#if __arm__
#error __ARM_KERNEL_PROTECT__ is not supported on ARM32
#endif /* __arm__ */
#endif /* __ARM_KERNEL_PROTECT__ */

#if defined(ARM_BOARD_WFE_TIMEOUT_NS)
#define __ARM_ENABLE_WFE_ 1
#else /* defined(ARM_BOARD_WFE_TIMEOUT_NS) */
#define __ARM_ENABLE_WFE_ 0
#endif /* defined(ARM_BOARD_WFE_TIMEOUT_NS) */

/*
 * The clutch scheduler is enabled only on non-AMP platforms for now.
 */
#if !__ARM_AMP__ && CONFIG_CLUTCH
#define CONFIG_SCHED_CLUTCH 1
#else /* !__ARM_AMP__ && CONFIG_CLUTCH */
#define CONFIG_SCHED_CLUTCH 0
#endif /* !__ARM_AMP__ && CONFIG_CLUTCH */

#if __ARM_AMP__ || CONFIG_SCHED_CLUTCH
#define CONFIG_THREAD_GROUPS 1
#else /* __ARM_AMP__ || CONFIG_SCHED_CLUTCH */
#define CONFIG_THREAD_GROUPS 0
#endif

#ifdef XNU_KERNEL_PRIVATE

#if __ARM_VFP__
#define ARM_VFP_DEBUG 0
#endif /* __ARM_VFP__ */

#endif /* XNU_KERNEL_PRIVATE */



/*
 * FSR registers
 *
 * CPSR: Current Program Status Register
 * SPSR: Saved Program Status Registers
 *
 *  31 30 29 28 27     24     19   16      9  8  7  6  5  4   0
 * +-----------------------------------------------------------+
 * | N| Z| C| V| Q|...| J|...|GE[3:0]|...| E| A| I| F| T| MODE |
 * +-----------------------------------------------------------+
 */

/*
 * Flags
 */
#define PSR_NF 0x80000000 /* Negative/Less than */
#define PSR_ZF 0x40000000 /* Zero */
#define PSR_CF 0x20000000 /* Carry/Borrow/Extend */
#define PSR_VF 0x10000000 /* Overflow */
#define PSR_QF 0x08000000 /* saturation flag (QADD ARMv5) */

/*
 * Modified execution mode flags
 */
#define PSR_JF  0x01000000 /* Jazelle flag (BXJ ARMv5) */
#define PSR_EF  0x00000200 /* mixed-endian flag (SETEND ARMv6) */
#define PSR_AF  0x00000100 /* precise abort flag (ARMv6) */
#define PSR_TF  0x00000020 /* thumb flag (BX ARMv4T) */
#define PSR_TFb 5          /* thumb flag (BX ARMv4T) */

/*
 * Interrupts
 */
#define PSR_IRQFb 7          /* IRQ : 0 = IRQ enable */
#define PSR_IRQF  0x00000080 /* IRQ : 0 = IRQ enable */
#define PSR_FIQF  0x00000040 /* FIQ : 0 = FIQ enable */

/*
 * CPU mode
 */
#define PSR_USER_MODE 0x00000010 /* User mode */
#define PSR_FIQ_MODE  0x00000011 /* FIQ mode */
#define PSR_IRQ_MODE  0x00000012 /* IRQ mode */
#define PSR_SVC_MODE  0x00000013 /* Supervisor mode */
#define PSR_ABT_MODE  0x00000017 /* Abort mode */
#define PSR_UND_MODE  0x0000001B /* Undefined mode */

#define PSR_MODE_MASK      0x0000001F
#define PSR_IS_KERNEL(psr) (((psr) & PSR_MODE_MASK) != PSR_USER_MODE)
#define PSR_IS_USER(psr)   (((psr) & PSR_MODE_MASK) == PSR_USER_MODE)

#define PSR_USERDFLT  PSR_USER_MODE
#define PSR_USER_MASK (PSR_AF | PSR_IRQF | PSR_FIQF | PSR_MODE_MASK)
#define PSR_USER_SET  PSR_USER_MODE

#define PSR_INTMASK PSR_IRQF /* Interrupt disable */

/*
 * FPEXC: Floating-Point Exception Register
 */

#define FPEXC_EX     0x80000000 /* Exception status */
#define FPEXC_EX_BIT 31
#define FPEXC_EN     0x40000000 /* VFP : 1 = EN enable */
#define FPEXC_EN_BIT 30


/*
 * FPSCR: Floating-point Status and Control Register
 */

#define FPSCR_DN      0x02000000          /* Default NaN */
#define FPSCR_FZ      0x01000000          /* Flush to zero */

#define FPSCR_DEFAULT FPSCR_DN | FPSCR_FZ


/*
 * FSR registers
 *
 * IFSR: Instruction Fault Status Register
 * DFSR: Data Fault Status Register
 */
#define FSR_ALIGN      0x00000001 /* Alignment */
#define FSR_DEBUG      0x00000002 /* Debug (watch/break) */
#define FSR_ICFAULT    0x00000004 /* Fault on instruction cache maintenance */
#define FSR_SFAULT     0x00000005 /* Translation Section */
#define FSR_PFAULT     0x00000007 /* Translation Page */
#define FSR_SACCESS    0x00000003 /* Section access */
#define FSR_PACCESS    0x00000006 /* Page Access */
#define FSR_SDOM       0x00000009 /* Domain Section */
#define FSR_PDOM       0x0000000B /* Domain Page */
#define FSR_SPERM      0x0000000D /* Permission Section */
#define FSR_PPERM      0x0000000F /* Permission Page */
#define FSR_EXT        0x00001000 /* External (Implementation Defined Classification) */

#define FSR_MASK       0x0000040F /* Valid bits */
#define FSR_ALIGN_MASK 0x0000040D /* Valid bits to check align */

#define DFSR_WRITE     0x00000800 /* write data abort fault */

#if defined (ARMA7) || defined (APPLE_ARM64_ARCH_FAMILY) || defined (BCM2837)

#define TEST_FSR_VMFAULT(status) \
	(((status) == FSR_PFAULT)     \
	|| ((status) == FSR_PPERM)    \
	|| ((status) == FSR_SFAULT)   \
	|| ((status) == FSR_SPERM)    \
	|| ((status) == FSR_ICFAULT)  \
	|| ((status) == FSR_SACCESS)  \
	|| ((status) == FSR_PACCESS))

#define TEST_FSR_TRANSLATION_FAULT(status)      \
	                        (((status) == FSR_SFAULT)       \
	                        || ((status) == FSR_PFAULT))

#else

#error Incompatible CPU type configured

#endif

/*
 * Cache configuration
 */

#if defined (ARMA7)

/* I-Cache */
#define MMU_I_CLINE     5                      /* cache line size as 1<<MMU_I_CLINE (32) */

/* D-Cache */
#define MMU_CSIZE       15                     /* cache size as 1<<MMU_CSIZE (32K) */
#define MMU_CLINE       6                      /* cache line size as 1<<MMU_CLINE (64) */
#define MMU_NWAY        2                      /* set associativity 1<<MMU_NWAY (4) */
#define MMU_I7SET       6                      /* cp15 c7 set incrementer 1<<MMU_I7SET */
#define MMU_I7WAY       30                     /* cp15 c7 way incrementer 1<<MMU_I7WAY */

#define MMU_SWAY        (MMU_CSIZE - MMU_NWAY) /* set size 1<<MMU_SWAY */
#define MMU_NSET        (MMU_SWAY - MMU_CLINE) /* lines per way 1<<MMU_NSET */

#define __ARM_L2CACHE__ 1

#define L2_CSIZE        __ARM_L2CACHE_SIZE_LOG__ /* cache size as 1<<MMU_CSIZE */
#define L2_CLINE        6                        /* cache line size as 1<<MMU_CLINE (64) */
#define L2_NWAY         3                        /* set associativity 1<<MMU_NWAY (8) */
#define L2_I7SET        6                        /* cp15 c7 set incrementer 1<<MMU_I7SET */
#define L2_I7WAY        29                       /* cp15 c7 way incrementer 1<<MMU_I7WAY */
#define L2_I9WAY        29                       /* cp15 c9 way incrementer 1<<MMU_I9WAY */

#define L2_SWAY         (L2_CSIZE - L2_NWAY)     /* set size 1<<MMU_SWAY */
#define L2_NSET         (L2_SWAY - L2_CLINE)     /* lines per way 1<<MMU_NSET */

#elif defined (APPLETYPHOON)

/* I-Cache */
#define MMU_I_CLINE 6                      /* cache line size as 1<<MMU_I_CLINE (64) */

/* D-Cache */
#define MMU_CSIZE   16                     /* cache size as 1<<MMU_CSIZE (64K) */
#define MMU_CLINE   6                      /* cache line size as 1<<MMU_CLINE (64) */
#define MMU_NWAY    1                      /* set associativity 1<<MMU_NWAY (2) */
#define MMU_I7SET   6                      /* cp15 c7 set incrementer 1<<MMU_I7SET */
#define MMU_I7WAY   31                     /* cp15 c7 way incrementer 1<<MMU_I7WAY */
#define MMU_I9WAY   31                     /* cp15 c9 way incrementer 1<<MMU_I9WAY */

#define MMU_SWAY    (MMU_CSIZE - MMU_NWAY) /* set size 1<<MMU_SWAY */
#define MMU_NSET    (MMU_SWAY - MMU_CLINE) /* lines per way 1<<MMU_NSET */

#define __ARM_L2CACHE__ 1

#define L2_CSIZE        __ARM_L2CACHE_SIZE_LOG__ /* cache size as 1<<L2_CSIZE */
#define L2_CLINE        6                        /* cache line size as 1<<L2_CLINE (64) */
#define L2_NWAY         3                        /* set associativity 1<<L2_NWAY (8) */
#define L2_I7SET        6                        /* cp15 c7 set incrementer 1<<L2_I7SET */
#define L2_I7WAY        29                       /* cp15 c7 way incrementer 1<<L2_I7WAY */
#define L2_I9WAY        29                       /* cp15 c9 way incrementer 1<<L2_I9WAY */

#define L2_SWAY         (L2_CSIZE - L2_NWAY)     /* set size 1<<L2_SWAY */
#define L2_NSET         (L2_SWAY - L2_CLINE)     /* lines per way 1<<L2_NSET */

#elif defined (APPLETWISTER)

/* I-Cache */
#define MMU_I_CLINE 6                      /* cache line size as 1<<MMU_I_CLINE (64) */

/* D-Cache */
#define MMU_CSIZE   16                     /* cache size as 1<<MMU_CSIZE (64K) */
#define MMU_CLINE   6                      /* cache line size is 1<<MMU_CLINE (64) */
#define MMU_NWAY    2                      /* set associativity 1<<MMU_NWAY (4) */
#define MMU_I7SET   6                      /* cp15 c7 set incrementer 1<<MMU_I7SET */
#define MMU_I7WAY   30                     /* cp15 c7 way incrementer 1<<MMU_I7WAY */
#define MMU_I9WAY   30                     /* cp15 c9 way incrementer 1<<MMU_I9WAY */

#define MMU_SWAY    (MMU_CSIZE - MMU_NWAY) /* set size 1<<MMU_SWAY */
#define MMU_NSET    (MMU_SWAY - MMU_CLINE) /* lines per way 1<<MMU_NSET */

/* L2-Cache */
#define __ARM_L2CACHE__ 1

/*
 * For reasons discussed in the platform expert code, we round the reported
 * L2 size to 4MB, and adjust the other parameters accordingly.
 */
#define L2_CSIZE        __ARM_L2CACHE_SIZE_LOG__ /* cache size as 1<<L2_CSIZE */
#define L2_CLINE        6                        /* cache line size as 1<<L2_CSIZE (64) */
#define L2_NWAY         4                        /* set associativity as 1<<L2_CLINE (16, is actually 12) */
#define L2_I7SET        6                        /* cp15 c7 set incrementer 1<<L2_I7SET */
#define L2_I7WAY        28                       /* cp15 c7 way incrementer 1<<L2_I7WAY */
#define L2_I9WAY        28                       /* cp15 c9 way incremenber 1<<L2_I9WAY */

#define L2_SWAY         (L2_CSIZE - L2_NWAY)     /* set size 1<<L2_SWAY */
#define L2_NSET         (L2_SWAY - L2_CLINE)     /* lines per way 1<<L2_NSET */

#elif defined (APPLEHURRICANE)

/* I-Cache */
#define MMU_I_CLINE 6                      /* cache line size as 1<<MMU_I_CLINE (64) */

/* D-Cache */
#define MMU_CSIZE   16                     /* cache size as 1<<MMU_CSIZE (64K) */
#define MMU_CLINE   6                      /* cache line size is 1<<MMU_CLINE (64) */
#define MMU_NWAY    2                      /* set associativity 1<<MMU_NWAY (4) */
#define MMU_I7SET   6                      /* cp15 c7 set incrementer 1<<MMU_I7SET */
#define MMU_I7WAY   30                     /* cp15 c7 way incrementer 1<<MMU_I7WAY */
#define MMU_I9WAY   30                     /* cp15 c9 way incrementer 1<<MMU_I9WAY */

#define MMU_SWAY    (MMU_CSIZE - MMU_NWAY) /* set size 1<<MMU_SWAY */
#define MMU_NSET    (MMU_SWAY - MMU_CLINE) /* lines per way 1<<MMU_NSET */

/* L2-Cache */
#define __ARM_L2CACHE__ 1

/*
 * For reasons discussed in the platform expert code, we round the reported
 * L2 size to 4MB, and adjust the other parameters accordingly.
 */
#define L2_CSIZE        __ARM_L2CACHE_SIZE_LOG__ /* cache size as 1<<L2_CSIZE */
#define L2_CLINE        6                        /* cache line size as 1<<L2_CSIZE (64) */
#define L2_NWAY         4                        /* set associativity as 1<<L2_CLINE (16, is actually 12) */
#define L2_I7SET        6                        /* cp15 c7 set incrementer 1<<L2_I7SET */
#define L2_I7WAY        28                       /* cp15 c7 way incrementer 1<<L2_I7WAY */
#define L2_I9WAY        28                       /* cp15 c9 way incremenber 1<<L2_I9WAY */

#define L2_SWAY         (L2_CSIZE - L2_NWAY)     /* set size 1<<L2_SWAY */
#define L2_NSET         (L2_SWAY - L2_CLINE)     /* lines per way 1<<L2_NSET */

#elif defined (APPLEMONSOON)

/* I-Cache, 96KB for Monsoon, 48KB for Mistral, 6-way. */
#define MMU_I_CLINE 6                      /* cache line size as 1<<MMU_I_CLINE (64) */

/* D-Cache, 64KB for Monsoon, 32KB for Mistral, 4-way. */
#define MMU_CSIZE   16                     /* cache size as 1<<MMU_CSIZE (64K) */
#define MMU_CLINE   6                      /* cache line size is 1<<MMU_CLINE (64) */
#define MMU_NWAY    2                      /* set associativity 1<<MMU_NWAY (4) */
#define MMU_I7SET   6                      /* cp15 c7 set incrementer 1<<MMU_I7SET */
#define MMU_I7WAY   30                     /* cp15 c7 way incrementer 1<<MMU_I7WAY */
#define MMU_I9WAY   30                     /* cp15 c9 way incrementer 1<<MMU_I9WAY */

#define MMU_SWAY    (MMU_CSIZE - MMU_NWAY) /* set size 1<<MMU_SWAY */
#define MMU_NSET    (MMU_SWAY - MMU_CLINE) /* lines per way 1<<MMU_NSET */

/* L2-Cache */
#define __ARM_L2CACHE__ 1

/*
 * LLC (Monsoon L2, Mistral L3): 8MB, 128-byte lines, 16-way.
 * L2E (Mistral L2): 1MB, 64-byte lines, 8-way.
 *
 * TODO: Our L2 cahes have different line sizes.  I begin to suspect
 * this may be a problem.
 */
#define L2_CSIZE        __ARM_L2CACHE_SIZE_LOG__ /* cache size as 1<<L2_CSIZE */
#define L2_CLINE        7                        /* cache line size as 1<<L2_CLINE (128) */
#define L2_NWAY         4                        /* set associativity as 1<<L2_NWAY (16) */
#define L2_I7SET        6                        /* TODO: cp15 c7 set incrementer 1<<L2_I7SET */
#define L2_I7WAY        28                       /* TODO: cp15 c7 way incrementer 1<<L2_I7WAY */
#define L2_I9WAY        28                       /* TODO: cp15 c9 way incremenber 1<<L2_I9WAY */

#define L2_SWAY         (L2_CSIZE - L2_NWAY)     /* set size 1<<L2_SWAY */
#define L2_NSET         (L2_SWAY - L2_CLINE)     /* lines per way 1<<L2_NSET */

#elif defined (BCM2837) /* Raspberry Pi 3 */

/* I-Cache. We don't have detailed spec so we just follow the ARM technical reference. */
#define MMU_I_CLINE 6

/* D-Cache. */
#define MMU_CSIZE   15
#define MMU_CLINE   6
#define MMU_NWAY    2

#define MMU_I7SET   6
#define MMU_I7WAY   30
#define MMU_I9WAY   30

#define MMU_SWAY    (MMU_CSIZE - MMU_NWAY)
#define MMU_NSET    (MMU_SWAY - MMU_CLINE)

#define __ARM_L2CACHE__ 1

#define L2_CSIZE        __ARM_L2CACHE_SIZE_LOG__
#define L2_CLINE        6
#define L2_NWAY         4
#define L2_I7SET        6
#define L2_I7WAY        28
#define L2_I9WAY        28
#define L2_SWAY         (L2_CSIZE - L2_NWAY)
#define L2_NSET         (L2_SWAY - L2_CLINE)

#else
#error processor not supported
#endif


#if (__ARM_VMSA__ <= 7)

/*
 *  SCTLR: System Control Register
 */
/*
 * System Control Register (SCTLR)
 *
 *  31 30 29  28   27    25 24 22 21   20  19   17  15 14 13 12 11 10           5    2 1 0
 * +-+--+---+---+----+-+--+--+--+--+----+---+-+--+-+-+--+--+--+--+--+---+-+------+--+-+-+-+
 * |0|TE|AFE|TRE|NMFI|0|EE|VE|11|FI|UWXN|WXN|1|HA|1|0|RR| V| I| Z|SW|000|1|C15BEN|11|C|A|M|
 * +-+--+---+---+----+-+--+--+--+--+----+---+-+--+-+-+--+--+--+--+--+---+-+------+--+-+-+-+
 *
 * Where:
 *   TE:     Thumb Exception enable
 *   AFE:    Access flag enable
 *   TRE:    TEX remap enable
 *   NMFI:   Non-maskable FIQ (NMFI) support
 *   EE:     Exception Endianness
 *   VE:     Interrupt Vectors Enable
 *   FI:     Fast interrupts configuration enable
 *   ITD:    IT Disable
 *   UWXN:   Unprivileged write permission implies PL1 XN
 *   WXN:    Write permission implies XN
 *   HA:     Hardware Access flag enable
 *   RR:     Round Robin select
 *   V:      High exception vectors
 *   I:      Instruction cache enable
 *   Z:      Branch prediction enable
 *   SW:     SWP/SWPB enable
 *   C15BEN: CP15 barrier enable
 *   C:      Cache enable
 *   A:      Alignment check enable
 *   M:      MMU enable
 */

#define SCTLR_RESERVED 0x82DD8394

#define SCTLR_ENABLE   0x00000001 /* MMU enable */
#define SCTLR_ALIGN    0x00000002 /* Alignment check enable */
#define SCTLR_DCACHE   0x00000004 /* Data or Unified Cache enable */
#define SCTLR_BEN      0x00000040 /* CP15 barrier enable */
#define SCTLR_SW       0x00000400 /* SWP/SWPB Enable */
#define SCTLR_PREDIC   0x00000800 /* Branch prediction enable */
#define SCTLR_ICACHE   0x00001000 /* Instruction cache enabled. */
#define SCTLR_HIGHVEC  0x00002000 /* Vector table at 0xffff0000 */
#define SCTLR_RROBIN   0x00004000 /* Round Robin replacement */
#define SCTLR_HA       0x00020000 /* Hardware Access flag enable */
#define SCTLR_NMFI     0x08000000 /* Non-maskable FIQ */
#define SCTLR_TRE      0x10000000 /* TEX remap enable */
#define SCTLR_AFE      0x20000000 /* Access flag enable */
#define SCTLR_TE       0x40000000 /* Thumb Exception enable */

#define SCTLR_DEFAULT \
	(SCTLR_AFE|SCTLR_TRE|SCTLR_HIGHVEC|SCTLR_ICACHE|SCTLR_PREDIC|SCTLR_DCACHE|SCTLR_ENABLE)


/*
 *  PRRR: Primary Region Remap Register
 *
 *  31            24       20  19  18  17  16                     0
 * +---------------------------------------------------------------+
 * |      NOSn      |  Res   |NS1|NS0|DS1|DS0|       TRn           |
 * +---------------------------------------------------------------+
 */

#define PRRR_NS1 0x00080000
#define PRRR_NS0 0x00040000
#define PRRR_DS1 0x00020000
#define PRRR_DS0 0x00010000

#define PRRR_NOSn_ISH(region) (0x1<<((region)+24))

#if defined (ARMA7)
#define PRRR_SETUP (0x1F08022A)
#else
#error processor not supported
#endif

/*
 *  NMRR, Normal Memory Remap Register
 *
 *   30  28  26  24  22  20  18  16  14  12  10   8   6   4   2   0
 * +---------------------------------------------------------------+
 * |OR7|OR6|OR5|OR4|OR3|OR2|OR1|OR0|IR7|IR6|IR5|IR4|IR3|IR2|IR1|IR0|
 * +---------------------------------------------------------------+
 */

#define NMRR_DISABLED    0x0 /* Non-cacheable */
#define NMRR_WRITEBACK   0x1 /* Write-Back, Write-Allocate */
#define NMRR_WRITETHRU   0x2 /* Write-Through, no Write-Allocate */
#define NMRR_WRITEBACKNO 0x3 /* Write-Back, no Write-Allocate */

#if defined (ARMA7)
#define NMRR_SETUP (0x01210121)
#else
#error processor not supported
#endif

/*
 * TTBR: Translation Table Base Register
 *
 */

#define TTBR_IRGN_DISBALED    0x00000000 /* inner non-cacheable */
#define TTBR_IRGN_WRITEBACK   0x00000040 /* inner write back and allocate */
#define TTBR_IRGN_WRITETHRU   0x00000001 /* inner write thru */
#define TTBR_IRGN_WRITEBACKNO 0x00000041 /* inner write back no allocate */

#define TTBR_RGN_DISBALED     0x00000000 /* outer non-cacheable */
#define TTBR_RGN_WRITEBACK    0x00000008 /* outer write back and allocate */
#define TTBR_RGN_WRITETHRU    0x00000010 /* outer write thru outer cache */
#define TTBR_RGN_WRITEBACKNO  0x00000018 /* outer write back no allocate */

#define TTBR_SHARED           0x00000002 /* Shareable memory atribute */
#define TTBR_SHARED_NOTOUTER  0x00000020 /* Outer not shareable memory atribute */

#if defined (ARMA7)
#define TTBR_SETUP (TTBR_RGN_WRITEBACK|TTBR_IRGN_WRITEBACK|TTBR_SHARED)
#else
#error processor not supported
#endif

/*
 * TTBCR: Translation Table Base Control register
 *
 *  31   3 2 0
 * +----------+
 * | zero | N |
 * +----------+
 *
 * If N=0, always use translation table base register 0.  Otherwise, if
 * bits [31:32-N] of the address are all zero use base register 0.  Otherwise,
 * use base register 1.
 *
 * Reading from this register also returns the page table boundary for TTB0.
 * Writing to it updates the boundary for TTB0. (0=16KB, 1=8KB, 2=4KB, etc...)
 */

#define TTBCR_N_1GB_TTB0 0x2 /* 1 GB TTB0, 3GB TTB1 */
#define TTBCR_N_2GB_TTB0 0x1 /* 2 GB TTB0, 2GB TTB1 */
#define TTBCR_N_4GB_TTB0 0x0 /* 4 GB TTB0 */
#define TTBCR_N_MASK     0x3



/*
 * ARM Page Granule
 */
#define ARM_PGSHIFT 12
#define ARM_PGBYTES (1 << ARM_PGSHIFT)
#define ARM_PGMASK  (ARM_PGBYTES-1)

/*
 * DACR: Domain Access Control register
 */

#define DAC_FAULT   0x0 /* invalid domain - everyone loses */
#define DAC_CLIENT  0x1 /* client domain - use AP bits */
#define DAC_RESERVE 0x2 /* reserved domain - undefined */
#define DAC_MANAGER 0x3 /* manager domain - all access */

#define DACR_SET(dom, x) ((x)<<((dom)<<1))


#define ARM_DOM_DEFAULT 0   /* domain that forces AP use */
#define ARM_DAC_SETUP   0x1

/*
 * ARM 2-level Page Table support
 */

/*
 *  Memory Attribute Index
 */
#define CACHE_ATTRINDX_WRITEBACK                 0x0 /* cache enabled, buffer enabled */
#define CACHE_ATTRINDX_WRITECOMB                 0x1 /* no cache, buffered writes */
#define CACHE_ATTRINDX_WRITETHRU                 0x2 /* cache enabled, buffer disabled */
#define CACHE_ATTRINDX_DISABLE                   0x3 /* no cache, no buffer */
#define CACHE_ATTRINDX_INNERWRITEBACK            0x4 /* inner cache enabled, buffer enabled, write allocate */
#define CACHE_ATTRINDX_POSTED                    CACHE_ATTRINDX_DISABLE
#define CACHE_ATTRINDX_POSTED_REORDERED          CACHE_ATTRINDX_DISABLE
#define CACHE_ATTRINDX_POSTED_COMBINED_REORDERED CACHE_ATTRINDX_DISABLE
#define CACHE_ATTRINDX_DEFAULT                   CACHE_ATTRINDX_WRITEBACK


/*
 * Access protection bit values
 */
#define AP_RWNA 0x0 /* priv=read-write, user=no-access  */
#define AP_RWRW 0x1 /* priv=read-write, user=read-write */
#define AP_RONA 0x2 /* priv=read-only , user=no-access  */
#define AP_RORO 0x3 /* priv=read-only , user=read-only  */

/*
 *  L1 Translation table
 *
 *  Each translation table is up to 16KB
 *  4096 32-bit entries of 1MB of address space.
 */

#define ARM_TT_L1_SIZE          0x00100000              /* size of area covered by a tte */
#define ARM_TT_L1_OFFMASK       0x000FFFFF              /* offset within an L1 entry */
#define ARM_TT_L1_TABLE_OFFMASK 0x000FFFFF              /* offset within an L1 entry */
#define ARM_TT_L1_BLOCK_OFFMASK 0x000FFFFF              /* offset within an L1 entry */
#define ARM_TT_L1_SUPER_OFFMASK 0x00FFFFFF              /* offset within an L1 entry */
#define ARM_TT_L1_SHIFT         20                      /* page descriptor shift */
#define ARM_TT_L1_INDEX_MASK    0xfff00000              /* mask for getting index in L1 table from virtual address */

#define ARM_TT_L1_PT_SIZE       (4 * ARM_TT_L1_SIZE)    /* 4 L1 table entries required to consume 1 L2 pagetable page */
#define ARM_TT_L1_PT_OFFMASK    (ARM_TT_L1_PT_SIZE - 1)

/*
 *  L2 Translation table
 *
 *  Each translation table is up to 1KB
 *  4096 32-bit entries of 1MB (2^30) of address space.
 */

#define ARM_TT_L2_SIZE       0x00001000 /* size of area covered by a tte */
#define ARM_TT_L2_OFFMASK    0x00000FFF /* offset within an L2 entry */
#define ARM_TT_L2_SHIFT      12         /* page descriptor shift */
#define ARM_TT_L2_INDEX_MASK 0x000ff000 /* mask for getting index in L2 table from virtual address */

/*
 * Convenience definitions for:
 *   ARM_TT_LEAF: The last level of the configured page table format.
 *   ARM_TT_TWIG: The second to last level of the configured page table format.
 *   ARM_TT_ROOT: The first level of the configured page table format.
 *
 *   My apologies to any botanists who may be reading this.
 */
#define ARM_TT_LEAF_SIZE       ARM_TT_L2_SIZE
#define ARM_TT_LEAF_OFFMASK    ARM_TT_L2_OFFMASK
#define ARM_TT_LEAF_SHIFT      ARM_TT_L2_SHIFT
#define ARM_TT_LEAF_INDEX_MASK ARM_TT_L2_INDEX_MASK

#define ARM_TT_TWIG_SIZE       ARM_TT_L1_SIZE
#define ARM_TT_TWIG_OFFMASK    ARM_TT_L1_OFFMASK
#define ARM_TT_TWIG_SHIFT      ARM_TT_L1_SHIFT
#define ARM_TT_TWIG_INDEX_MASK ARM_TT_L1_INDEX_MASK

#define ARM_TT_ROOT_SIZE       ARM_TT_L1_SIZE
#define ARM_TT_ROOT_OFFMASK    ARM_TT_L1_OFFMASK
#define ARM_TT_ROOT_SHIFT      ARM_TT_L1_SHIFT
#define ARM_TT_ROOT_INDEX_MASK ARM_TT_L1_INDEX_MASK

/*
 * Level 1 Translation Table Entry
 *
 * page table entry
 *
 *  31                  10 9 8  5  4  2  0
 * +----------------------+-+----+--+--+--+
 * | page table base addr | |dom |XN|00|01|
 * +----------------------+-+----+--+--+--+
 *
 * direct (1MB) section entry
 *
 *  31        20 18    15  12 10 9 8  5  4  2  0
 * +------------+--+-+-+-+---+--+-+----+--+--+--+
 * | base addr  |00|G|S|A|TEX|AP| |dom |XN|CB|10|
 * +------------+--+-+-+-+---+--+-+----+--+--+--+
 *
 *  super (16MB) section entry
 *
 *  31     24 23  18    15  12 10 9 8  5  4  2  0
 * +---------+------+-+-+-+---+--+-+----+--+--+--+
 * |base addr|000001|G|S|A|TEX|AP| |dom |XN|CB|10|
 * +---------+------+-+-+-+---+--+-+----+--+--+--+
 *
 * where:
 *   'G' is the notGlobal bit
 *   'S' is the shared bit
 *   'A' in the access permission extension (APX) bit
 *   'TEX' remap register control bits
 *   'AP' is the access protection
 *   'dom' is the domain for the translation
 *   'XN' is the eXecute Never bit
 *   'CB' is the cache/buffer attribute
 */

#define ARM_TTE_EMPTY            0x00000000                     /* unasigned entry */

#define ARM_TTE_TYPE_FAULT       0x00000000                     /* fault entry type */
#define ARM_TTE_TYPE_TABLE       0x00000001                     /* page table type */
#define ARM_TTE_TYPE_BLOCK       0x00000002                     /* section entry type */
#define ARM_TTE_TYPE_MASK        0x00000003                     /* mask for extracting the type */

#define ARM_TTE_BLOCK_NGSHIFT    17
#define ARM_TTE_BLOCK_NG_MASK    0x00020000                     /* mask to determine notGlobal bit */
#define ARM_TTE_BLOCK_NG         0x00020000                     /* value for a per-process mapping */

#define ARM_TTE_BLOCK_SHSHIFT    16
#define ARM_TTE_BLOCK_SH_MASK    0x00010000                     /* shared (SMP) mapping mask */
#define ARM_TTE_BLOCK_SH         0x00010000                     /* shared (SMP) mapping */

#define ARM_TTE_BLOCK_CBSHIFT    2
#define ARM_TTE_BLOCK_CB(x)      ((x) << ARM_TTE_BLOCK_CBSHIFT)
#define ARM_TTE_BLOCK_CB_MASK    (3<< ARM_TTE_BLOCK_CBSHIFT)

#define ARM_TTE_BLOCK_AP0SHIFT   10
#define ARM_TTE_BLOCK_AP0        (1<<ARM_TTE_BLOCK_AP0SHIFT)
#define ARM_TTE_BLOCK_AP0_MASK   (1<<ARM_TTE_BLOCK_AP0SHIFT)

#define ARM_TTE_BLOCK_AP1SHIFT   11
#define ARM_TTE_BLOCK_AP1        (1<<ARM_TTE_BLOCK_AP1SHIFT)
#define ARM_TTE_BLOCK_AP1_MASK   (1<<ARM_TTE_BLOCK_AP1SHIFT)

#define ARM_TTE_BLOCK_AP2SHIFT   15
#define ARM_TTE_BLOCK_AP2        (1<<ARM_TTE_BLOCK_AP2SHIFT)
#define ARM_TTE_BLOCK_AP2_MASK   (1<<ARM_TTE_BLOCK_AP2SHIFT)

/* access protections */
#define ARM_TTE_BLOCK_AP(ap) \
	((((ap)&0x1)<<ARM_TTE_BLOCK_AP1SHIFT) |    \
	((((ap)>>1)&0x1)<<ARM_TTE_BLOCK_AP2SHIFT))

/* mask access protections */
#define ARM_TTE_BLOCK_APMASK \
	(ARM_TTE_BLOCK_AP1_MASK | ARM_TTE_BLOCK_AP2_MASK)

#define ARM_TTE_BLOCK_AF         ARM_TTE_BLOCK_AP0             /* value for access */
#define ARM_TTE_BLOCK_AFMASK     ARM_TTE_BLOCK_AP0_MASK        /* access mask */

#define ARM_TTE_TABLE_MASK       0xFFFFFC00                    /* mask for a L2 page table entry */
#define ARM_TTE_TABLE_SHIFT      10                            /* shift for  L2 page table phys address */

#define ARM_TTE_BLOCK_L1_MASK    0xFFF00000                    /* mask to extract phys address from L1 section entry */
#define ARM_TTE_BLOCK_L1_SHIFT   20                            /* shift for 1MB section phys address */

#define ARM_TTE_SUPER_L1_MASK    0xFF000000                    /* mask to extract phys address from L1 super entry */
#define ARM_TTE_SUPER_L1_SHIFT   24                            /* shift for 16MB section phys address */

#define ARM_TTE_BLOCK_SUPER      0x00040000                    /* make section a 16MB section */
#define ARM_TTE_BLOCK_SUPER_MASK 0x00F40000                    /* make section a 16MB section */

#define ARM_TTE_BLOCK_NXSHIFT    4
#define ARM_TTE_BLOCK_NX         0x00000010                    /* section is no execute */
#define ARM_TTE_BLOCK_NX_MASK    0x00000010                    /* mask for extracting no execute bit */
#define ARM_TTE_BLOCK_PNX        ARM_TTE_BLOCK_NX

#define ARM_TTE_BLOCK_TEX0SHIFT  12
#define ARM_TTE_BLOCK_TEX0       (1<<ARM_TTE_BLOCK_TEX0SHIFT)
#define ARM_TTE_BLOCK_TEX0_MASK  (1<<ARM_TTE_BLOCK_TEX0SHIFT)

#define ARM_TTE_BLOCK_TEX1SHIFT  13
#define ARM_TTE_BLOCK_TEX1       (1<<ARM_TTE_BLOCK_TEX1SHIFT)
#define ARM_TTE_BLOCK_TEX1_MASK  (1<<ARM_TTE_BLOCK_TEX1SHIFT)

#define ARM_TTE_BLOCK_TEX2SHIFT  14
#define ARM_TTE_BLOCK_TEX2       (1<<ARM_TTE_BLOCK_TEX2SHIFT)
#define ARM_TTE_BLOCK_TEX2_MASK  (1<<ARM_TTE_BLOCK_TEX2SHIFT)


/* mask memory attributes index */
#define ARM_TTE_BLOCK_ATTRINDX(i) \
	((((i)&0x3)<<ARM_TTE_BLOCK_CBSHIFT) |      \
	((((i)>>2)&0x1)<<ARM_TTE_BLOCK_TEX0SHIFT))

/* mask memory attributes index */
#define ARM_TTE_BLOCK_ATTRINDXMASK \
	(ARM_TTE_BLOCK_CB_MASK | ARM_TTE_BLOCK_TEX0_MASK)


/*
 * Level 2 Page table entries
 *
 * The following page table entry types are possible:
 *
 * fault page entry
 *  31                                     2  0
 * +----------------------------------------+--+
 * |    ignored                             |00|
 * +----------------------------------------+--+
 *
 * large (64KB) page entry
 *  31            16 15  12     9   6  4 3 2  0
 * +----------------+--+---+-+-+-+---+--+-+-+--+
 * | base phys addr |XN|TEX|G|S|A|000|AP|C|B|01|
 * +----------------+--+---+-+-+-+---+--+-+-+--+
 *
 * small (4KB) page entry
 *  31                   12     9   6  4 3 2 1  0
 * +-----------------------+-+-+-+---+--+-+-+-+--+
 * | base phys addr        |G|S|A|TEX|AP|C|B|1|XN|
 * +-----------------------+-+-+-+---+--+-+-+-+--+
 *
 * also where:
 *   'XN' is the eXecute Never bit
 *   'G' is the notGlobal (process-specific) bit
 *   'S' is the shared bit
 *   'A' in the access permission extension (ATX) bit
 *   'TEX' remap register control bits
 *   'AP' is the access protection
 *   'dom' is the domain for the translation
 *   'C' is the cache attribute
 *   'B' is the write buffer attribute
 */

/* markers for (invalid) PTE for a page sent to compressor */
#define ARM_PTE_COMPRESSED      ARM_PTE_TEX1                                  /* compressed... */
#define ARM_PTE_COMPRESSED_ALT  ARM_PTE_TEX2                                  /* ... and was "alt_acct" */
#define ARM_PTE_COMPRESSED_MASK (ARM_PTE_COMPRESSED | ARM_PTE_COMPRESSED_ALT)
#define ARM_PTE_IS_COMPRESSED(x, p) \
	((((x) & 0x3) == 0) &&                   /* PTE is not valid... */         \
	 ((x) & ARM_PTE_COMPRESSED) &&           /* ...has "compressed" marker" */ \
	 ((!((x) & ~ARM_PTE_COMPRESSED_MASK)) || /* ...no other bits */            \
	  (panic("compressed PTE %p 0x%x has extra bits 0x%x: corrupted?",         \
	         (p), (x), (x) & ~ARM_PTE_COMPRESSED_MASK), FALSE)))

#define PTE_SHIFT              2                      /* shift width of a pte (sizeof(pte) == (1 << PTE_SHIFT)) */
#define PTE_PGENTRIES          (1024 >> PTE_SHIFT)    /* number of ptes per page */

#define ARM_PTE_EMPTY          0x00000000             /* unasigned - invalid entry */

#define ARM_PTE_TYPE_FAULT     0x00000000             /* fault entry type */
#define ARM_PTE_TYPE_VALID     0x00000002             /* valid L2 entry */
#define ARM_PTE_TYPE           0x00000002             /* small page entry type */
#define ARM_PTE_TYPE_MASK      0x00000002             /* mask to get pte type */

#define ARM_PTE_NG_MASK        0x00000800             /* mask to determine notGlobal bit */
#define ARM_PTE_NG             0x00000800             /* value for a per-process mapping */

#define ARM_PTE_SHSHIFT        10
#define ARM_PTE_SHMASK         0x00000400             /* shared (SMP) mapping mask */
#define ARM_PTE_SH             0x00000400             /* shared (SMP) mapping */

#define ARM_PTE_CBSHIFT        2
#define ARM_PTE_CB(x)          ((x)<<ARM_PTE_CBSHIFT)
#define ARM_PTE_CB_MASK        (0x3<<ARM_PTE_CBSHIFT)

#define ARM_PTE_AP0SHIFT       4
#define ARM_PTE_AP0            (1<<ARM_PTE_AP0SHIFT)
#define ARM_PTE_AP0_MASK       (1<<ARM_PTE_AP0SHIFT)

#define ARM_PTE_AP1SHIFT       5
#define ARM_PTE_AP1            (1<<ARM_PTE_AP1SHIFT)
#define ARM_PTE_AP1_MASK       (1<<ARM_PTE_AP1SHIFT)

#define ARM_PTE_AP2SHIFT       9
#define ARM_PTE_AP2            (1<<ARM_PTE_AP2SHIFT)
#define ARM_PTE_AP2_MASK       (1<<ARM_PTE_AP2SHIFT)

/* access protections */
#define ARM_PTE_AP(ap) \
	((((ap)&0x1)<<ARM_PTE_AP1SHIFT) |    \
	((((ap)>>1)&0x1)<<ARM_PTE_AP2SHIFT))

/* mask access protections */
#define ARM_PTE_APMASK \
	(ARM_PTE_AP1_MASK | ARM_PTE_AP2_MASK)

#define ARM_PTE_AF             ARM_PTE_AP0            /* value for access */
#define ARM_PTE_AFMASK         ARM_PTE_AP0_MASK       /* access mask */

#define ARM_PTE_PAGE_MASK      0xFFFFF000             /* mask for a small page */
#define ARM_PTE_PAGE_SHIFT     12                     /* page shift for 4KB page */

#define ARM_PTE_NXSHIFT        0
#define ARM_PTE_NX             0x00000001             /* small page no execute */
#define ARM_PTE_NX_MASK        (1<<ARM_PTE_NXSHIFT)

#define ARM_PTE_PNXSHIFT       0
#define ARM_PTE_PNX            0x00000000             /* no privilege execute. not impl */
#define ARM_PTE_PNX_MASK       (0<<ARM_PTE_NXSHIFT)

#define ARM_PTE_TEX0SHIFT      6
#define ARM_PTE_TEX0           (1<<ARM_PTE_TEX0SHIFT)
#define ARM_PTE_TEX0_MASK      (1<<ARM_PTE_TEX0SHIFT)

#define ARM_PTE_TEX1SHIFT      7
#define ARM_PTE_TEX1           (1<<ARM_PTE_TEX1SHIFT)
#define ARM_PTE_TEX1_MASK      (1<<ARM_PTE_TEX1SHIFT)

#define ARM_PTE_WRITEABLESHIFT ARM_PTE_TEX1SHIFT
#define ARM_PTE_WRITEABLE      ARM_PTE_TEX1
#define ARM_PTE_WRITEABLE_MASK ARM_PTE_TEX1_MASK

#define ARM_PTE_TEX2SHIFT      8
#define ARM_PTE_TEX2           (1<<ARM_PTE_TEX2SHIFT)
#define ARM_PTE_TEX2_MASK      (1<<ARM_PTE_TEX2SHIFT)

#define ARM_PTE_WIREDSHIFT     ARM_PTE_TEX2SHIFT
#define ARM_PTE_WIRED          ARM_PTE_TEX2
#define ARM_PTE_WIRED_MASK     ARM_PTE_TEX2_MASK

/* mask memory attributes index */
#define ARM_PTE_ATTRINDX(indx) \
	((((indx)&0x3)<<ARM_PTE_CBSHIFT) |      \
	((((indx)>>2)&0x1)<<ARM_PTE_TEX0SHIFT))

/* mask memory attributes index */
#define ARM_PTE_ATTRINDXMASK \
	(ARM_PTE_CB_MASK | ARM_PTE_TEX0_MASK)

#define ARM_SMALL_PAGE_SIZE    (4096)         /* 4KB */
#define ARM_LARGE_PAGE_SIZE    (64*1024)      /* 64KB */
#define ARM_SECTION_SIZE       (1024*1024)    /* 1MB */
#define ARM_SUPERSECTION_SIZE  (16*1024*1024) /* 16MB */

#define TLBI_ADDR_SHIFT (12)
#define TLBI_ADDR_SIZE  (20)
#define TLBI_ADDR_MASK  (((1ULL << TLBI_ADDR_SIZE) - 1))
#define TLBI_ASID_SHIFT (0)
#define TLBI_ASID_SIZE  (8)
#define TLBI_ASID_MASK  (((1ULL << TLBI_ASID_SIZE) - 1))
#endif

/*
 * Format of the Debug Status and Control Register (DBGDSCR)
 */
#define ARM_DBGDSCR_RXFULL                 (1 << 30)
#define ARM_DBGDSCR_TXFULL                 (1 << 29)
#define ARM_DBGDSCR_RXFULL_1               (1 << 27)
#define ARM_DBGDSCR_TXFULL_1               (1 << 26)
#define ARM_DBGDSCR_PIPEADV                (1 << 25)
#define ARM_DBGDSCR_INSTRCOMPL_1           (1 << 24)
#define ARM_DBGDSCR_EXTDCCMODE_MASK        (3 << 20)
#define ARM_DBGDSCR_EXTDCCMODE_NONBLOCKING (0 << 20)
#define ARM_DBGDSCR_EXTDCCMODE_STALL       (1 << 20)
#define ARM_DBGDSCR_EXTDCCMODE_FAST        (1 << 20)
#define ARM_DBGDSCR_ADADISCARD             (1 << 19)
#define ARM_DBGDSCR_NS                     (1 << 18)
#define ARM_DBGDSCR_SPNIDDIS               (1 << 17)
#define ARM_DBGDSCR_SPIDDIS                (1 << 16)
#define ARM_DBGDSCR_MDBGEN                 (1 << 15)
#define ARM_DBGDSCR_HDBGEN                 (1 << 14)
#define ARM_DBGDSCR_ITREN                  (1 << 13)
#define ARM_DBGDSCR_UDCCDIS                (1 << 12)
#define ARM_DBGDSCR_INTDIS                 (1 << 11)
#define ARM_DBGDSCR_DBGACK                 (1 << 10)
#define ARM_DBGDSCR_DBGNOPWRDWN            (1 << 9)
#define ARM_DBGDSCR_UND_1                  (1 << 8)
#define ARM_DBGDSCR_ADABORT_1              (1 << 7)
#define ARM_DBGDSCR_SDABORT_1              (1 << 6)
#define ARM_DBGDSCR_MOE_MASK               (15 << 2)
#define ARM_DBGDSCR_MOE_HALT_REQUEST       (0 << 2)
#define ARM_DBGDSCR_MOE_BREAKPOINT         (1 << 2)
#define ARM_DBGDSCR_MOE_ASYNC_WATCHPOINT   (2 << 2)
#define ARM_DBGDSCR_MOE_BKPT_INSTRUCTION   (3 << 2)
#define ARM_DBGDSCR_MOE_EXT_DEBUG_REQ      (4 << 2)
#define ARM_DBGDSCR_MOE_VECTOR_CATCH       (5 << 2)
#define ARM_DBGDSCR_MOE_DSIDE_ABORT        (6 << 2)
#define ARM_DBGDSCR_MOE_ISIDE_ABORT        (7 << 2)
#define ARM_DBGDSCR_MOE_OS_UNLOCK_CATCH    (8 << 2)
#define ARM_DBGDSCR_MOE_SYNC_WATCHPOINT    (10 << 2)

#define ARM_DBGDSCR_RESTARTED              (1 << 1)
#define ARM_DBGDSCR_HALTED                 (1 << 0)

/*
 * Format of the Debug & Watchpoint Breakpoint Value and Control Registers
 * Using ARMv7 names; ARMv6 and ARMv6.1 are bit-compatible
 */
#define ARM_DBG_VR_ADDRESS_MASK             0xFFFFFFFC /* BVR & WVR */
#define ARM_DBGBVR_CONTEXTID_MASK           0xFFFFFFFF /* BVR only  */

#define ARM_DBG_CR_ADDRESS_MASK_MASK        0x1F000000 /* BCR & WCR */
#define ARM_DBGBCR_MATCH_MASK               (1 << 22)  /* BCR only  */
#define ARM_DBGBCR_MATCH_MATCH              (0 << 22)
#define ARM_DBGBCR_MATCH_MISMATCH           (1 << 22)
#define ARM_DBGBCR_TYPE_MASK                (1 << 21)  /* BCR only */
#define ARM_DBGBCR_TYPE_IVA                 (0 << 21)
#define ARM_DBGBCR_TYPE_CONTEXTID           (1 << 21)
#define ARM_DBG_CR_LINKED_MASK              (1 << 20)  /* BCR & WCR */
#define ARM_DBG_CR_LINKED_LINKED            (1 << 20)
#define ARM_DBG_CR_LINKED_UNLINKED          (0 << 20)
#define ARM_DBG_CR_LINKED_BRP_MASK          0x000F0000 /* BCR & WCR */
#define ARM_DBG_CR_SECURITY_STATE_MASK      (3 << 14)  /* BCR & WCR */
#define ARM_DBG_CR_SECURITY_STATE_BOTH      (0 << 14)
#define ARM_DBG_CR_SECURITY_STATE_NONSECURE (1 << 14)
#define ARM_DBG_CR_SECURITY_STATE_SECURE    (2 << 14)
#define ARM_DBG_CR_HIGHER_MODE_MASK         (1 << 13)  /* BCR & WCR */
#define ARM_DBG_CR_HIGHER_MODE_ENABLE       (1 << 13)
#define ARM_DBG_CR_HIGHER_MODE_DISABLE      (0 << 13)
#define ARM_DBGWCR_BYTE_ADDRESS_SELECT_MASK 0x00001FE0 /* WCR only  */
#define ARM_DBG_CR_BYTE_ADDRESS_SELECT_MASK 0x000001E0 /* BCR & WCR */
#define ARM_DBGWCR_ACCESS_CONTROL_MASK      (3 << 3)   /* WCR only */
#define ARM_DBCWCR_ACCESS_CONTROL_LOAD      (1 << 3)
#define ARM_DBCWCR_ACCESS_CONTROL_STORE     (2 << 3)
#define ARM_DBCWCR_ACCESS_CONTROL_ANY       (3 << 3)
#define ARM_DBG_CR_MODE_CONTROL_MASK        (3 << 1)   /* BCR & WCR */
#define ARM_DBG_CR_MODE_CONTROL_U_S_S       (0 << 1)   /* BCR only  */
#define ARM_DBG_CR_MODE_CONTROL_PRIVILEGED  (1 << 1)   /* BCR & WCR */
#define ARM_DBG_CR_MODE_CONTROL_USER        (2 << 1)   /* BCR & WCR */
#define ARM_DBG_CR_MODE_CONTROL_ANY         (3 << 1)   /* BCR & WCR */
#define ARM_DBG_CR_ENABLE_MASK              (1 << 0)   /* BCR & WCR */
#define ARM_DBG_CR_ENABLE_ENABLE            (1 << 0)
#define ARM_DBG_CR_ENABLE_DISABLE           (0 << 0)

/*
 * Format of the Device Power-down and Reset Status Register (DBGPRSR)
 */
#define ARM_DBGPRSR_STICKY_RESET_STATUS     (1 << 3)
#define ARM_DBGPRSR_RESET_STATUS            (1 << 2)
#define ARM_DBGPRSR_STICKY_POWERDOWN_STATUS (1 << 1)
#define ARM_DBGPRSR_POWERUP_STATUS          (1 << 0)

/*
 * Format of the OS Lock Access (DBGOSLAR) and Lock Access Registers (DBGLAR)
 */
#define ARM_DBG_LOCK_ACCESS_KEY 0xC5ACCE55

/* ARMv7 Debug register map */
#define ARM_DEBUG_OFFSET_DBGDIDR       (0x000)
#define ARM_DEBUG_OFFSET_DBGWFAR       (0x018)
#define ARM_DEBUG_OFFSET_DBGVCR        (0x01C)
#define ARM_DEBUG_OFFSET_DBGECR        (0x024)
#define ARM_DEBUG_OFFSET_DBGDSCCR      (0x028)
#define ARM_DEBUG_OFFSET_DBGDSMCR      (0x02C)
#define ARM_DEBUG_OFFSET_DBGDTRRX      (0x080)
#define ARM_DEBUG_OFFSET_DBGITR        (0x084) /* Write-only */
#define ARM_DEBUG_OFFSET_DBGPCSR       (0x084) /* Read-only */
#define ARM_DEBUG_OFFSET_DBGDSCR       (0x088)
#define ARM_DEBUG_OFFSET_DBGDTRTX      (0x08C)
#define ARM_DEBUG_OFFSET_DBGDRCR       (0x090)
#define ARM_DEBUG_OFFSET_DBGBVR        (0x100) /* 0x100 - 0x13C */
#define ARM_DEBUG_OFFSET_DBGBCR        (0x140) /* 0x140 - 0x17C */
#define ARM_DEBUG_OFFSET_DBGWVR        (0x180) /* 0x180 - 0x1BC */
#define ARM_DEBUG_OFFSET_DBGWCR        (0x1C0) /* 0x1C0 - 0x1FC */
#define ARM_DEBUG_OFFSET_DBGOSLAR      (0x300)
#define ARM_DEBUG_OFFSET_DBGOSLSR      (0x304)
#define ARM_DEBUG_OFFSET_DBGOSSRR      (0x308)
#define ARM_DEBUG_OFFSET_DBGPRCR       (0x310)
#define ARM_DEBUG_OFFSET_DBGPRSR       (0x314)
#define ARM_DEBUG_OFFSET_DBGITCTRL     (0xF00)
#define ARM_DEBUG_OFFSET_DBGCLAIMSET   (0xFA0)
#define ARM_DEBUG_OFFSET_DBGCLAIMCLR   (0xFA4)
#define ARM_DEBUG_OFFSET_DBGLAR        (0xFB0)
#define ARM_DEBUG_OFFSET_DBGLSR        (0xFB4)
#define ARM_DEBUG_OFFSET_DBGAUTHSTATUS (0xFB8)
#define ARM_DEBUG_OFFSET_DBGDEVID      (0xFC8)
#define ARM_DEBUG_OFFSET_DBGDEVTYPE    (0xFCC)
#define ARM_DEBUG_OFFSET_DBGPID0       (0xFD0)
#define ARM_DEBUG_OFFSET_DBGPID1       (0xFD4)
#define ARM_DEBUG_OFFSET_DBGPID2       (0xFD8)
#define ARM_DEBUG_OFFSET_DBGPID3       (0xFDA)
#define ARM_DEBUG_OFFSET_DBGPID4       (0xFDC)
#define ARM_DEBUG_OFFSET_DBGCID0       (0xFF0)
#define ARM_DEBUG_OFFSET_DBGCID1       (0xFF4)
#define ARM_DEBUG_OFFSET_DBGCID2       (0xFF8)
#define ARM_DEBUG_OFFSET_DBGCID3       (0xFFA)
#define ARM_DEBUG_OFFSET_DBGCID4       (0xFFC)

/*
 * Media and VFP Feature Register 1 (MVFR1)
 */
#define MVFR_ASIMD_HPFP 0x00100000UL

/*
 * Main ID Register (MIDR)
 *
 *  31 24 23 20 19  16 15   4 3   0
 * +-----+-----+------+------+-----+
 * | IMP | VAR | ARCH | PNUM | REV |
 * +-----+-----+------+------+-----+
 *
 * where:
 *   IMP:  Implementor code
 *   VAR:  Variant number
 *   ARCH: Architecture code
 *   PNUM: Primary part number
 *   REV:  Minor revision number
 */
#define MIDR_REV_SHIFT  0
#define MIDR_REV_MASK   (0xf << MIDR_REV_SHIFT)
#define MIDR_PNUM_SHIFT 4
#define MIDR_PNUM_MASK  (0xfff << MIDR_PNUM_SHIFT)
#define MIDR_ARCH_SHIFT 16
#define MIDR_ARCH_MASK  (0xf << MIDR_ARCH_SHIFT)
#define MIDR_VAR_SHIFT  20
#define MIDR_VAR_MASK   (0xf << MIDR_VAR_SHIFT)
#define MIDR_IMP_SHIFT  24
#define MIDR_IMP_MASK   (0xff << MIDR_IMP_SHIFT)

#ifdef __arm__

/* Macros meant to make __builtin_arm_* functions easier to use. */
#define MRC_SCTLR    15,0,1,0,0
#define MCR_SCTLR(x) 15,0,(x),1,0,0

#define MRC_ACTLR    15,0,1,0,1
#define MCR_ACTLR(x) 15,0,(x),1,0,1

#endif /* __arm__ */

#endif /* _ARM_PROC_REG_H_ */
