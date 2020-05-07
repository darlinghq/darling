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
#ifdef KERNEL_PRIVATE
#ifndef _I386_MACHINE_CHECK_H_
#define _I386_MACHINE_CHECK_H_

#include <stdint.h>

#include <i386/cpu_data.h>

/*
 * This header defines the machine check architecture for Pentium4 and Xeon.
 */

/*
 * Macro BITS(n,m) returns the number of bits between bit(n) and bit(m),
 * where (n>m). Macro BIT1(n) is cosmetic and returns 1.
 */
#define BITS(n, m)       ((n)-(m)+1)
#define BIT1(n)         (1)

/*
 * IA32 SDM 14.3.1 Machine-Check Global Control MSRs:
 */
#define IA32_MCG_CAP            (0x179)
typedef union {
	struct {
		uint64_t        count                   :BITS(7, 0);
		uint64_t        mcg_ctl_p               :BIT1(8);
		uint64_t        mcg_ext_p               :BIT1(9);
		uint64_t        mcg_ext_corr_err_p      :BIT1(10);
		uint64_t        mcg_tes_p               :BIT1(11);
		uint64_t        mcg_ecms                :BIT1(12);
		uint64_t        mcg_reserved2           :BITS(15, 13);
		uint64_t        mcg_ext_cnt             :BITS(23, 16);
		uint64_t        mcg_ser_p               :BIT1(24);
	}          bits;
	uint64_t   u64;
} ia32_mcg_cap_t;

#define IA32_MCG_STATUS         (0x17A)
typedef union {
	struct {
		uint64_t        ripv                    :BIT1(0);
		uint64_t        eipv                    :BIT1(1);
		uint64_t        mcip                    :BIT1(2);
	}          bits;
	uint64_t   u64;
} ia32_mcg_status_t;

#define IA32_MCG_CTL            (0x17B)
typedef uint64_t        ia32_mcg_ctl_t;
#define IA32_MCG_CTL_ENABLE     (0xFFFFFFFFFFFFFFFFULL)
#define IA32_MCG_CTL_DISABLE    (0x0ULL)


/*
 * IA32 SDM 14.3.2 Error-Reporting Register Banks:
 */
#define IA32_MCi_CTL(i)         (0x400 + 4*(i))
#define IA32_MCi_STATUS(i)      (0x401 + 4*(i))
#define IA32_MCi_ADDR(i)        (0x402 + 4*(i))
#define IA32_MCi_MISC(i)        (0x403 + 4*(i))

#define IA32_MC0_CTL            IA32_MCi_CTL(0)
#define IA32_MC0_STATUS         IA32_MCi_STATUS(0)
#define IA32_MC0_ADDR           IA32_MCi_ADDR(0)
#define IA32_MC0_MISC           IA32_MCi_MISC(0)

#define IA32_MC1_CTL            IA32_MCi_CTL(1)
#define IA32_MC1_STATUS         IA32_MCi_STATUS(1)
#define IA32_MC1_ADDR           IA32_MCi_ADDR(1)
#define IA32_MC1_MISC           IA32_MCi_MISC(1)

#define IA32_MC2_CTL            IA32_MCi_CTL(2)
#define IA32_MC2_STATUS         IA32_MCi_STATUS(2)
#define IA32_MC2_ADDR           IA32_MCi_ADDR(2)
#define IA32_MC2_MISC           IA32_MCi_MISC(2)

#define IA32_MC3_CTL            IA32_MCi_CTL(3)
#define IA32_MC3_STATUS         IA32_MCi_STATUS(3)
#define IA32_MC3_ADDR           IA32_MCi_ADDR(3)
#define IA32_MC3_MISC           IA32_MCi_MISC(3)

#define IA32_MC4_CTL            IA32_MCi_CTL(4)
#define IA32_MC4_STATUS         IA32_MCi_STATUS(4)
#define IA32_MC4_ADDR           IA32_MCi_ADDR(4)
#define IA32_MC4_MISC           IA32_MCi_MISC(4)

typedef uint64_t        ia32_mci_ctl_t;
#define IA32_MCi_CTL_EE(j)      (0x1ULL << (j))
#define IA32_MCi_CTL_ENABLE_ALL (0xFFFFFFFFFFFFFFFFULL)

typedef union {
	struct {
		uint64_t        mca_error               :BITS(15, 0);
		uint64_t        model_specific_error    :BITS(31, 16);
		uint64_t        other_information       :BITS(56, 32);
		uint64_t        pcc                     :BIT1(57);
		uint64_t        addrv                   :BIT1(58);
		uint64_t        miscv                   :BIT1(59);
		uint64_t        en                      :BIT1(60);
		uint64_t        uc                      :BIT1(61);
		uint64_t        over                    :BIT1(62);
		uint64_t        val                     :BIT1(63);
	}           bits;
	struct {        /* Variant if threshold-based error status present: */
		uint64_t        mca_error               :BITS(15, 0);
		uint64_t        model_specific_error    :BITS(31, 16);
		uint64_t        other_information       :BITS(52, 32);
		uint64_t        threshold               :BITS(54, 53);
		uint64_t        ar                      :BIT1(55);
		uint64_t        s                       :BIT1(56);
		uint64_t        pcc                     :BIT1(57);
		uint64_t        addrv                   :BIT1(58);
		uint64_t        miscv                   :BIT1(59);
		uint64_t        en                      :BIT1(60);
		uint64_t        uc                      :BIT1(61);
		uint64_t        over                    :BIT1(62);
		uint64_t        val                     :BIT1(63);
	}           bits_tes_p;
	uint64_t    u64;
} ia32_mci_status_t;

/* Values for threshold_status if mcg_tes_p == 1 and uc == 0 */
#define THRESHOLD_STATUS_NO_TRACKING    0
#define THRESHOLD_STATUS_GREEN          1
#define THRESHOLD_STATUS_YELLOW         2
#define THRESHOLD_STATUS_RESERVED       3

typedef uint64_t        ia32_mci_addr_t;
typedef uint64_t        ia32_mci_misc_t;

extern void             mca_cpu_alloc(cpu_data_t *cdp);
extern void             mca_cpu_init(void);
extern void             mca_dump(void);
extern void             mca_check_save(void);
extern boolean_t        mca_is_cmci_present(void);

#endif  /* _I386_MACHINE_CHECK_H_ */
#endif  /* KERNEL_PRIVATE */
