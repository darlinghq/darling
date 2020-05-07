/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
 * Processor registers for i386 and i486.
 */
#ifndef _I386_PROC_REG_H_
#define _I386_PROC_REG_H_

/*
 * Model Specific Registers
 */
#define MSR_P5_TSC              0x10    /* Time Stamp Register */
#define MSR_P5_CESR             0x11    /* Control and Event Select Register */
#define MSR_P5_CTR0             0x12    /* Counter #0 */
#define MSR_P5_CTR1             0x13    /* Counter #1 */

#define MSR_P5_CESR_PC          0x0200  /* Pin Control */
#define MSR_P5_CESR_CC          0x01C0  /* Counter Control mask */
#define MSR_P5_CESR_ES          0x003F  /* Event Control mask */

#define MSR_P5_CESR_SHIFT       16              /* Shift to get Counter 1 */
#define MSR_P5_CESR_MASK        (MSR_P5_CESR_PC|\
	                         MSR_P5_CESR_CC|\
	                         MSR_P5_CESR_ES) /* Mask Counter */

#define MSR_P5_CESR_CC_CLOCK    0x0100  /* Clock Counting (otherwise Event) */
#define MSR_P5_CESR_CC_DISABLE  0x0000  /* Disable counter */
#define MSR_P5_CESR_CC_CPL012   0x0040  /* Count if the CPL == 0, 1, 2 */
#define MSR_P5_CESR_CC_CPL3     0x0080  /* Count if the CPL == 3 */
#define MSR_P5_CESR_CC_CPL      0x00C0  /* Count regardless of the CPL */

#define MSR_P5_CESR_ES_DATA_READ       0x000000 /* Data Read */
#define MSR_P5_CESR_ES_DATA_WRITE      0x000001 /* Data Write */
#define MSR_P5_CESR_ES_DATA_RW         0x101000 /* Data Read or Write */
#define MSR_P5_CESR_ES_DATA_TLB_MISS   0x000010 /* Data TLB Miss */
#define MSR_P5_CESR_ES_DATA_READ_MISS  0x000011 /* Data Read Miss */
#define MSR_P5_CESR_ES_DATA_WRITE_MISS 0x000100 /* Data Write Miss */
#define MSR_P5_CESR_ES_DATA_RW_MISS    0x101001 /* Data Read or Write Miss */
#define MSR_P5_CESR_ES_HIT_EM          0x000101 /* Write (hit) to M|E state */
#define MSR_P5_CESR_ES_DATA_CACHE_WB   0x000110 /* Cache lines written back */
#define MSR_P5_CESR_ES_EXTERNAL_SNOOP  0x000111 /* External Snoop */
#define MSR_P5_CESR_ES_CACHE_SNOOP_HIT 0x001000 /* Data cache snoop hits */
#define MSR_P5_CESR_ES_MEM_ACCESS_PIPE 0x001001 /* Mem. access in both pipes */
#define MSR_P5_CESR_ES_BANK_CONFLICTS  0x001010 /* Bank conflicts */
#define MSR_P5_CESR_ES_MISALIGNED      0x001011 /* Misaligned Memory or I/O */
#define MSR_P5_CESR_ES_CODE_READ       0x001100 /* Code Read */
#define MSR_P5_CESR_ES_CODE_TLB_MISS   0x001101 /* Code TLB miss */
#define MSR_P5_CESR_ES_CODE_CACHE_MISS 0x001110 /* Code Cache miss */
#define MSR_P5_CESR_ES_SEGMENT_LOADED  0x001111 /* Any segment reg. loaded */
#define MSR_P5_CESR_ES_BRANCHE         0x010010 /* Branches */
#define MSR_P5_CESR_ES_BTB_HIT         0x010011 /* BTB Hits */
#define MSR_P5_CESR_ES_BRANCHE_BTB     0x010100 /* Taken branch or BTB Hit */
#define MSR_P5_CESR_ES_PIPELINE_FLUSH  0x010101 /* Pipeline Flushes */
#define MSR_P5_CESR_ES_INSTRUCTION     0x010110 /* Instruction executed */
#define MSR_P5_CESR_ES_INSTRUCTION_V   0x010111 /* Inst. executed (v-pipe) */
#define MSR_P5_CESR_ES_BUS_CYCLE       0x011000 /* Clocks while bus cycle */
#define MSR_P5_CESR_ES_FULL_WRITE_BUF  0x011001 /* Clocks while full wrt buf. */
#define MSR_P5_CESR_ES_DATA_MEM_READ   0x011010 /* Pipeline waiting for read */
#define MSR_P5_CESR_ES_WRITE_EM        0x011011 /* Stall on write E|M state */
#define MSR_P5_CESR_ES_LOCKED_CYCLE    0x011100 /* Locked bus cycles */
#define MSR_P5_CESR_ES_IO_CYCLE        0x011101 /* I/O Read or Write cycles */
#define MSR_P5_CESR_ES_NON_CACHEABLE   0x011110 /* Non-cacheable Mem. read */
#define MSR_P5_CESR_ES_AGI             0x011111 /* Stall because of AGI */
#define MSR_P5_CESR_ES_FLOP            0x100010 /* Floating Point operations */
#define MSR_P5_CESR_ES_BREAK_DR0       0x100011 /* Breakpoint matches on DR0 */
#define MSR_P5_CESR_ES_BREAK_DR1       0x100100 /* Breakpoint matches on DR1 */
#define MSR_P5_CESR_ES_BREAK_DR2       0x100101 /* Breakpoint matches on DR2 */
#define MSR_P5_CESR_ES_BREAK_DR3       0x100110 /* Breakpoint matches on DR3 */
#define MSR_P5_CESR_ES_HARDWARE_IT     0x100111 /* Hardware interrupts */

/*
 * CR0
 */
#define CR0_PG  0x80000000      /*	 Enable paging */
#define CR0_CD  0x40000000      /* i486: Cache disable */
#define CR0_NW  0x20000000      /* i486: No write-through */
#define CR0_AM  0x00040000      /* i486: Alignment check mask */
#define CR0_WP  0x00010000      /* i486: Write-protect kernel access */
#define CR0_NE  0x00000020      /* i486: Handle numeric exceptions */
#define CR0_ET  0x00000010      /*	 Extension type is 80387 */
                                /*	 (not official) */
#define CR0_TS  0x00000008      /*	 Task switch */
#define CR0_EM  0x00000004      /*	 Emulate coprocessor */
#define CR0_MP  0x00000002      /*	 Monitor coprocessor */
#define CR0_PE  0x00000001      /*	 Enable protected mode */

/*
 * CR4
 */
#define CR4_SEE         0x00008000      /* Secure Enclave Enable XXX */
#define CR4_SMAP        0x00200000      /* Supervisor-Mode Access Protect */
#define CR4_SMEP        0x00100000      /* Supervisor-Mode Execute Protect */
#define CR4_OSXSAVE     0x00040000      /* OS supports XSAVE */
#define CR4_PCIDE       0x00020000      /* PCID Enable */
#define CR4_RDWRFSGS    0x00010000      /* RDWRFSGS Enable */
#define CR4_SMXE        0x00004000      /* Enable SMX operation */
#define CR4_VMXE        0x00002000      /* Enable VMX operation */
#define CR4_OSXMM       0x00000400      /* SSE/SSE2 exception support in OS */
#define CR4_OSFXS       0x00000200      /* SSE/SSE2 OS supports FXSave */
#define CR4_PCE         0x00000100      /* Performance-Monitor Count Enable */
#define CR4_PGE         0x00000080      /* Page Global Enable */
#define CR4_MCE         0x00000040      /* Machine Check Exceptions */
#define CR4_PAE         0x00000020      /* Physical Address Extensions */
#define CR4_PSE         0x00000010      /* Page Size Extensions */
#define CR4_DE          0x00000008      /* Debugging Extensions */
#define CR4_TSD         0x00000004      /* Time Stamp Disable */
#define CR4_PVI         0x00000002      /* Protected-mode Virtual Interrupts */
#define CR4_VME         0x00000001      /* Virtual-8086 Mode Extensions */

/*
 * XCR0 - XFEATURE_ENABLED_MASK (a.k.a. XFEM) register
 */
#define XCR0_X87        (1ULL << 0)     /* x87, FPU/MMX (always set) */
#define XCR0_SSE        (1ULL << 1)     /* SSE supported by XSAVE/XRESTORE */
#define XCR0_YMM        (1ULL << 2)     /* YMM state available */
#define XCR0_BNDREGS    (1ULL << 3)     /* MPX Bounds register state */
#define XCR0_BNDCSR     (1ULL << 4)     /* MPX Bounds configuration/state  */
#if !defined(RC_HIDE_XNU_J137)
#define XCR0_OPMASK     (1ULL << 5)     /* Opmask register state */
#define XCR0_ZMM_HI256  (1ULL << 6)     /* ZMM upper 256-bit state */
#define XCR0_HI16_ZMM   (1ULL << 7)     /* ZMM16..ZMM31 512-bit state */
#endif /* not RC_HIDE_XNU_J137 */
#define XFEM_X87        XCR0_X87
#define XFEM_SSE        XCR0_SSE
#define XFEM_YMM        XCR0_YMM
#define XFEM_BNDREGS    XCR0_BNDREGS
#define XFEM_BNDCSR     XCR0_BNDCSR
#if !defined(XNU_HODE_J137)
#define XFEM_OPMASK     XCR0_OPMASK
#define XFEM_ZMM_HI256  XCR0_ZMM_HI256
#define XFEM_HI16_ZMM   XCR0_HI16_ZMM
#define XFEM_ZMM        (XFEM_ZMM_HI256 | XFEM_HI16_ZMM | XFEM_OPMASK)
#endif /* not XNU_HODE_J137 */
#define XCR0 (0)

#define PMAP_PCID_PRESERVE (1ULL << 63)
#define PMAP_PCID_MASK (0xFFF)

#define EARLY_GSBASE_MAGIC 0xffffdeadbeefee00

/*
 * If thread groups are needed for x86, set this to 1
 */
#define CONFIG_THREAD_GROUPS 0

#ifndef ASSEMBLER

#include <sys/cdefs.h>
#include <stdint.h>

__BEGIN_DECLS

#define set_ts() set_cr0(get_cr0() | CR0_TS)

static inline uint16_t
get_es(void)
{
	uint16_t es;
	__asm__ volatile ("mov %%es, %0" : "=r" (es));
	return es;
}

static inline void
set_es(uint16_t es)
{
	__asm__ volatile ("mov %0, %%es" : : "r" (es));
}

static inline uint16_t
get_ds(void)
{
	uint16_t ds;
	__asm__ volatile ("mov %%ds, %0" : "=r" (ds));
	return ds;
}

static inline void
set_ds(uint16_t ds)
{
	__asm__ volatile ("mov %0, %%ds" : : "r" (ds));
}

static inline uint16_t
get_fs(void)
{
	uint16_t fs;
	__asm__ volatile ("mov %%fs, %0" : "=r" (fs));
	return fs;
}

static inline void
set_fs(uint16_t fs)
{
	__asm__ volatile ("mov %0, %%fs" : : "r" (fs));
}

static inline uint16_t
get_gs(void)
{
	uint16_t gs;
	__asm__ volatile ("mov %%gs, %0" : "=r" (gs));
	return gs;
}

static inline void
set_gs(uint16_t gs)
{
	__asm__ volatile ("mov %0, %%gs" : : "r" (gs));
}

static inline uint16_t
get_ss(void)
{
	uint16_t ss;
	__asm__ volatile ("mov %%ss, %0" : "=r" (ss));
	return ss;
}

static inline void
set_ss(uint16_t ss)
{
	__asm__ volatile ("mov %0, %%ss" : : "r" (ss));
}

static inline uintptr_t
get_cr0(void)
{
	uintptr_t cr0;
	__asm__ volatile ("mov %%cr0, %0" : "=r" (cr0));
	return cr0;
}

static inline void
set_cr0(uintptr_t value)
{
	__asm__ volatile ("mov %0, %%cr0" : : "r" (value));
}

static inline uintptr_t
get_cr2(void)
{
	uintptr_t cr2;
	__asm__ volatile ("mov %%cr2, %0" : "=r" (cr2));
	return cr2;
}

static inline uintptr_t
get_cr3_raw(void)
{
	uintptr_t cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r" (cr3));
	return cr3;
}

static inline void
set_cr3_raw(uintptr_t value)
{
	__asm__ volatile ("mov %0, %%cr3" : : "r" (value));
}

static inline uintptr_t
get_cr3_base(void)
{
	uintptr_t cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r" (cr3));
	return cr3 & ~(0xFFFULL);
}

static inline void
set_cr3_composed(uintptr_t base, uint16_t pcid, uint64_t preserve)
{
	__asm__ volatile ("mov %0, %%cr3" : : "r" (base | pcid | ( (preserve) << 63) ));
}

static inline uintptr_t
get_cr4(void)
{
	uintptr_t cr4;
	__asm__ volatile ("mov %%cr4, %0" : "=r" (cr4));
	return cr4;
}

static inline void
set_cr4(uintptr_t value)
{
	__asm__ volatile ("mov %0, %%cr4" : : "r" (value));
}

static inline uintptr_t
x86_get_flags(void)
{
	uintptr_t erflags;
	__asm__ volatile ("pushf; pop	%0"  :  "=r" (erflags));
	return erflags;
}

static inline void
clear_ts(void)
{
	__asm__ volatile ("clts");
}

static inline unsigned short
get_tr(void)
{
	unsigned short seg;
	__asm__ volatile ("str %0" : "=rm" (seg));
	return seg;
}

static inline void
set_tr(unsigned int seg)
{
	__asm__ volatile ("ltr %0" : : "rm" ((unsigned short)(seg)));
}

static inline unsigned short
sldt(void)
{
	unsigned short seg;
	__asm__ volatile ("sldt %0" : "=rm" (seg));
	return seg;
}

static inline void
lldt(unsigned int seg)
{
	__asm__ volatile ("lldt %0" : : "rm" ((unsigned short)(seg)));
}

static inline void
lgdt(uintptr_t *desc)
{
	__asm__ volatile ("lgdt %0" : : "m" (*desc));
}

static inline void
lidt(uintptr_t *desc)
{
	__asm__ volatile ("lidt %0" : : "m" (*desc));
}

static inline void
swapgs(void)
{
	__asm__ volatile ("swapgs");
}

static inline void
hlt(void)
{
	__asm__ volatile ("hlt");
}

#ifdef MACH_KERNEL_PRIVATE

extern int rdmsr64_carefully(uint32_t msr, uint64_t *val);
extern int wrmsr64_carefully(uint32_t msr, uint64_t val);
#endif  /* MACH_KERNEL_PRIVATE */

static inline void
wbinvd(void)
{
	__asm__ volatile ("wbinvd");
}

static inline void
invlpg(uintptr_t addr)
{
	__asm__  volatile ("invlpg (%0)" :: "r" (addr) : "memory");
}

static inline void
clac(void)
{
	__asm__  volatile ("clac");
}

static inline void
stac(void)
{
	__asm__  volatile ("stac");
}

/*
 * Access to machine-specific registers (available on 586 and better only)
 * Note: the rd* operations modify the parameters directly (without using
 * pointer indirection), this allows gcc to optimize better
 */

#define rdmsr(msr, lo, hi) \
	__asm__ volatile("rdmsr" : "=a" (lo), "=d" (hi) : "c" (msr))

#define wrmsr(msr, lo, hi) \
	__asm__ volatile("wrmsr" : : "c" (msr), "a" (lo), "d" (hi))

#define rdtsc(lo, hi) \
	__asm__ volatile("lfence; rdtsc" : "=a" (lo), "=d" (hi))

#define rdtsc_nofence(lo, hi) \
	__asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi))

#define write_tsc(lo, hi) wrmsr(0x10, lo, hi)

#define rdpmc(counter, lo, hi) \
	__asm__ volatile("rdpmc" : "=a" (lo), "=d" (hi) : "c" (counter))

#ifdef XNU_KERNEL_PRIVATE
extern void do_mfence(void);
#define mfence() do_mfence()
#endif

#ifdef __LP64__
static inline uint64_t
rdpmc64(uint32_t pmc)
{
	uint32_t lo = 0, hi = 0;
	rdpmc(pmc, lo, hi);
	return (((uint64_t)hi) << 32) | ((uint64_t)lo);
}

static inline uint64_t
rdmsr64(uint32_t msr)
{
	uint32_t lo = 0, hi = 0;
	rdmsr(msr, lo, hi);
	return (((uint64_t)hi) << 32) | ((uint64_t)lo);
}

static inline void
wrmsr64(uint32_t msr, uint64_t val)
{
	wrmsr(msr, (val & 0xFFFFFFFFUL), ((val >> 32) & 0xFFFFFFFFUL));
}

static inline uint64_t
rdtsc64(void)
{
	uint64_t lo, hi;
	rdtsc(lo, hi);
	return ((hi) << 32) | (lo);
}

static inline uint64_t
rdtscp64(uint32_t *aux)
{
	uint64_t lo, hi;
	__asm__ volatile ("rdtscp; mov %%ecx, %1"
                                          : "=a" (lo), "=d" (hi), "=m" (*aux)
                                          :
                                          : "ecx");
	return ((hi) << 32) | (lo);
}
#endif /* __LP64__ */

/*
 * rdmsr_carefully() returns 0 when the MSR has been read successfully,
 * or non-zero (1) if the MSR does not exist.
 * The implementation is in locore.s.
 */
extern int rdmsr_carefully(uint32_t msr, uint32_t *lo, uint32_t *hi);
__END_DECLS

#endif  /* ASSEMBLER */

#define MSR_IA32_P5_MC_ADDR                     0
#define MSR_IA32_P5_MC_TYPE                     1
#define MSR_IA32_PLATFORM_ID                    0x17
#define MSR_IA32_EBL_CR_POWERON                 0x2a

#define MSR_IA32_APIC_BASE                      0x1b
#define     MSR_IA32_APIC_BASE_BSP                  (1<<8)
#define     MSR_IA32_APIC_BASE_EXTENDED             (1<<10)
#define     MSR_IA32_APIC_BASE_ENABLE               (1<<11)
#define     MSR_IA32_APIC_BASE_BASE                 (0xfffff<<12)

#define MSR_CORE_THREAD_COUNT                   0x35

#define MSR_IA32_FEATURE_CONTROL                0x3a
#define     MSR_IA32_FEATCTL_LOCK                   (1<<0)
#define     MSR_IA32_FEATCTL_VMXON_SMX              (1<<1)
#define     MSR_IA32_FEATCTL_VMXON                  (1<<2)
#define     MSR_IA32_FEATCTL_CSTATE_SMI             (1<<16)

#define MSR_IA32_UPDT_TRIG                      0x79
#define MSR_IA32_BIOS_SIGN_ID                   0x8b
#define MSR_IA32_UCODE_WRITE                    MSR_IA32_UPDT_TRIG
#define MSR_IA32_UCODE_REV                      MSR_IA32_BIOS_SIGN_ID

#define MSR_IA32_PERFCTR0                       0xc1
#define MSR_IA32_PERFCTR1                       0xc2
#define MSR_IA32_PERFCTR3                       0xc3
#define MSR_IA32_PERFCTR4                       0xc4

#define MSR_PLATFORM_INFO                       0xce

#define MSR_IA32_MPERF                          0xE7
#define MSR_IA32_APERF                          0xE8

#define MSR_IA32_ARCH_CAPABILITIES              0x10a
#define         MSR_IA32_ARCH_CAPABILITIES_RDCL_NO      (1ULL << 0)
#define         MSR_IA32_ARCH_CAPABILITIES_IBRS_ALL     (1ULL << 1)
#define         MSR_IA32_ARCH_CAPABILITIES_RSBA         (1ULL << 2)
#define         MSR_IA32_ARCH_CAPABILITIES_L1DF_NO      (1ULL << 3)
#define         MSR_IA32_ARCH_CAPABILITIES_SSB_NO       (1ULL << 4)
#define         MSR_IA32_ARCH_CAPABILITIES_MDS_NO       (1ULL << 5)

#define MSR_IA32_TSX_FORCE_ABORT                0x10f
#define         MSR_IA32_TSXFA_RTM_FORCE_ABORT  (1ULL << 0)     /* Bit 0 */

#define MSR_IA32_BBL_CR_CTL                     0x119


#define MSR_IA32_SYSENTER_CS                    0x174
#define MSR_IA32_SYSENTER_ESP                   0x175
#define MSR_IA32_SYSENTER_EIP                   0x176

#define MSR_IA32_MCG_CAP                        0x179
#define MSR_IA32_MCG_STATUS                     0x17a
#define MSR_IA32_MCG_CTL                        0x17b

#define MSR_IA32_EVNTSEL0                       0x186
#define MSR_IA32_EVNTSEL1                       0x187
#define MSR_IA32_EVNTSEL2                       0x188
#define MSR_IA32_EVNTSEL3                       0x189

#define MSR_FLEX_RATIO                          0x194
#define MSR_IA32_PERF_STS                       0x198
#define MSR_IA32_PERF_CTL                       0x199
#define MSR_IA32_CLOCK_MODULATION               0x19a

#define MSR_IA32_MISC_ENABLE                    0x1a0


#define MSR_IA32_PACKAGE_THERM_STATUS           0x1b1
#define MSR_IA32_PACKAGE_THERM_INTERRUPT        0x1b2

#define MSR_IA32_DEBUGCTLMSR                    0x1d9
#define MSR_IA32_LASTBRANCHFROMIP               0x1db
#define MSR_IA32_LASTBRANCHTOIP                 0x1dc
#define MSR_IA32_LASTINTFROMIP                  0x1dd
#define MSR_IA32_LASTINTTOIP                    0x1de

#define MSR_IA32_CR_PAT                         0x277

#define MSR_IA32_MTRRCAP                        0xfe
#define MSR_IA32_MTRR_DEF_TYPE                  0x2ff
#define MSR_IA32_MTRR_PHYSBASE(n)               (0x200 + 2*(n))
#define MSR_IA32_MTRR_PHYSMASK(n)               (0x200 + 2*(n) + 1)
#define MSR_IA32_MTRR_FIX64K_00000              0x250
#define MSR_IA32_MTRR_FIX16K_80000              0x258
#define MSR_IA32_MTRR_FIX16K_A0000              0x259
#define MSR_IA32_MTRR_FIX4K_C0000               0x268
#define MSR_IA32_MTRR_FIX4K_C8000               0x269
#define MSR_IA32_MTRR_FIX4K_D0000               0x26a
#define MSR_IA32_MTRR_FIX4K_D8000               0x26b
#define MSR_IA32_MTRR_FIX4K_E0000               0x26c
#define MSR_IA32_MTRR_FIX4K_E8000               0x26d
#define MSR_IA32_MTRR_FIX4K_F0000               0x26e
#define MSR_IA32_MTRR_FIX4K_F8000               0x26f

#define MSR_IA32_PERF_FIXED_CTR0                0x309

#define MSR_IA32_PERF_FIXED_CTR_CTRL            0x38D
#define MSR_IA32_PERF_GLOBAL_STATUS             0x38E
#define MSR_IA32_PERF_GLOBAL_CTRL               0x38F
#define MSR_IA32_PERF_GLOBAL_OVF_CTRL   0x390

#define MSR_IA32_PKG_C3_RESIDENCY               0x3F8
#define MSR_IA32_PKG_C6_RESIDENCY               0x3F9
#define MSR_IA32_PKG_C7_RESIDENCY               0x3FA

#define MSR_IA32_CORE_C3_RESIDENCY              0x3FC
#define MSR_IA32_CORE_C6_RESIDENCY              0x3FD
#define MSR_IA32_CORE_C7_RESIDENCY              0x3FE

#define MSR_IA32_MC0_CTL                        0x400
#define MSR_IA32_MC0_STATUS                     0x401
#define MSR_IA32_MC0_ADDR                       0x402
#define MSR_IA32_MC0_MISC                       0x403

#define MSR_IA32_VMX_BASE                                       0x480
#define MSR_IA32_VMX_BASIC                                      MSR_IA32_VMX_BASE
#define MSR_IA32_VMX_PINBASED_CTLS                      MSR_IA32_VMX_BASE+1
#define MSR_IA32_VMX_PROCBASED_CTLS                     MSR_IA32_VMX_BASE+2
#define MSR_IA32_VMX_EXIT_CTLS                          MSR_IA32_VMX_BASE+3
#define MSR_IA32_VMX_ENTRY_CTLS                         MSR_IA32_VMX_BASE+4
#define MSR_IA32_VMX_MISC                                       MSR_IA32_VMX_BASE+5
#define MSR_IA32_VMX_CR0_FIXED0                         MSR_IA32_VMX_BASE+6
#define MSR_IA32_VMX_CR0_FIXED1                         MSR_IA32_VMX_BASE+7
#define MSR_IA32_VMX_CR4_FIXED0                         MSR_IA32_VMX_BASE+8
#define MSR_IA32_VMX_CR4_FIXED1                         MSR_IA32_VMX_BASE+9
#define MSR_IA32_VMX_VMCS_ENUM                          MSR_IA32_VMX_BASE+10
#define MSR_IA32_VMX_PROCBASED_CTLS2            MSR_IA32_VMX_BASE+11
#define MSR_IA32_VMX_EPT_VPID_CAP                       MSR_IA32_VMX_BASE+12
#define         MSR_IA32_VMX_EPT_VPID_CAP_AD_SHIFT      21
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS         MSR_IA32_VMX_BASE+13
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS        MSR_IA32_VMX_BASE+14
#define MSR_IA32_VMX_TRUE_VMEXIT_CTLS           MSR_IA32_VMX_BASE+15
#define MSR_IA32_VMX_TRUE_VMENTRY_CTLS          MSR_IA32_VMX_BASE+16
#define MSR_IA32_VMX_VMFUNC                                     MSR_IA32_VMX_BASE+17

#define MSR_IA32_DS_AREA                        0x600

#define MSR_IA32_PKG_POWER_SKU_UNIT             0x606
#define MSR_IA32_PKG_C2_RESIDENCY               0x60D
#define MSR_IA32_PKG_ENERGY_STATUS              0x611
#define MSR_IA32_DDR_ENERGY_STATUS              0x619
#define MSR_IA32_LLC_FLUSHED_RESIDENCY_TIMER    0x61D
#define MSR_IA32_RING_PERF_STATUS               0x621

#define MSR_IA32_PKG_C8_RESIDENCY               0x630
#define MSR_IA32_PKG_C9_RESIDENCY               0x631
#define MSR_IA32_PKG_C10_RESIDENCY              0x632

#define MSR_IA32_PP0_ENERGY_STATUS              0x639
#define MSR_IA32_PP1_ENERGY_STATUS              0x641
#define MSR_IA32_IA_PERF_LIMIT_REASONS_SKL      0x64F

#define MSR_IA32_IA_PERF_LIMIT_REASONS          0x690
#define MSR_IA32_GT_PERF_LIMIT_REASONS          0x6B0

#define MSR_IA32_TSC_DEADLINE                   0x6e0

#define MSR_IA32_EFER                           0xC0000080
#define     MSR_IA32_EFER_SCE                       0x00000001
#define     MSR_IA32_EFER_LME                       0x00000100
#define     MSR_IA32_EFER_LMA                       0x00000400
#define     MSR_IA32_EFER_NXE                       0x00000800

#define MSR_IA32_STAR                           0xC0000081
#define MSR_IA32_LSTAR                          0xC0000082
#define MSR_IA32_CSTAR                          0xC0000083
#define MSR_IA32_FMASK                          0xC0000084

#define MSR_IA32_FS_BASE                        0xC0000100
#define MSR_IA32_GS_BASE                        0xC0000101
#define MSR_IA32_KERNEL_GS_BASE                 0xC0000102
#define MSR_IA32_TSC_AUX                        0xC0000103

#define HV_VMX_EPTP_MEMORY_TYPE_UC              0x0
#define HV_VMX_EPTP_MEMORY_TYPE_WB              0x6
#define HV_VMX_EPTP_WALK_LENGTH(wl)             (0ULL | ((((wl) - 1) & 0x7) << 3))
#define HV_VMX_EPTP_ENABLE_AD_FLAGS             (1ULL << 6)

#endif  /* _I386_PROC_REG_H_ */
