/*
 * Copyright (c) 2000-2019 Apple Computer, Inc. All rights reserved.
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
 * x86 CPU identification
 *
 */

#ifndef _MACHINE_CPUID_H_
#define _MACHINE_CPUID_H_

#include <sys/appleapiopts.h>

#if defined(MACH_KERNEL_PRIVATE) && !defined(ASSEMBLER)
#include <i386/hw_defs.h>
#include <i386/pio.h>
#include <i386/machine_routines.h>
#endif

#ifdef __APPLE_API_PRIVATE

#define CPUID_VID_INTEL         "GenuineIntel"
#define CPUID_VID_AMD           "AuthenticAMD"

#define CPUID_VMM_ID_VMWARE             "VMwareVMware"
#define CPUID_VMM_ID_PARALLELS  "Parallels\0\0\0"

#define CPUID_STRING_UNKNOWN    "Unknown CPU Typ"

#define _Bit(n)                 (1ULL << n)
#define _HBit(n)                (1ULL << ((n)+32))

/*
 * The CPUID_FEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 1:
 */
#define CPUID_FEATURE_FPU       _Bit(0)   /* Floating point unit on-chip */
#define CPUID_FEATURE_VME       _Bit(1)   /* Virtual Mode Extension */
#define CPUID_FEATURE_DE        _Bit(2)   /* Debugging Extension */
#define CPUID_FEATURE_PSE       _Bit(3)   /* Page Size Extension */
#define CPUID_FEATURE_TSC       _Bit(4)   /* Time Stamp Counter */
#define CPUID_FEATURE_MSR       _Bit(5)   /* Model Specific Registers */
#define CPUID_FEATURE_PAE       _Bit(6)   /* Physical Address Extension */
#define CPUID_FEATURE_MCE       _Bit(7)   /* Machine Check Exception */
#define CPUID_FEATURE_CX8       _Bit(8)   /* CMPXCHG8B */
#define CPUID_FEATURE_APIC      _Bit(9)   /* On-chip APIC */
#define CPUID_FEATURE_SEP       _Bit(11)  /* Fast System Call */
#define CPUID_FEATURE_MTRR      _Bit(12)  /* Memory Type Range Register */
#define CPUID_FEATURE_PGE       _Bit(13)  /* Page Global Enable */
#define CPUID_FEATURE_MCA       _Bit(14)  /* Machine Check Architecture */
#define CPUID_FEATURE_CMOV      _Bit(15)  /* Conditional Move Instruction */
#define CPUID_FEATURE_PAT       _Bit(16)  /* Page Attribute Table */
#define CPUID_FEATURE_PSE36     _Bit(17)  /* 36-bit Page Size Extension */
#define CPUID_FEATURE_PSN       _Bit(18)  /* Processor Serial Number */
#define CPUID_FEATURE_CLFSH     _Bit(19)  /* CLFLUSH Instruction supported */
#define CPUID_FEATURE_DS        _Bit(21)  /* Debug Store */
#define CPUID_FEATURE_ACPI      _Bit(22)  /* Thermal monitor and Clock Ctrl */
#define CPUID_FEATURE_MMX       _Bit(23)  /* MMX supported */
#define CPUID_FEATURE_FXSR      _Bit(24)  /* Fast floating pt save/restore */
#define CPUID_FEATURE_SSE       _Bit(25)  /* Streaming SIMD extensions */
#define CPUID_FEATURE_SSE2      _Bit(26)  /* Streaming SIMD extensions 2 */
#define CPUID_FEATURE_SS        _Bit(27)  /* Self-Snoop */
#define CPUID_FEATURE_HTT       _Bit(28)  /* Hyper-Threading Technology */
#define CPUID_FEATURE_TM        _Bit(29)  /* Thermal Monitor (TM1) */
#define CPUID_FEATURE_PBE       _Bit(31)  /* Pend Break Enable */

#define CPUID_FEATURE_SSE3      _HBit(0)  /* Streaming SIMD extensions 3 */
#define CPUID_FEATURE_PCLMULQDQ _HBit(1)  /* PCLMULQDQ instruction */
#define CPUID_FEATURE_DTES64    _HBit(2)  /* 64-bit DS layout */
#define CPUID_FEATURE_MONITOR   _HBit(3)  /* Monitor/mwait */
#define CPUID_FEATURE_DSCPL     _HBit(4)  /* Debug Store CPL */
#define CPUID_FEATURE_VMX       _HBit(5)  /* VMX */
#define CPUID_FEATURE_SMX       _HBit(6)  /* SMX */
#define CPUID_FEATURE_EST       _HBit(7)  /* Enhanced SpeedsTep (GV3) */
#define CPUID_FEATURE_TM2       _HBit(8)  /* Thermal Monitor 2 */
#define CPUID_FEATURE_SSSE3     _HBit(9)  /* Supplemental SSE3 instructions */
#define CPUID_FEATURE_CID       _HBit(10) /* L1 Context ID */
#define CPUID_FEATURE_SEGLIM64  _HBit(11) /* 64-bit segment limit checking */
#define CPUID_FEATURE_FMA       _HBit(12) /* Fused-Multiply-Add support */
#define CPUID_FEATURE_CX16      _HBit(13) /* CmpXchg16b instruction */
#define CPUID_FEATURE_xTPR      _HBit(14) /* Send Task PRiority msgs */
#define CPUID_FEATURE_PDCM      _HBit(15) /* Perf/Debug Capability MSR */

#define CPUID_FEATURE_PCID      _HBit(17) /* ASID-PCID support */
#define CPUID_FEATURE_DCA       _HBit(18) /* Direct Cache Access */
#define CPUID_FEATURE_SSE4_1    _HBit(19) /* Streaming SIMD extensions 4.1 */
#define CPUID_FEATURE_SSE4_2    _HBit(20) /* Streaming SIMD extensions 4.2 */
#define CPUID_FEATURE_x2APIC    _HBit(21) /* Extended APIC Mode */
#define CPUID_FEATURE_MOVBE     _HBit(22) /* MOVBE instruction */
#define CPUID_FEATURE_POPCNT    _HBit(23) /* POPCNT instruction */
#define CPUID_FEATURE_TSCTMR    _HBit(24) /* TSC deadline timer */
#define CPUID_FEATURE_AES       _HBit(25) /* AES instructions */
#define CPUID_FEATURE_XSAVE     _HBit(26) /* XSAVE instructions */
#define CPUID_FEATURE_OSXSAVE   _HBit(27) /* XGETBV/XSETBV instructions */
#define CPUID_FEATURE_AVX1_0    _HBit(28) /* AVX 1.0 instructions */
#define CPUID_FEATURE_F16C      _HBit(29) /* Float16 convert instructions */
#define CPUID_FEATURE_RDRAND    _HBit(30) /* RDRAND instruction */
#define CPUID_FEATURE_VMM       _HBit(31) /* VMM (Hypervisor) present */

/*
 * Leaf 7, subleaf 0 additional features.
 * Bits returned in %ebx:%ecx to a CPUID request with {%eax,%ecx} of (0x7,0x0}:
 */
#define CPUID_LEAF7_FEATURE_RDWRFSGS   _Bit(0)    /* FS/GS base read/write */
#define CPUID_LEAF7_FEATURE_TSCOFF     _Bit(1)    /* TSC thread offset */
#define CPUID_LEAF7_FEATURE_SGX        _Bit(2)    /* Software Guard eXtensions */
#define CPUID_LEAF7_FEATURE_BMI1       _Bit(3)    /* Bit Manipulation Instrs, set 1 */
#define CPUID_LEAF7_FEATURE_HLE        _Bit(4)    /* Hardware Lock Elision*/
#define CPUID_LEAF7_FEATURE_AVX2       _Bit(5)    /* AVX2 Instructions */
#define CPUID_LEAF7_FEATURE_FDPEO      _Bit(6)    /* x87 FPU Data Pointer updated only on x87 exceptions */
#define CPUID_LEAF7_FEATURE_SMEP       _Bit(7)    /* Supervisor Mode Execute Protect */
#define CPUID_LEAF7_FEATURE_BMI2       _Bit(8)    /* Bit Manipulation Instrs, set 2 */
#define CPUID_LEAF7_FEATURE_ERMS       _Bit(9)    /* Enhanced Rep Movsb/Stosb */
#define CPUID_LEAF7_FEATURE_INVPCID    _Bit(10)   /* INVPCID intruction, TDB */
#define CPUID_LEAF7_FEATURE_RTM        _Bit(11)   /* RTM */
#define CPUID_LEAF7_FEATURE_PQM        _Bit(12)   /* Platform Qos Monitoring */
#define CPUID_LEAF7_FEATURE_FPU_CSDS   _Bit(13)   /* FPU CS/DS deprecation */
#define CPUID_LEAF7_FEATURE_MPX        _Bit(14)   /* Memory Protection eXtensions */
#define CPUID_LEAF7_FEATURE_PQE        _Bit(15)   /* Platform Qos Enforcement */
#define CPUID_LEAF7_FEATURE_AVX512F    _Bit(16)   /* AVX512F instructions */
#define CPUID_LEAF7_FEATURE_AVX512DQ   _Bit(17)   /* AVX512DQ instructions */
#define CPUID_LEAF7_FEATURE_RDSEED     _Bit(18)   /* RDSEED Instruction */
#define CPUID_LEAF7_FEATURE_ADX        _Bit(19)   /* ADX Instructions */
#define CPUID_LEAF7_FEATURE_SMAP       _Bit(20)   /* Supervisor Mode Access Protect */
#define CPUID_LEAF7_FEATURE_AVX512IFMA _Bit(21)   /* AVX512IFMA instructions */
#define CPUID_LEAF7_FEATURE_CLFSOPT    _Bit(23)   /* CLFSOPT */
#define CPUID_LEAF7_FEATURE_CLWB       _Bit(24)   /* CLWB */
#define CPUID_LEAF7_FEATURE_IPT        _Bit(25)   /* Intel Processor Trace */
#define CPUID_LEAF7_FEATURE_AVX512CD   _Bit(28)   /* AVX512CD instructions */
#define CPUID_LEAF7_FEATURE_SHA        _Bit(29)   /* SHA instructions */
#define CPUID_LEAF7_FEATURE_AVX512BW   _Bit(30)   /* AVX512BW instructions */
#define CPUID_LEAF7_FEATURE_AVX512VL   _Bit(31)   /* AVX512VL instructions */

#define CPUID_LEAF7_FEATURE_PREFETCHWT1  _HBit(0)  /* Prefetch Write/T1 hint */
#define CPUID_LEAF7_FEATURE_AVX512VBMI   _HBit(1)  /* AVX512VBMI instructions */
#define CPUID_LEAF7_FEATURE_UMIP         _HBit(2)  /* User Mode Instruction Prevention */
#define CPUID_LEAF7_FEATURE_PKU          _HBit(3)  /* Protection Keys for Usermode */
#define CPUID_LEAF7_FEATURE_OSPKE        _HBit(4)  /* OS has enabled PKE */
#define CPUID_LEAF7_FEATURE_WAITPKG      _HBit(5)  /* WAITPKG instructions */
#define CPUID_LEAF7_FEATURE_GFNI         _HBit(8)  /* Galois Field New Instructions */
#define CPUID_LEAF7_FEATURE_VAES         _HBit(9)  /* Vector-encoded AES */
#define CPUID_LEAF7_FEATURE_VPCLMULQDQ   _HBit(10) /* Vector Carryless-multiply */
#define CPUID_LEAF7_FEATURE_AVX512VNNI   _HBit(11) /* AVX512 Vector Neural Net Instructions */
#define CPUID_LEAF7_FEATURE_AVX512BITALG _HBit(12) /* AVX512 VPOPCNT{B,W} and VPSHUFBITQMB */
#define CPUID_LEAF7_FEATURE_AVX512VPCDQ  _HBit(14) /* AVX512 VPOPCNTDQ instruction */
#define CPUID_LEAF7_FEATURE_RDPID        _HBit(22) /* RDPID and IA32_TSC_AUX */
#define CPUID_LEAF7_FEATURE_CLDEMOTE     _HBit(25) /* Cache line demote */
#define CPUID_LEAF7_FEATURE_MOVDIRI      _HBit(27) /* MOVDIRI instruction */
#define CPUID_LEAF7_FEATURE_MOVDIRI64B   _HBit(28) /* MOVDIRI64B instruction */
#define CPUID_LEAF7_FEATURE_SGXLC        _HBit(30) /* SGX Launch Configuration */

/*
 * Values in EDX returned by CPUID Leaf 7, subleaf 0
 */
#define CPUID_LEAF7_EXTFEATURE_AVX5124VNNIW     _Bit(2)         /* AVX512_4VNNIW */
#define CPUID_LEAF7_EXTFEATURE_AVX5124FMAPS     _Bit(3)         /* AVX512_4FMAPS */
#define CPUID_LEAF7_EXTFEATURE_FSREPMOV         _Bit(4)         /* Fast Short REP MOV */
#define CPUID_LEAF7_EXTFEATURE_MDCLEAR          _Bit(10)        /* Overloaded VERW / L1D_FLUSH */
#define CPUID_LEAF7_EXTFEATURE_TSXFA            _Bit(13)        /* TSX RTM_FORCE_ABORT MSR */
#define CPUID_LEAF7_EXTFEATURE_IBRS             _Bit(26)        /* IBRS / IBPB */
#define CPUID_LEAF7_EXTFEATURE_STIBP            _Bit(27)        /* Single Thread Indirect Branch Predictors */
#define CPUID_LEAF7_EXTFEATURE_L1DF             _Bit(28)        /* L1D_FLUSH MSR */
#define CPUID_LEAF7_EXTFEATURE_ACAPMSR          _Bit(29)        /* ARCH_CAP MSR */
#define CPUID_LEAF7_EXTFEATURE_CCAPMSR          _Bit(30)        /* CORE_CAP MSR */
#define CPUID_LEAF7_EXTFEATURE_SSBD             _Bit(31)        /* Speculative Store Bypass Disable */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000001:
 */
#define CPUID_EXTFEATURE_SYSCALL   _Bit(11)     /* SYSCALL/sysret */
#define CPUID_EXTFEATURE_XD        _Bit(20)     /* eXecute Disable */

#define CPUID_EXTFEATURE_1GBPAGE   _Bit(26)     /* 1GB pages */
#define CPUID_EXTFEATURE_RDTSCP    _Bit(27)     /* RDTSCP */
#define CPUID_EXTFEATURE_EM64T     _Bit(29)     /* Extended Mem 64 Technology */

#define CPUID_EXTFEATURE_LAHF      _HBit(0)     /* LAFH/SAHF instructions */
#define CPUID_EXTFEATURE_LZCNT     _HBit(5)     /* LZCNT instruction */
#define CPUID_EXTFEATURE_PREFETCHW _HBit(8)     /* PREFETCHW instruction */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000007:
 */
#define CPUID_EXTFEATURE_TSCI      _Bit(8)      /* TSC Invariant */

/*
 * CPUID_X86_64_H_FEATURE_SUBSET and CPUID_X86_64_H_LEAF7_FEATURE_SUBSET
 * indicate the bitmask of features that must be present before the system
 * is eligible to run the "x86_64h" "Haswell feature subset" slice.
 */
#define CPUID_X86_64_H_FEATURE_SUBSET ( CPUID_FEATURE_FMA    | \
	                                CPUID_FEATURE_SSE4_2 | \
	                                CPUID_FEATURE_MOVBE  | \
	                                CPUID_FEATURE_POPCNT | \
	                                CPUID_FEATURE_AVX1_0   \
	                              )

#define CPUID_X86_64_H_EXTFEATURE_SUBSET ( CPUID_EXTFEATURE_LZCNT \
	                                 )

#define CPUID_X86_64_H_LEAF7_FEATURE_SUBSET ( CPUID_LEAF7_FEATURE_BMI1 | \
	                                      CPUID_LEAF7_FEATURE_AVX2 | \
	                                      CPUID_LEAF7_FEATURE_BMI2   \
	                                    )

#define CPUID_CACHE_SIZE        16      /* Number of descriptor values */

#define CPUID_MWAIT_EXTENSION   _Bit(0) /* enumeration of WMAIT extensions */
#define CPUID_MWAIT_BREAK       _Bit(1) /* interrupts are break events	   */

#define CPUID_MODEL_PENRYN              0x17
#define CPUID_MODEL_NEHALEM             0x1A
#define CPUID_MODEL_FIELDS              0x1E    /* Lynnfield, Clarksfield */
#define CPUID_MODEL_DALES               0x1F    /* Havendale, Auburndale */
#define CPUID_MODEL_NEHALEM_EX          0x2E
#define CPUID_MODEL_DALES_32NM          0x25    /* Clarkdale, Arrandale */
#define CPUID_MODEL_WESTMERE            0x2C    /* Gulftown, Westmere-EP/-WS */
#define CPUID_MODEL_WESTMERE_EX         0x2F
#define CPUID_MODEL_SANDYBRIDGE         0x2A
#define CPUID_MODEL_JAKETOWN            0x2D
#define CPUID_MODEL_IVYBRIDGE           0x3A
#define CPUID_MODEL_IVYBRIDGE_EP        0x3E
#define CPUID_MODEL_CRYSTALWELL         0x46
#define CPUID_MODEL_HASWELL             0x3C
#define CPUID_MODEL_HASWELL_EP          0x3F
#define CPUID_MODEL_HASWELL_ULT         0x45
#define CPUID_MODEL_BROADWELL           0x3D
#define CPUID_MODEL_BROADWELL_ULX       0x3D
#define CPUID_MODEL_BROADWELL_ULT       0x3D
#define CPUID_MODEL_BRYSTALWELL         0x47
#define CPUID_MODEL_SKYLAKE             0x4E
#define CPUID_MODEL_SKYLAKE_ULT         0x4E
#define CPUID_MODEL_SKYLAKE_ULX         0x4E
#define CPUID_MODEL_SKYLAKE_DT          0x5E
#if !defined(RC_HIDE_XNU_J137)
#define CPUID_MODEL_SKYLAKE_W           0x55
#define PLATID_XEON_SP_1                0x00
#define PLATID_XEON_SP_2                0x07
#define PLATID_MAYBE_XEON_SP            0x01
#endif /* not RC_HIDE_XNU_J137 */
#define CPUID_MODEL_KABYLAKE            0x8E
#define CPUID_MODEL_KABYLAKE_ULT        0x8E
#define CPUID_MODEL_KABYLAKE_ULX        0x8E
#define CPUID_MODEL_KABYLAKE_DT         0x9E

#define CPUID_VMM_FAMILY_UNKNOWN        0x0
#define CPUID_VMM_FAMILY_VMWARE         0x1
#define CPUID_VMM_FAMILY_PARALLELS      0x2

#ifndef ASSEMBLER
#include <stdint.h>
#include <mach/mach_types.h>
#include <kern/kern_types.h>
#include <mach/machine.h>


typedef enum { eax, ebx, ecx, edx } cpuid_register_t;
static inline void
cpuid(uint32_t *data)
{
	__asm__ volatile ("cpuid"
                : "=a" (data[eax]),
                  "=b" (data[ebx]),
                  "=c" (data[ecx]),
                  "=d" (data[edx])
                : "a"  (data[eax]),
                  "b"  (data[ebx]),
                  "c"  (data[ecx]),
                  "d"  (data[edx]));
}

static inline void
do_cpuid(uint32_t selector, uint32_t *data)
{
	__asm__ volatile ("cpuid"
                : "=a" (data[0]),
                  "=b" (data[1]),
                  "=c" (data[2]),
                  "=d" (data[3])
                : "a"(selector),
                  "b" (0),
                  "c" (0),
                  "d" (0));
}

/*
 * Cache ID descriptor structure, used to parse CPUID leaf 2.
 * Note: not used in kernel.
 */
typedef enum { Lnone, L1I, L1D, L2U, L3U, LCACHE_MAX } cache_type_t;
typedef struct {
	unsigned char   value;          /* Descriptor value */
	cache_type_t    type;           /* Cache type */
	unsigned int    size;           /* Cache size */
	unsigned int    linesize;       /* Cache line size */
#ifdef KERNEL
	const char      *description;   /* Cache description */
#endif /* KERNEL */
} cpuid_cache_desc_t;

#ifdef KERNEL
#define CACHE_DESC(value, type, size, linesize, text) \
	{ value, type, size, linesize, text }
#else
#define CACHE_DESC(value, type, size, linesize, text) \
	{ value, type, size, linesize }
#endif /* KERNEL */

/* Monitor/mwait Leaf: */
typedef struct {
	uint32_t        linesize_min;
	uint32_t        linesize_max;
	uint32_t        extensions;
	uint32_t        sub_Cstates;
} cpuid_mwait_leaf_t;

/* Thermal and Power Management Leaf: */
typedef struct {
	boolean_t       sensor;
	boolean_t       dynamic_acceleration;
	boolean_t       invariant_APIC_timer;
	boolean_t       core_power_limits;
	boolean_t       fine_grain_clock_mod;
	boolean_t       package_thermal_intr;
	uint32_t        thresholds;
	boolean_t       ACNT_MCNT;
	boolean_t       hardware_feedback;
	boolean_t       energy_policy;
} cpuid_thermal_leaf_t;


/* XSAVE Feature Leaf: */
typedef struct {
	uint32_t        extended_state[4];      /* eax .. edx */
} cpuid_xsave_leaf_t;


/* Architectural Performance Monitoring Leaf: */
typedef struct {
	uint8_t         version;
	uint8_t         number;
	uint8_t         width;
	uint8_t         events_number;
	uint32_t        events;
	uint8_t         fixed_number;
	uint8_t         fixed_width;
} cpuid_arch_perf_leaf_t;

/* The TSC to Core Crystal (RefCLK) Clock Information leaf */
typedef struct {
	uint32_t        numerator;
	uint32_t        denominator;
} cpuid_tsc_leaf_t;

/* Physical CPU info - this is exported out of the kernel (kexts), so be wary of changes */
typedef struct {
	char            cpuid_vendor[16];
	char            cpuid_brand_string[48];
	const char      *cpuid_model_string;

	cpu_type_t      cpuid_type;     /* this is *not* a cpu_type_t in our <mach/machine.h> */
	uint8_t         cpuid_family;
	uint8_t         cpuid_model;
	uint8_t         cpuid_extmodel;
	uint8_t         cpuid_extfamily;
	uint8_t         cpuid_stepping;
	uint64_t        cpuid_features;
	uint64_t        cpuid_extfeatures;
	uint32_t        cpuid_signature;
	uint8_t         cpuid_brand;
	uint8_t         cpuid_processor_flag;

	uint32_t        cache_size[LCACHE_MAX];
	uint32_t        cache_linesize;

	uint8_t         cache_info[64];    /* list of cache descriptors */

	uint32_t        cpuid_cores_per_package;
	uint32_t        cpuid_logical_per_package;
	uint32_t        cache_sharing[LCACHE_MAX];
	uint32_t        cache_partitions[LCACHE_MAX];

	cpu_type_t      cpuid_cpu_type;                 /* <mach/machine.h> */
	cpu_subtype_t   cpuid_cpu_subtype;              /* <mach/machine.h> */

	/* Per-vendor info */
	cpuid_mwait_leaf_t      cpuid_mwait_leaf;
#define cpuid_mwait_linesize_max        cpuid_mwait_leaf.linesize_max
#define cpuid_mwait_linesize_min        cpuid_mwait_leaf.linesize_min
#define cpuid_mwait_extensions          cpuid_mwait_leaf.extensions
#define cpuid_mwait_sub_Cstates         cpuid_mwait_leaf.sub_Cstates
	cpuid_thermal_leaf_t    cpuid_thermal_leaf;
	cpuid_arch_perf_leaf_t  cpuid_arch_perf_leaf;
	uint32_t        unused[4];                      /* cpuid_xsave_leaf */

	/* Cache details: */
	uint32_t        cpuid_cache_linesize;
	uint32_t        cpuid_cache_L2_associativity;
	uint32_t        cpuid_cache_size;

	/* Virtual and physical address aize: */
	uint32_t        cpuid_address_bits_physical;
	uint32_t        cpuid_address_bits_virtual;

	uint32_t        cpuid_microcode_version;

	/* Numbers of tlbs per processor [i|d, small|large, level0|level1] */
	uint32_t        cpuid_tlb[2][2][2];
			#define TLB_INST        0
			#define TLB_DATA        1
			#define TLB_SMALL       0
			#define TLB_LARGE       1
	uint32_t        cpuid_stlb;

	uint32_t        core_count;
	uint32_t        thread_count;

	/* Max leaf ids available from CPUID */
	uint32_t        cpuid_max_basic;
	uint32_t        cpuid_max_ext;

	/* Family-specific info links */
	uint32_t                cpuid_cpufamily;
	cpuid_mwait_leaf_t      *cpuid_mwait_leafp;
	cpuid_thermal_leaf_t    *cpuid_thermal_leafp;
	cpuid_arch_perf_leaf_t  *cpuid_arch_perf_leafp;
	cpuid_xsave_leaf_t      *cpuid_xsave_leafp;
	uint64_t                cpuid_leaf7_features;
	uint64_t                cpuid_leaf7_extfeatures;
	cpuid_tsc_leaf_t        cpuid_tsc_leaf;
	cpuid_xsave_leaf_t      cpuid_xsave_leaf[2];
} i386_cpu_info_t;

#if defined(MACH_KERNEL_PRIVATE) && !defined(ASSEMBLER)
/* Only for 32bit values */
#define bit32(n)                (1U << (n))
#define bitmask32(h, l)          ((bit32(h)|(bit32(h)-1)) & ~(bit32(l)-1))
#define bitfield32(x, h, l)       ((((x) & bitmask32(h,l)) >> l))

typedef struct {
	char            cpuid_vmm_vendor[16];
	uint32_t        cpuid_vmm_family;
	uint32_t        cpuid_vmm_bus_frequency;
	uint32_t        cpuid_vmm_tsc_frequency;
} i386_vmm_info_t;

typedef enum {
	CPU_INTEL_SEGCHK = 1,
	CPU_INTEL_TSXFA = 2
} cpu_wa_e;

typedef enum {
	CWA_ON = 2,
	CWA_FORCE_ON = 3,       /* FORCE_ON shares bit 1 so consumers can test that for ON */
	CWA_OFF = 4,
	CWA_FORCE_OFF = 5       /* Similarly for FORCE_OFF sharing bit 2 */
} cwa_classifier_e;

static inline int
is_xeon_sp(uint8_t platid)
{
	if (platid == PLATID_XEON_SP_1 || platid == PLATID_XEON_SP_2) {
		return 1;
	}
	if (platid != PLATID_MAYBE_XEON_SP) {
		return 0;
	}
	boolean_t intrs = ml_set_interrupts_enabled(FALSE);
	outl(cfgAdr, XeonCapID5);
	uint32_t cap5reg = inl(cfgDat);
	ml_set_interrupts_enabled(intrs);
	/* Read from PCI config space 1:30:3:0x98 [bits 13:9] */
	if (bitfield32(cap5reg, 13, 9) == 3) {
		return 1;
	}
	return 0;
}

extern int force_tecs_at_idle;

#endif /* defined(MACH_KERNEL_PRIVATE) && !defined(ASSEMBLER) */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * External declarations
 */
extern cpu_type_t       cpuid_cputype(void);
extern cpu_subtype_t    cpuid_cpusubtype(void);
extern void             cpuid_cpu_display(const char *);
extern void             cpuid_feature_display(const char *);
extern void             cpuid_extfeature_display(const char *);
extern char *           cpuid_get_feature_names(uint64_t, char *, unsigned);
extern char *           cpuid_get_extfeature_names(uint64_t, char *, unsigned);
extern char *           cpuid_get_leaf7_feature_names(uint64_t, char *, unsigned);
extern char *           cpuid_get_leaf7_extfeature_names(uint64_t, char *, unsigned);

extern uint64_t         cpuid_features(void);
extern uint64_t         cpuid_extfeatures(void);
extern uint64_t         cpuid_leaf7_features(void);
extern uint64_t         cpuid_leaf7_extfeatures(void);
extern uint32_t         cpuid_family(void);
extern uint32_t         cpuid_cpufamily(void);

extern i386_cpu_info_t  *cpuid_info(void);
extern void             cpuid_set_info(void);

#ifdef MACH_KERNEL_PRIVATE
extern boolean_t        cpuid_vmm_present(void);
extern i386_vmm_info_t  *cpuid_vmm_info(void);
extern uint32_t         cpuid_vmm_family(void);
extern cwa_classifier_e cpuid_wa_required(cpu_wa_e wa);
extern void cpuid_do_was(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ASSEMBLER */

#endif /* __APPLE_API_PRIVATE */
#endif /* _MACHINE_CPUID_H_ */
