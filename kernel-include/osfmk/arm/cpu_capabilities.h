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
#ifdef  PRIVATE

#ifndef _ARM_CPU_CAPABILITIES_H
#define _ARM_CPU_CAPABILITIES_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <mach/vm_types.h>
#endif

#define USER_TIMEBASE_NONE   0
#define USER_TIMEBASE_SPEC   1

/*
 * This is the authoritative way to determine from user mode what
 * implementation-specific processor features are available.
 * This API only supported for Apple internal use.
 *
 */

/*
 * Bit definitions for _cpu_capabilities:
 */
#define kHasICDSBShift                  2
#define kHasICDSB                       0x00000004      // ICache Data Syncronization on DSB enabled (H13)
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
#define kHasARMv82FHM                   0x00004000      // Optional ARMv8.2 FMLAL/FMLSL instructions (required in ARMv8.4)
#define kUP                             0x00008000      // set if (kNumCPUs == 1)
#define kNumCPUs                        0x00FF0000      // number of CPUs (see _NumCPUs() below)
#define kHasARMv8Crypto                 0x01000000      // Optional ARMv8 Crypto extensions
#define kHasARMv81Atomics               0x02000000      // ARMv8.1 Atomic instructions supported
#define kHasARMv8Crc32                  0x04000000      // Optional ARMv8 crc32 instructions (required in ARMv8.1)

#define kNumCPUsShift                   16              // see _NumCPUs() below
/*
 * Bit definitions for multiuser_config:
 */
#define kIsMultiUserDevice      0x80000000      // this device is in multiuser mode
#define kMultiUserCurrentUserMask       0x7fffffff      // the current user UID of the multiuser device

#ifndef __ASSEMBLER__
#include <sys/commpage.h>

__BEGIN_DECLS
extern int  _get_cpu_capabilities( void );
__END_DECLS

__inline static
int
_NumCPUs( void )
{
	return (_get_cpu_capabilities() & kNumCPUs) >> kNumCPUsShift;
}


typedef struct {
	volatile uint64_t       TimeBase;
	volatile uint32_t       TimeStamp_sec;
	volatile uint32_t       TimeStamp_usec;
	volatile uint32_t       TimeBaseTicks_per_sec;
	volatile uint32_t       TimeBaseTicks_per_usec;
	volatile uint64_t       TimeBase_magic;
	volatile uint32_t       TimeBase_add;
	volatile uint32_t       TimeBase_shift;
} commpage_timeofday_data_t;

__BEGIN_DECLS
extern vm_address_t                             _get_commpage_priv_address(void);
__END_DECLS

#endif /* __ASSEMBLER__ */


/*
 * The shared kernel/user "comm page(s)":
 */

#if defined(__LP64__)

#define _COMM_PAGE64_BASE_ADDRESS               (0x0000000FFFFFC000ULL) /* In TTBR0 */
#define _COMM_HIGH_PAGE64_BASE_ADDRESS  (0xFFFFFFF0001FC000ULL) /* Just below the kernel, safely in TTBR1; only used for testing */

#define _COMM_PAGE64_AREA_LENGTH                (_COMM_PAGE32_AREA_LENGTH)
#define _COMM_PAGE64_AREA_USED                  (-1)

#define _COMM_PAGE_PRIV(_addr_)                 ((_addr_) - (_COMM_PAGE_START_ADDRESS) + _get_commpage_priv_address())

#ifdef KERNEL_PRIVATE
#define _COMM_PAGE_RW_OFFSET                    (0)
#define _COMM_PAGE_AREA_LENGTH                  (PAGE_SIZE)

#define _COMM_PAGE_BASE_ADDRESS                 (_get_commpage_priv_address())
#define _COMM_PAGE_START_ADDRESS                (_get_commpage_priv_address())
#else /* KERNEL_PRIVATE */
#define _COMM_PAGE_AREA_LENGTH                  (4096)

#define _COMM_PAGE_BASE_ADDRESS                 _COMM_PAGE64_BASE_ADDRESS
#define _COMM_PAGE_START_ADDRESS                _COMM_PAGE64_BASE_ADDRESS
#endif /* KERNEL_PRIVATE */

#else

#define _COMM_PAGE64_BASE_ADDRESS               (-1)
#define _COMM_PAGE64_AREA_LENGTH                (-1)
#define _COMM_PAGE64_AREA_USED                  (-1)

// macro to change a user comm page address to one that is accessible from privileged mode
// this macro is stubbed as PAN is not available on AARCH32,
// but this may still be required for compatibility
#define _COMM_PAGE_PRIV(_addr_)                 (_addr_)

#ifdef KERNEL_PRIVATE
#define _COMM_PAGE_RW_OFFSET                    (_get_commpage_priv_address()-_COMM_PAGE_BASE_ADDRESS)
#define _COMM_PAGE_AREA_LENGTH                  (PAGE_SIZE)
#else
#define _COMM_PAGE_AREA_LENGTH                  (4096)
#endif

#define _COMM_PAGE_BASE_ADDRESS                 _COMM_PAGE32_BASE_ADDRESS
#define _COMM_PAGE_START_ADDRESS                _COMM_PAGE32_BASE_ADDRESS

#endif

#define _COMM_PAGE32_BASE_ADDRESS               (0xFFFF4000)            /* Must be outside of normal map bounds */
#define _COMM_PAGE32_AREA_LENGTH                (_COMM_PAGE_AREA_LENGTH)

#define _COMM_PAGE_TEXT_START                   (-1)
#define _COMM_PAGE32_TEXT_START                 (-1)
#define _COMM_PAGE64_TEXT_START                 (-1)
#define _COMM_PAGE_PFZ_START_OFFSET             (-1)
#define _COMM_PAGE_PFZ_END_OFFSET               (-1)

#define _COMM_PAGE32_OBJC_SIZE                  0ULL
#define _COMM_PAGE32_OBJC_BASE                  0ULL
#define _COMM_PAGE64_OBJC_SIZE                  0ULL
#define _COMM_PAGE64_OBJC_BASE                  0ULL

/*
 * data in the comm pages
 * apply _COMM_PAGE_PRIV macro to use these in privileged mode
 */
#define _COMM_PAGE_SIGNATURE                    (_COMM_PAGE_START_ADDRESS+0x000)        // first few bytes are a signature
#define _COMM_PAGE_SIGNATURELEN                 (0x10)
#define _COMM_PAGE_VERSION                      (_COMM_PAGE_START_ADDRESS+0x01E)        // 16-bit version#
#define _COMM_PAGE_THIS_VERSION                 3                                       // version of the commarea format

#define _COMM_PAGE_CPU_CAPABILITIES             (_COMM_PAGE_START_ADDRESS+0x020)        // uint32_t _cpu_capabilities
#define _COMM_PAGE_NCPUS                        (_COMM_PAGE_START_ADDRESS+0x022)        // uint8_t number of configured CPUs
#define _COMM_PAGE_USER_PAGE_SHIFT_32           (_COMM_PAGE_START_ADDRESS+0x024)        // VM page shift for 32-bit processes
#define _COMM_PAGE_USER_PAGE_SHIFT_64           (_COMM_PAGE_START_ADDRESS+0x025)        // VM page shift for 64-bit processes
#define _COMM_PAGE_CACHE_LINESIZE               (_COMM_PAGE_START_ADDRESS+0x026)        // uint16_t cache line size
#define _COMM_PAGE_SCHED_GEN                    (_COMM_PAGE_START_ADDRESS+0x028)        // uint32_t scheduler generation number (count of pre-emptions)
#define _COMM_PAGE_SPIN_COUNT                   (_COMM_PAGE_START_ADDRESS+0x02C)        // uint32_t max spin count for mutex's
#define _COMM_PAGE_MEMORY_PRESSURE              (_COMM_PAGE_START_ADDRESS+0x030)        // uint32_t copy of vm_memory_pressure
#define _COMM_PAGE_ACTIVE_CPUS                  (_COMM_PAGE_START_ADDRESS+0x034)        // uint8_t number of active CPUs (hw.activecpu)
#define _COMM_PAGE_PHYSICAL_CPUS                (_COMM_PAGE_START_ADDRESS+0x035)        // uint8_t number of physical CPUs (hw.physicalcpu_max)
#define _COMM_PAGE_LOGICAL_CPUS                 (_COMM_PAGE_START_ADDRESS+0x036)        // uint8_t number of logical CPUs (hw.logicalcpu_max)
#define _COMM_PAGE_KERNEL_PAGE_SHIFT            (_COMM_PAGE_START_ADDRESS+0x037)        // uint8_t kernel vm page shift */
#define _COMM_PAGE_MEMORY_SIZE                  (_COMM_PAGE_START_ADDRESS+0x038)        // uint64_t max memory size */
#define _COMM_PAGE_TIMEOFDAY_DATA               (_COMM_PAGE_START_ADDRESS+0x040)        // used by gettimeofday(). Currently, sizeof(commpage_timeofday_data_t) = 40. A new struct is used on gettimeofday but space is reserved on the commpage for compatibility
#define _COMM_PAGE_CPUFAMILY                    (_COMM_PAGE_START_ADDRESS+0x080)        // used by memcpy() resolver
#define _COMM_PAGE_DEV_FIRM                     (_COMM_PAGE_START_ADDRESS+0x084)        // uint32_t handle on PE_i_can_has_debugger
#define _COMM_PAGE_TIMEBASE_OFFSET              (_COMM_PAGE_START_ADDRESS+0x088)        // uint64_t timebase offset for constructing mach_absolute_time()
#define _COMM_PAGE_USER_TIMEBASE                (_COMM_PAGE_START_ADDRESS+0x090)        // uint8_t is userspace mach_absolute_time supported (can read the timebase)
#define _COMM_PAGE_CONT_HWCLOCK                 (_COMM_PAGE_START_ADDRESS+0x091)        // uint8_t is always-on hardware clock present for mach_continuous_time()
#define _COMM_PAGE_DTRACE_DOF_ENABLED           (_COMM_PAGE_START_ADDRESS+0x092)        // uint8_t 0 if userspace DOF disable, 1 if enabled
#define _COMM_PAGE_UNUSED0                      (_COMM_PAGE_START_ADDRESS+0x093)        // 5 unused bytes
#define _COMM_PAGE_CONT_TIMEBASE                (_COMM_PAGE_START_ADDRESS+0x098)        // uint64_t base for mach_continuous_time()
#define _COMM_PAGE_BOOTTIME_USEC                (_COMM_PAGE_START_ADDRESS+0x0A0)        // uint64_t boottime in microseconds

// aligning to 64byte for cacheline size
#define _COMM_PAGE_APPROX_TIME                  (_COMM_PAGE_START_ADDRESS+0x0C0)        // uint64_t last known mach_absolute_time()
#define _COMM_PAGE_APPROX_TIME_SUPPORTED        (_COMM_PAGE_START_ADDRESS+0x0C8)        // uint8_t is mach_approximate_time supported
#define _COMM_PAGE_UNUSED1                      (_COMM_PAGE_START_ADDRESS+0x0C9)        // 55 unused bytes, align next mutable value to a separate cache line

#define _COMM_PAGE_KDEBUG_ENABLE                (_COMM_PAGE_START_ADDRESS+0x100)        // uint32_t export kdebug status bits to userspace
#define _COMM_PAGE_ATM_DIAGNOSTIC_CONFIG        (_COMM_PAGE_START_ADDRESS+0x104)        // uint32_t export "atm_diagnostic_config" to userspace
#define _COMM_PAGE_MULTIUSER_CONFIG             (_COMM_PAGE_START_ADDRESS+0x108)        // uint32_t export "multiuser_config" to userspace


#define _COMM_PAGE_NEWTIMEOFDAY_DATA            (_COMM_PAGE_START_ADDRESS+0x120)        // used by gettimeofday(). Currently, sizeof(new_commpage_timeofday_data_t) = 40.
#define _COMM_PAGE_REMOTETIME_PARAMS            (_COMM_PAGE_START_ADDRESS+0x148)        // used by mach_bridge_remote_time(). Currently, sizeof(struct bt_params) = 24
#define _COMM_PAGE_DYLD_SYSTEM_FLAGS            (_COMM_PAGE_START_ADDRESS+0x160)        // uint64_t export kern.dyld_system_flags to userspace

// aligning to 128 bytes for cacheline/fabric size
#define _COMM_PAGE_CPU_QUIESCENT_COUNTER        (_COMM_PAGE_START_ADDRESS+0x180)        // uint64_t, but reserve the whole 128 (0x80) bytes

#define _COMM_PAGE_END                          (_COMM_PAGE_START_ADDRESS+0x1000)       // end of common page

#endif /* _ARM_CPU_CAPABILITIES_H */
#endif /* PRIVATE */
