/*
 * Copyright (c) 2003-2019 Apple Inc. All rights reserved.
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

#ifndef _I386_CPU_CAPABILITIES_H
#define _I386_CPU_CAPABILITIES_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif

/*
 * This API only supported for Apple internal use.
 */

/* Bit definitions for _cpu_capabilities: */

#define kHasMMX                         0x00000001
#define kHasSSE                         0x00000002
#define kHasSSE2                        0x00000004
#define kHasSSE3                        0x00000008
#define kCache32                        0x00000010      /* cache line size is 32 bytes */
#define kCache64                        0x00000020
#define kCache128                       0x00000040
#define kFastThreadLocalStorage         0x00000080      /* TLS ptr is kept in a user-mode-readable register */
#define kHasSupplementalSSE3            0x00000100
#define k64Bit                          0x00000200      /* processor supports EM64T (not what mode you're running in) */
#define kHasSSE4_1                      0x00000400
#define kHasSSE4_2                      0x00000800
#define kHasAES                         0x00001000
#define kInOrderPipeline                0x00002000
#define kSlow                           0x00004000      /* tsc < nanosecond */
#define kUP                             0x00008000      /* set if (kNumCPUs == 1) */
#define kNumCPUs                        0x00FF0000      /* number of CPUs (see _NumCPUs() below) */
#define kNumCPUsShift                   16
#define kHasAVX1_0                      0x01000000
#define kHasRDRAND                      0x02000000
#define kHasF16C                        0x04000000
#define kHasENFSTRG                     0x08000000
#define kHasFMA                         0x10000000
#define kHasAVX2_0                      0x20000000
#define kHasBMI1                        0x40000000
#define kHasBMI2                        0x80000000
/* Extending into 64-bits from here: */
#define kHasRTM                 0x0000000100000000ULL
#define kHasHLE                 0x0000000200000000ULL
#define kHasRDSEED              0x0000000800000000ULL
#define kHasADX                 0x0000000400000000ULL
#define kHasMPX                 0x0000001000000000ULL
#define kHasSGX                 0x0000002000000000ULL
#define kHasAVX512F             0x0000004000000000ULL
#define kHasAVX512CD            0x0000008000000000ULL
#define kHasAVX512DQ            0x0000010000000000ULL
#define kHasAVX512BW            0x0000020000000000ULL
#define kHasAVX512IFMA          0x0000040000000000ULL
#define kHasAVX512VBMI          0x0000080000000000ULL
#define kHasAVX512VL            0x0000100000000000ULL
#define kHasVAES                0x0000200000000000ULL
#define kHasVPCLMULQDQ          0x0000400000000000ULL
#define kHasAVX512VNNI          0x0000800000000000ULL
#define kHasAVX512BITALG        0x0001000000000000ULL
#define kHasAVX512VPOPCNTDQ     0x0002000000000000ULL


#ifndef __ASSEMBLER__
#include <sys/cdefs.h>
#include <sys/commpage.h>

__BEGIN_DECLS
extern uint64_t  _get_cpu_capabilities( void );
__END_DECLS

__inline static
int
_NumCPUs( void )
{
	return (int) (_get_cpu_capabilities() & kNumCPUs) >> kNumCPUsShift;
}

#endif /* __ASSEMBLER__ */

/* The following macro is used to generate the 64-bit commpage address for a given
 * routine, based on its 32-bit address.  This is used in the kernel to compile
 * the 64-bit commpage.  Since the kernel can be a 32-bit object, cpu_capabilities.h
 * only defines the 32-bit address.
 */
#define _COMM_PAGE_32_TO_64( ADDRESS )  ( ADDRESS + _COMM_PAGE64_START_ADDRESS - _COMM_PAGE32_START_ADDRESS )


/*
 * The shared kernel/user "comm page(s)":
 *
 * The last several pages of every address space are reserved for the kernel/user
 * "comm area". During system initialization, the kernel populates the comm pages with
 * code customized for the particular processor and platform.
 *
 * Because Mach VM cannot map the last page of an address space, we don't use it.
 */

#define _COMM_PAGE32_AREA_LENGTH        ( 1 * 4096 )                            /* reserved length of entire comm area */
#define _COMM_PAGE32_BASE_ADDRESS       ( 0xffff0000 )                          /* base address of allocated memory */
#define _COMM_PAGE32_START_ADDRESS      ( _COMM_PAGE32_BASE_ADDRESS )   /* address traditional commpage code starts on */
#define _COMM_PAGE32_AREA_USED          ( 1 * 4096 )                            /* this is the amt actually allocated */
#define _COMM_PAGE32_SIGS_OFFSET        0x8000                                      /* offset to routine signatures */

#define _COMM_PAGE64_AREA_LENGTH        ( 1 * 4096 )                    /* reserved length of entire comm area (2MB) */
#ifdef __ASSEMBLER__
#define _COMM_PAGE64_BASE_ADDRESS       ( 0x00007fffffe00000 )   /* base address of allocated memory */
#else /* __ASSEMBLER__ */
#define _COMM_PAGE64_BASE_ADDRESS       ( 0x00007fffffe00000ULL )   /* base address of allocated memory */
#endif /* __ASSEMBLER__ */
#define _COMM_PAGE64_START_ADDRESS      ( _COMM_PAGE64_BASE_ADDRESS )   /* address traditional commpage code starts on */
#define _COMM_PAGE64_AREA_USED          ( 1 * 4096 )                            /* this is the amt actually populated */

/* no need for an Objective-C area on Intel */
#define _COMM_PAGE32_OBJC_SIZE          0ULL
#define _COMM_PAGE32_OBJC_BASE          0ULL
#define _COMM_PAGE64_OBJC_SIZE          0ULL
#define _COMM_PAGE64_OBJC_BASE          0ULL

#ifdef KERNEL_PRIVATE

/* Inside the kernel, comm page addresses are absolute addresses
 * assuming they are a part of the 32-bit commpage. They may
 * be mapped somewhere else, especially for the 64-bit commpage.
 */
#define _COMM_PAGE_START_ADDRESS        _COMM_PAGE32_START_ADDRESS
#define _COMM_PAGE_SIGS_OFFSET          _COMM_PAGE32_SIGS_OFFSET

#else /* !KERNEL_PRIVATE */

#if defined(__i386__)

#define _COMM_PAGE_AREA_LENGTH          _COMM_PAGE32_AREA_LENGTH
#define _COMM_PAGE_BASE_ADDRESS         _COMM_PAGE32_BASE_ADDRESS
#define _COMM_PAGE_START_ADDRESS        _COMM_PAGE32_START_ADDRESS
#define _COMM_PAGE_AREA_USED            _COMM_PAGE32_AREA_USED
#define _COMM_PAGE_SIGS_OFFSET          _COMM_PAGE32_SIGS_OFFSET

#elif defined(__x86_64__)

#define _COMM_PAGE_AREA_LENGTH          _COMM_PAGE64_AREA_LENGTH
#define _COMM_PAGE_BASE_ADDRESS         _COMM_PAGE64_BASE_ADDRESS
#define _COMM_PAGE_START_ADDRESS        _COMM_PAGE64_START_ADDRESS
#define _COMM_PAGE_AREA_USED            _COMM_PAGE64_AREA_USED

#else
#error architecture not supported
#endif

#endif /* !KERNEL_PRIVATE */

/* data in the comm page */

#define _COMM_PAGE_SIGNATURE            (_COMM_PAGE_START_ADDRESS+0x000)        /* first 16 bytes are a signature */
#define _COMM_PAGE_SIGNATURELEN         (0x10)
#define _COMM_PAGE_CPU_CAPABILITIES64   (_COMM_PAGE_START_ADDRESS+0x010)        /* uint64_t _cpu_capabilities */
#define _COMM_PAGE_UNUSED               (_COMM_PAGE_START_ADDRESS+0x018)        /* 6 unused bytes */
#define _COMM_PAGE_VERSION              (_COMM_PAGE_START_ADDRESS+0x01E)        /* 16-bit version# */
#define _COMM_PAGE_THIS_VERSION         13                                      /* in ver 13, _COMM_PAGE_NT_SHIFT defaults to 0 (was 32) */

#define _COMM_PAGE_CPU_CAPABILITIES     (_COMM_PAGE_START_ADDRESS+0x020)        /* uint32_t _cpu_capabilities (retained for compatibility) */
#define _COMM_PAGE_NCPUS                (_COMM_PAGE_START_ADDRESS+0x022)        /* uint8_t number of configured CPUs (hw.logicalcpu at boot time) */
#define _COMM_PAGE_UNUSED0              (_COMM_PAGE_START_ADDRESS+0x024)        /* 2 unused bytes, previouly reserved for expansion of cpu_capabilities */
#define _COMM_PAGE_CACHE_LINESIZE       (_COMM_PAGE_START_ADDRESS+0x026)        /* uint16_t cache line size */

#define _COMM_PAGE_SCHED_GEN            (_COMM_PAGE_START_ADDRESS+0x028)        /* uint32_t scheduler generation number (count of pre-emptions) */
#define _COMM_PAGE_MEMORY_PRESSURE      (_COMM_PAGE_START_ADDRESS+0x02c)        /* uint32_t copy of vm_memory_pressure */
#define _COMM_PAGE_SPIN_COUNT           (_COMM_PAGE_START_ADDRESS+0x030)        /* uint32_t max spin count for mutex's */

#define _COMM_PAGE_ACTIVE_CPUS          (_COMM_PAGE_START_ADDRESS+0x034)        /* uint8_t number of active CPUs (hw.activecpu) */
#define _COMM_PAGE_PHYSICAL_CPUS        (_COMM_PAGE_START_ADDRESS+0x035)        /* uint8_t number of physical CPUs (hw.physicalcpu_max) */
#define _COMM_PAGE_LOGICAL_CPUS         (_COMM_PAGE_START_ADDRESS+0x036)        /* uint8_t number of logical CPUs (hw.logicalcpu_max) */
#define _COMM_PAGE_UNUSED1              (_COMM_PAGE_START_ADDRESS+0x037)        /* 1 unused bytes */
#define _COMM_PAGE_MEMORY_SIZE          (_COMM_PAGE_START_ADDRESS+0x038)        /* uint64_t max memory size */

#define _COMM_PAGE_CPUFAMILY            (_COMM_PAGE_START_ADDRESS+0x040)        /* uint32_t hw.cpufamily, x86*/
#define _COMM_PAGE_KDEBUG_ENABLE        (_COMM_PAGE_START_ADDRESS+0x044)        /* uint32_t export "kdebug_enable" to userspace */
#define _COMM_PAGE_ATM_DIAGNOSTIC_CONFIG        (_COMM_PAGE_START_ADDRESS+0x48) /* uint32_t export "atm_diagnostic_config" to userspace */

#define _COMM_PAGE_DTRACE_DOF_ENABLED   (_COMM_PAGE_START_ADDRESS+0x04C)        /* uint8_t 0 if userspace DOF disable, 1 if enabled */
#define _COMM_PAGE_UNUSED2              (_COMM_PAGE_START_ADDRESS+0x04D)        /* [0x4D,0x50) unused */

#define _COMM_PAGE_TIME_DATA_START      (_COMM_PAGE_START_ADDRESS+0x050)        /* base of offsets below (_NT_SCALE etc) */
#define _COMM_PAGE_NT_TSC_BASE          (_COMM_PAGE_START_ADDRESS+0x050)        /* used by nanotime() */
#define _COMM_PAGE_NT_SCALE             (_COMM_PAGE_START_ADDRESS+0x058)        /* used by nanotime() */
#define _COMM_PAGE_NT_SHIFT             (_COMM_PAGE_START_ADDRESS+0x05c)        /* used by nanotime() */
#define _COMM_PAGE_NT_NS_BASE           (_COMM_PAGE_START_ADDRESS+0x060)        /* used by nanotime() */
#define _COMM_PAGE_NT_GENERATION        (_COMM_PAGE_START_ADDRESS+0x068)        /* used by nanotime() */
#define _COMM_PAGE_GTOD_GENERATION      (_COMM_PAGE_START_ADDRESS+0x06c)        /* used by gettimeofday() */
#define _COMM_PAGE_GTOD_NS_BASE         (_COMM_PAGE_START_ADDRESS+0x070)        /* used by gettimeofday() */
#define _COMM_PAGE_GTOD_SEC_BASE        (_COMM_PAGE_START_ADDRESS+0x078)        /* used by gettimeofday() */

/* NOTE: APPROX_TIME must be aligned to 64-byte cache line size: */
#define _COMM_PAGE_APPROX_TIME          (_COMM_PAGE_START_ADDRESS+0x080)        /* used by mach_approximate_time() */
#define _COMM_PAGE_APPROX_TIME_SUPPORTED (_COMM_PAGE_START_ADDRESS+0x088)       /* used by mach_approximate_time() */

/* Align following entries to next cache line */
#define _COMM_PAGE_CONT_TIMEBASE        (_COMM_PAGE_START_ADDRESS+0x0C0)        /* used by mach_continuous_time() */
#define _COMM_PAGE_BOOTTIME_USEC        (_COMM_PAGE_START_ADDRESS+0x0C8)        /* uint64_t boottime */
#define _COMM_PAGE_NEWTIMEOFDAY_DATA    (_COMM_PAGE_START_ADDRESS+0x0D0)        /* used by gettimeofday(). Currently, sizeof(new_commpage_timeofday_data_t) = 40 */

/* Resume packed values to the next cacheline */
#define _COMM_PAGE_DYLD_SYSTEM_FLAGS    (_COMM_PAGE_START_ADDRESS+0x100)        /* uint64_t export kern.dyld_system_flags to userspace */

#define _COMM_PAGE_END                  (_COMM_PAGE_START_ADDRESS+0xfff)        /* end of common page */

/* Warning: kernel commpage.h has a matching c typedef for the following.  They must be kept in sync.  */
/* These offsets are from _COMM_PAGE_TIME_DATA_START */

#define _NT_TSC_BASE                    0
#define _NT_SCALE                       8
#define _NT_SHIFT                       12
#define _NT_NS_BASE                     16
#define _NT_GENERATION                  24
#define _GTOD_GENERATION                28
#define _GTOD_NS_BASE                   32
#define _GTOD_SEC_BASE                  40

/* jump table (jmp to this address, which may be a branch to the actual code somewhere else) */
/* When new jump table entries are added, corresponding symbols should be added below        */
/* New slots should be allocated with at least 16-byte alignment. Some like bcopy require    */
/* 32-byte alignment, and should be aligned as such in the assembly source before they are relocated */
#define _COMM_PAGE_TEXT_START           (_COMM_PAGE_START_ADDRESS+0x1000)
#define _COMM_PAGE32_TEXT_START         (_COMM_PAGE32_BASE_ADDRESS+0x1000)    /* start of text section */
#define _COMM_PAGE64_TEXT_START         (_COMM_PAGE64_BASE_ADDRESS+0x1000)
#define _COMM_PAGE_TEXT_AREA_USED       ( 1 * 4096 )
#define _COMM_PAGE_TEXT_AREA_LENGTH     ( 1 * 4096 )
#define _PFZ32_SLIDE_RANGE              ( 14 ) /* pages between 0xfffff000 and _COMM_PAGE32_TEXT_START */
#define _PFZ64_SLIDE_RANGE              ( 510 ) /* pages between 0x00007ffffffff000 and _COMM_PAGE64_TEXT_START */

/* setup start offset in the commpage text region for each jump table entry
 * the Comm Page Offset is shortened to _COMM_TEXT_[label]_OFFSET
 */

#define _COMM_TEXT_PREEMPT_OFFSET               (0x5a0) /* called from withing pfz */
#define _COMM_TEXT_BACKOFF_OFFSET               (0x600) /* called from PFZ */
#define _COMM_TEXT_RET_OFFSET                   (0x680) /* called from PFZ */
#define _COMM_TEXT_PFZ_START_OFFSET             (0xc00) /* offset for Preemption Free Zone */
#define _COMM_TEXT_PFZ_ENQUEUE_OFFSET           (0xc00) /* internal FIFO enqueue */
#define _COMM_TEXT_PFZ_DEQUEUE_OFFSET           (0xc80) /* internal FIFO dequeue */
#define _COMM_TEXT_UNUSED_OFFSET                (0xd00) /* end of routines in text page */
#define _COMM_TEXT_PFZ_END_OFFSET               (0xd00) /* offset for end of PFZ */


#define _COMM_PAGE_PREEMPT              (_COMM_PAGE_TEXT_START+_COMM_TEXT_PREEMPT_OFFSET)
#define _COMM_PAGE_BACKOFF              (_COMM_PAGE_TEXT_START+_COMM_TEXT_BACKOFF_OFFSET)
#define _COMM_PAGE_RET                  (_COMM_PAGE_TEXT_START+_COMM_TEXT_RET_OFFSET)

#define _COMM_PAGE_PFZ_START            (_COMM_PAGE_TEXT_START+_COMM_PAGE_PFZ_START_OFFSET)

#define _COMM_PAGE_PFZ_ENQUEUE          (_COMM_PAGE_TEXT_START+_COMM_TEXT_PFZ_ENQUEUE_OFFSET)
#define _COMM_PAGE_PFZ_DEQUEUE          (_COMM_PAGE_TEXT_START+_COMM_TEXT_PFZ_DEQUEUE_OFFSET)

#define _COMM_PAGE_UNUSED6              (_COMM_PAGE_TEXT_START+_COMM_TEXT_UNUSED_OFFSET)
#define _COMM_PAGE_PFZ_END              (_COMM_PAGE_TEXT_START+_COMM_TEXT_PFZ_END_OFFSET)
#define _COMM_PAGE_TEXT_END             (_COMM_PAGE_TEXT_START+_COMM_TEXT_PFZ_END_OFFSET) /* end of common text page */

/* _COMM_PAGE_COMPARE_AND_SWAP{32,64}B are not used on x86 and are
 * maintained here for source compatability.  These will be removed at
 * some point, so don't go relying on them. */
#define _COMM_PAGE_COMPARE_AND_SWAP32B  (_COMM_PAGE_START_ADDRESS+0xf80)        /* compare-and-swap word w barrier */
#define _COMM_PAGE_COMPARE_AND_SWAP64B  (_COMM_PAGE_START_ADDRESS+0xfc0)        /* compare-and-swap doubleword w barrier */

#ifdef __ASSEMBLER__
#ifdef __COMM_PAGE_SYMBOLS

#define CREATE_COMM_PAGE_SYMBOL(symbol_name, symbol_address)            \
	                        .org	(symbol_address - (_COMM_PAGE_START_ADDRESS & 0xFFFFE000)) ;\
symbol_name: nop

.text
/* Required to make a well behaved symbol file */

CREATE_COMM_PAGE_SYMBOL(___preempt, _COMM_PAGE_PREEMPT)
CREATE_COMM_PAGE_SYMBOL(___backoff, _COMM_PAGE_BACKOFF)
CREATE_COMM_PAGE_SYMBOL(___pfz_enqueue, _COMM_PAGE_PFZ_ENQUEUE)
CREATE_COMM_PAGE_SYMBOL(___pfz_dequeue, _COMM_PAGE_PFZ_DEQUEUE)
CREATE_COMM_PAGE_SYMBOL(___end_comm_page, _COMM_PAGE_END)

.data                   /* Required to make a well behaved symbol file */
.long   0               /* Required to make a well behaved symbol file */

#endif /* __COMM_PAGE_SYMBOLS */
#endif /* __ASSEMBLER__ */

#endif /* _I386_CPU_CAPABILITIES_H */
#endif /* PRIVATE */
