/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
 * @OSF_FREE_COPYRIGHT@
 */
/*
 * @APPLE_FREE_COPYRIGHT@
 */

/*
 *	Here are the Diagnostic interface interfaces
 *	Lovingly crafted by Bill Angell using traditional methods
 */
#ifdef  KERNEL_PRIVATE

#ifndef _DIAGNOSTICS_H_
#define _DIAGNOSTICS_H_

#if !(defined(__i386__) || defined(__x86_64__))
#error This file is not useful on non-Intel
#endif

int diagCall64(x86_saved_state_t *regs);

#define diagSCnum 0x00006000

#define dgAdjTB 0
#define dgLRA 1
#define dgpcpy 2
#define dgreset 3
#define dgtest 4
#define dgBMphys 5
#define dgUnMap 6
#define dgBootScreen 7
#define dgFlush 8
#define dgAlign 9
#define dgGzallocTest 10
#define dgmck 11
#define dg64 12
#define dgProbeRead 13
#define dgCPNull 14
#define dgPerfMon 15
#define dgMapPage 16
#define dgPowerStat 17
#define dgBind 18
#define dgAcntg 20
#define dgKlra 21
#define dgEnaPMC 22
#define dgWar 23
#define dgNapStat 24
#define dgRuptStat 25
#define dgPermCheck 26

typedef struct diagWork {                       /* Diagnostic work area */
	unsigned int dgLock;                    /* Lock if needed */
	unsigned int dgFlags;                   /* Flags */
#define enaExpTrace 0x00000001
#define enaUsrFCall 0x00000002
#define enaUsrPhyMp 0x00000004
#define enaDiagSCs  0x00000008
#define enaDiagDM  0x00000010
#define enaDiagEM  0x00000020
#define enaDiagTrap  0x00000040
#define enaNotifyEM  0x00000080

	unsigned int dgMisc0;
	unsigned int dgMisc1;
	unsigned int dgMisc2;
	unsigned int dgMisc3;
	unsigned int dgMisc4;
	unsigned int dgMisc5;
} diagWork;

extern diagWork dgWork;

#define FIXED_PMC (1 << 30)
#define FIXED_PMC0 (FIXED_PMC)
#define FIXED_PMC1 (FIXED_PMC | 1)
#define FIXED_PMC2 (FIXED_PMC | 2)
#define GPMC0 (0)
#define GPMC1 (1)
#define GPMC2 (2)
#define GPMC3 (3)

static inline uint64_t
read_pmc(uint32_t counter)
{
	uint32_t lo = 0, hi = 0;
	__asm__ volatile ("rdpmc" : "=a" (lo), "=d" (hi) : "c" (counter));
	return (((uint64_t)hi) << 32) | ((uint64_t)lo);
}
#endif /* _DIAGNOSTICS_H_ */

#endif /* KERNEL_PRIVATE */
