/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#pragma once

#include <arm64/proc_reg.h>
#include <machine/atomic.h>

#define tlbi_addr(x) ((((x) >> 12) & TLBI_ADDR_MASK) << TLBI_ADDR_SHIFT)
#define tlbi_asid(x) (((uintptr_t)(x) & TLBI_ASID_MASK) << TLBI_ASID_SHIFT)

#if __ARM_KERNEL_PROTECT__
/*
 * __ARM_KERNEL_PROTECT__ adds two complications to TLB management:
 *
 * 1. As each pmap has two ASIDs, every TLB operation that targets an ASID must
 *   target both ASIDs for the pmap that owns the target ASID.
 *
 * 2. Any TLB operation targeting the kernel_pmap ASID (ASID 0) must target all
 *   ASIDs (as kernel_pmap mappings may be referenced while using an ASID that
 *   belongs to another pmap).  We expect these routines to be called with the
 *   EL0 ASID for the target; not the EL1 ASID.
 */
#endif /* __ARM_KERNEL_PROTECT__ */

static inline void
sync_tlb_flush(void)
{
	__builtin_arm_dsb(DSB_ISH);
	__builtin_arm_isb(ISB_SY);
}

// flush_mmu_tlb: full TLB flush on all cores
static inline void
flush_mmu_tlb_async(void)
{
	asm volatile ("tlbi vmalle1is");
}

static inline void
flush_mmu_tlb(void)
{
	flush_mmu_tlb_async();
	sync_tlb_flush();
}

// flush_core_tlb: full TLB flush on local core only
static inline void
flush_core_tlb_async(void)
{
	asm volatile ("tlbi vmalle1");
}

static inline void
flush_core_tlb(void)
{
	flush_core_tlb_async();
	sync_tlb_flush();
}

// flush_mmu_tlb_allentries_async: flush entries that map VA range, all ASIDS, all cores
// start and end are in units of 4K pages.
static inline void
flush_mmu_tlb_allentries_async(uint64_t start, uint64_t end)
{
#if __ARM_16K_PG__
	start = start & ~0x3ULL;

	/*
	 * The code below is not necessarily correct.  From an overview of
	 * the client code, the expected contract for TLB flushes is that
	 * we will expand from an "address, length" pair to "start address,
	 * end address" in the course of a TLB flush.  This suggests that
	 * a flush for "X, X+4" is actually only asking for a flush of a
	 * single 16KB page.  At the same time, we'd like to be prepared
	 * for bad inputs (X, X+3), so add 3 and then truncate the 4KB page
	 * number to a 16KB page boundary.  This should deal correctly with
	 * unaligned inputs.
	 *
	 * If our expecations about client behavior are wrong however, this
	 * will lead to occasional TLB corruption on platforms with 16KB
	 * pages.
	 */
	end = (end + 0x3ULL) & ~0x3ULL;
#endif // __ARM_16K_PG__
	for (; start < end; start += (ARM_PGBYTES / 4096)) {
		asm volatile ("tlbi vaae1is, %0" : : "r"(start));
	}
}

static inline void
flush_mmu_tlb_allentries(uint64_t start, uint64_t end)
{
	flush_mmu_tlb_allentries_async(start, end);
	sync_tlb_flush();
}

// flush_mmu_tlb_entry: flush TLB entries that map a VA and ASID, all cores
// Will also flush global entries that match the VA
static inline void
flush_mmu_tlb_entry_async(uint64_t val)
{
#if __ARM_KERNEL_PROTECT__
	uint64_t asid = val >> TLBI_ASID_SHIFT;
	if (asid == 0) {
		asm volatile ("tlbi vaae1is, %0" : : "r"(val));
		return;
	}
	val = val & ~(1ULL << TLBI_ASID_SHIFT);
	asm volatile ("tlbi vae1is, %0" : : "r"(val));
	val = val | (1ULL << TLBI_ASID_SHIFT);
#endif /* __ARM_KERNEL_PROTECT__ */
	asm volatile ("tlbi vae1is, %0" : : "r"(val));
}

static inline void
flush_mmu_tlb_entry(uint64_t val)
{
	flush_mmu_tlb_entry_async(val);
	sync_tlb_flush();
}

// flush_mmu_tlb_entries: flush TLB entries that map a VA range and ASID, all cores
// start and end must have the ASID in the high 16 bits, with the VA in units of 4K in the lowest bits
// Will also flush global entries that match the VA range
static inline void
flush_mmu_tlb_entries_async(uint64_t start, uint64_t end)
{
#if __ARM_16K_PG__
	start = start & ~0x3ULL;

	/*
	 * The code below is not necessarily correct.  From an overview of
	 * the client code, the expected contract for TLB flushes is that
	 * we will expand from an "address, length" pair to "start address,
	 * end address" in the course of a TLB flush.  This suggests that
	 * a flush for "X, X+4" is actually only asking for a flush of a
	 * single 16KB page.  At the same time, we'd like to be prepared
	 * for bad inputs (X, X+3), so add 3 and then truncate the 4KB page
	 * number to a 16KB page boundary.  This should deal correctly with
	 * unaligned inputs.
	 *
	 * If our expecations about client behavior are wrong however, this
	 * will lead to occasional TLB corruption on platforms with 16KB
	 * pages.
	 */
	end = (end + 0x3ULL) & ~0x3ULL;
#endif // __ARM_16K_PG__
#if __ARM_KERNEL_PROTECT__
	uint64_t asid = start >> TLBI_ASID_SHIFT;
	/*
	 * If we are flushing ASID 0, this is a kernel operation.  With this
	 * ASID scheme, this means we should flush all ASIDs.
	 */
	if (asid == 0) {
		for (; start < end; start += (ARM_PGBYTES / 4096)) {
			asm volatile ("tlbi vaae1is, %0" : : "r"(start));
		}
		return;
	}
	start = start | (1ULL << TLBI_ASID_SHIFT);
	end = end | (1ULL << TLBI_ASID_SHIFT);
	for (; start < end; start += (ARM_PGBYTES / 4096)) {
		start = start & ~(1ULL << TLBI_ASID_SHIFT);
		asm volatile ("tlbi vae1is, %0" : : "r"(start));
		start = start | (1ULL << TLBI_ASID_SHIFT);
		asm volatile ("tlbi vae1is, %0" : : "r"(start));
	}
#else
	for (; start < end; start += (ARM_PGBYTES / 4096)) {
		asm volatile ("tlbi vae1is, %0" : : "r"(start));
	}
#endif /* __ARM_KERNEL_PROTECT__ */
}

static inline void
flush_mmu_tlb_entries(uint64_t start, uint64_t end)
{
	flush_mmu_tlb_entries_async(start, end);
	sync_tlb_flush();
}

// flush_mmu_tlb_asid: flush all entries that match an ASID, on all cores
// ASID must be in high 16 bits of argument
// Will not flush global entries
static inline void
flush_mmu_tlb_asid_async(uint64_t val)
{
#if __ARM_KERNEL_PROTECT__
	/*
	 * If we are flushing ASID 0, this is a kernel operation.  With this
	 * ASID scheme, this means we should flush all ASIDs.
	 */
	uint64_t asid = val >> TLBI_ASID_SHIFT;
	if (asid == 0) {
		asm volatile ("tlbi vmalle1is");
		return;
	}
	val = val & ~(1ULL << TLBI_ASID_SHIFT);
	asm volatile ("tlbi aside1is, %0" : : "r"(val));
	val = val | (1ULL << TLBI_ASID_SHIFT);
#endif /* __ARM_KERNEL_PROTECT__ */
	asm volatile ("tlbi aside1is, %0" : : "r"(val));
}

static inline void
flush_mmu_tlb_asid(uint64_t val)
{
	flush_mmu_tlb_asid_async(val);
	sync_tlb_flush();
}

// flush_core_tlb_asid: flush all entries that match an ASID, local core only
// ASID must be in high 16 bits of argument
// Will not flush global entries
static inline void
flush_core_tlb_asid_async(uint64_t val)
{
#if __ARM_KERNEL_PROTECT__
	/*
	 * If we are flushing ASID 0, this is a kernel operation.  With this
	 * ASID scheme, this means we should flush all ASIDs.
	 */
	uint64_t asid = val >> TLBI_ASID_SHIFT;
	if (asid == 0) {
		asm volatile ("tlbi vmalle1");
		return;
	}
	val = val & ~(1ULL << TLBI_ASID_SHIFT);
	asm volatile ("tlbi aside1, %0" : : "r"(val));
	val = val | (1ULL << TLBI_ASID_SHIFT);
#endif /* __ARM_KERNEL_PROTECT__ */
	asm volatile ("tlbi aside1, %0" : : "r"(val));
}

static inline void
flush_core_tlb_asid(uint64_t val)
{
	flush_core_tlb_asid_async(val);
	sync_tlb_flush();
}

#if __ARM_RANGE_TLBI__
#if __ARM_KERNEL_PROTECT__
	#error __ARM_RANGE_TLBI__ + __ARM_KERNEL_PROTECT__ is not currently supported
#endif

#define ARM64_16K_TLB_RANGE_PAGES (1ULL << 21)
#define rtlbi_addr(x) (((x) >> RTLBI_ADDR_SHIFT) & RTLBI_ADDR_MASK)
#define rtlbi_scale(x) ((uint64_t)(x) << RTLBI_SCALE_SHIFT)
#define rtlbi_num(x) ((uint64_t)(x) << RTLBI_NUM_SHIFT)

/**
 * Given the number of pages to invalidate, generate the correct parameter to
 * pass to any of the TLBI by range methods.
 */
static inline uint64_t
generate_rtlbi_param(ppnum_t npages, uint32_t asid, vm_offset_t va)
{
	/**
	 * Per the armv8.4 RTLBI extension spec, the range encoded in the rtlbi register operand is defined by:
	 * BaseADDR <= VA < BaseADDR+((NUM+1)*2^(5*SCALE+1) * Translation_Granule_Size)
	 */
	unsigned order = (sizeof(npages) * 8) - __builtin_clz(npages - 1) - 1;
	unsigned scale = ((order ? order : 1) - 1) / 5;
	unsigned granule = 1 << ((5 * scale) + 1);
	unsigned num = (((npages + granule - 1) & ~(granule - 1)) / granule) - 1;
	return tlbi_asid(asid) | RTLBI_TG | rtlbi_scale(scale) | rtlbi_num(num) | rtlbi_addr(va);
}

// flush_mmu_tlb_range: flush TLB entries that map a VA range using a single instruction
// The argument should be encoded according to generate_rtlbi_param().
// Follows the same ASID matching behavior as flush_mmu_tlb_entries()
static inline void
flush_mmu_tlb_range_async(uint64_t val)
{
	asm volatile ("tlbi rvae1is, %0" : : "r"(val));
}

static inline void
flush_mmu_tlb_range(uint64_t val)
{
	flush_mmu_tlb_range_async(val);
	sync_tlb_flush();
}

// flush_mmu_tlb_allrange: flush TLB entries that map a VA range using a single instruction
// The argument should be encoded according to generate_rtlbi_param().
// Follows the same ASID matching behavior as flush_mmu_tlb_allentries()
static inline void
flush_mmu_tlb_allrange_async(uint64_t val)
{
	asm volatile ("tlbi rvaae1is, %0" : : "r"(val));
}

static inline void
flush_mmu_tlb_allrange(uint64_t val)
{
	flush_mmu_tlb_allrange_async(val);
	sync_tlb_flush();
}

#endif // __ARM_RANGE_TLBI__


