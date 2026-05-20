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
#ifdef	PRIVATE

#ifndef _ARM_CPU_CAPABILITIES_H
#define _ARM_CPU_CAPABILITIES_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif

/*
 * Bit definitions for _cpu_capabilities:
 */
#define kHasNeonFP16                    0x00000008      // ARM v8.2 NEON FP16 supported
#define kCache32                        0x00000010      // cache line size is 32 bytes
#define kCache64                        0x00000020      // cache line size is 64 bytes
#define kCache128                       0x00000040      // cache line size is 128 bytes
#define kFastThreadLocalStorage         0x00000080      // TLS ptr is kept in a user-mode-readable register
#define kHasNeon                        0x00000100      // Advanced SIMD is supported
#define kHasNeonHPFP                    0x00000200      // Advanced SIMD half-precision
#define kHasVfp                         0x00000400      // VFP is supported
#define kHasUCNormalMemory              0x00000800      // Uncacheable normal memory type supported
#define kHasEvent                       0x00001000      // WFE/SVE and period event wakeup
#define kHasFMA                         0x00002000      // Fused multiply add is supported
#define kHasARMv82FHM                   0x00004000      // Optional ARMv8.2 FMLAL/FMLSL instructions
#define kUP                             0x00008000      // set if (kNumCPUs == 1)
#define kNumCPUs                        0x00FF0000      // number of CPUs (see _NumCPUs() below)
#define kHasARMv8Crypto                 0x01000000      // Optional ARMv8 Crypto extensions
#define kHasARMv81Atomics               0x02000000      // ARMv8.1 Atomic instructions supported
#define kHasARMv8Crc32                  0x04000000      // Optional ARMv8 crc32 instructions
#define kHasARMv82SHA512                0x80000000      // Optional ARMv8.2 SHA512 instructions
/* Extending into 64-bits from here: */
#define kHasARMv82SHA3          0x0000000100000000ULL    // Optional ARMv8.2 SHA3 instructions

#define kNumCPUsShift                   16              // see _NumCPUs() below

/*
 * The shared kernel/user "comm page(s)":
 */

#define _COMM_PAGE32_BASE_ADDRESS               (0xFFFF4000)
#define _COMM_PAGE32_AREA_LENGTH                (4096)
#define _COMM_PAGE32_TEXT_START                  (-1)

#define _COMM_PAGE64_BASE_ADDRESS               (0x0000000FFFFFC000ULL)
#define _COMM_PAGE64_AREA_LENGTH                (4096)

#define _COMM_PAGE32_OBJC_SIZE                  0ULL
#define _COMM_PAGE32_OBJC_BASE                  0ULL
#define _COMM_PAGE64_OBJC_SIZE                  0ULL
#define _COMM_PAGE64_OBJC_BASE                  0ULL

#define _COMM_PAGE_AREA_LENGTH                  (4096)
#define _COMM_PAGE_BASE_ADDRESS                 _COMM_PAGE64_BASE_ADDRESS
#define _COMM_PAGE_START_ADDRESS                _COMM_PAGE64_BASE_ADDRESS

/*
 * data in the comm pages
 */
#define _COMM_PAGE_SIGNATURE                    (_COMM_PAGE_START_ADDRESS+0x000)
#define _COMM_PAGE_SIGNATURELEN                 (0x10)
#define _COMM_PAGE_CPU_CAPABILITIES64           (_COMM_PAGE_START_ADDRESS+0x010)
#define _COMM_PAGE_UNUSED                       (_COMM_PAGE_START_ADDRESS+0x018)
#define _COMM_PAGE_VERSION                      (_COMM_PAGE_START_ADDRESS+0x01E)
#define _COMM_PAGE_THIS_VERSION                 3

#define _COMM_PAGE_CPU_CAPABILITIES             (_COMM_PAGE_START_ADDRESS+0x020)
#define _COMM_PAGE_NCPUS                        (_COMM_PAGE_START_ADDRESS+0x022)
#define _COMM_PAGE_USER_PAGE_SHIFT_32           (_COMM_PAGE_START_ADDRESS+0x024)
#define _COMM_PAGE_USER_PAGE_SHIFT_64           (_COMM_PAGE_START_ADDRESS+0x025)
#define _COMM_PAGE_CACHE_LINESIZE               (_COMM_PAGE_START_ADDRESS+0x026)
#define _COMM_PAGE_UNUSED4                      (_COMM_PAGE_START_ADDRESS+0x028)
#define _COMM_PAGE_UNUSED3                      (_COMM_PAGE_START_ADDRESS+0x02C)
#define _COMM_PAGE_MEMORY_PRESSURE              (_COMM_PAGE_START_ADDRESS+0x030)
#define _COMM_PAGE_ACTIVE_CPUS                  (_COMM_PAGE_START_ADDRESS+0x034)
#define _COMM_PAGE_PHYSICAL_CPUS                (_COMM_PAGE_START_ADDRESS+0x035)
#define _COMM_PAGE_LOGICAL_CPUS                 (_COMM_PAGE_START_ADDRESS+0x036)
#define _COMM_PAGE_KERNEL_PAGE_SHIFT            (_COMM_PAGE_START_ADDRESS+0x037)
#define _COMM_PAGE_MEMORY_SIZE                  (_COMM_PAGE_START_ADDRESS+0x038)
#define _COMM_PAGE_TIMEOFDAY_DATA               (_COMM_PAGE_START_ADDRESS+0x040)
#define _COMM_PAGE_CPUFAMILY                    (_COMM_PAGE_START_ADDRESS+0x080)
#define _COMM_PAGE_DEV_FIRM                     (_COMM_PAGE_START_ADDRESS+0x084)
#define _COMM_PAGE_TIMEBASE_OFFSET              (_COMM_PAGE_START_ADDRESS+0x088)
#define _COMM_PAGE_USER_TIMEBASE                (_COMM_PAGE_START_ADDRESS+0x090)
#define _COMM_PAGE_CONT_HWCLOCK                 (_COMM_PAGE_START_ADDRESS+0x091)
#define _COMM_PAGE_DTRACE_DOF_ENABLED           (_COMM_PAGE_START_ADDRESS+0x092)
#define _COMM_PAGE_CONT_TIMEBASE                (_COMM_PAGE_START_ADDRESS+0x098)
#define _COMM_PAGE_BOOTTIME_USEC                (_COMM_PAGE_START_ADDRESS+0x0A0)

#define _COMM_PAGE_APPROX_TIME                  (_COMM_PAGE_START_ADDRESS+0x0C0)
#define _COMM_PAGE_APPROX_TIME_SUPPORTED        (_COMM_PAGE_START_ADDRESS+0x0C8)

#define _COMM_PAGE_KDEBUG_ENABLE                (_COMM_PAGE_START_ADDRESS+0x100)
#define _COMM_PAGE_ATM_DIAGNOSTIC_CONFIG        (_COMM_PAGE_START_ADDRESS+0x104)
#define _COMM_PAGE_MULTIUSER_CONFIG             (_COMM_PAGE_START_ADDRESS+0x108)

#define _COMM_PAGE_NEWTIMEOFDAY_DATA            (_COMM_PAGE_START_ADDRESS+0x120)
#define _COMM_PAGE_DYLD_FLAGS                   (_COMM_PAGE_START_ADDRESS+0x160)

#define _COMM_PAGE_END                          (_COMM_PAGE_START_ADDRESS+0xfff)

#endif /* _ARM_CPU_CAPABILITIES_H */
#endif /* PRIVATE */
