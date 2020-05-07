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

#include <arm/proc_reg.h>

#define tlbi_addr(x) ((((x) >> 12) & TLBI_ADDR_MASK) << TLBI_ADDR_SHIFT)
#define tlbi_asid(x) (((uintptr_t)(x) & TLBI_ASID_MASK) << TLBI_ASID_SHIFT)

extern void sync_tlb_flush(void);
extern void flush_mmu_tlb_async(void);
extern void flush_mmu_tlb(void);
extern void flush_core_tlb_async(void);
extern void flush_core_tlb(void);
extern void flush_mmu_tlb_entry_async(uint32_t);
extern void flush_mmu_tlb_entry(uint32_t);
extern void flush_mmu_tlb_entries_async(uint32_t, uint32_t);
extern void flush_mmu_tlb_entries(uint32_t, uint32_t);
extern void flush_mmu_tlb_mva_entries_async(uint32_t);
extern void flush_mmu_tlb_mva_entries(uint32_t);
extern void flush_mmu_tlb_asid_async(uint32_t);
extern void flush_mmu_tlb_asid(uint32_t);
extern void flush_core_tlb_asid_async(uint32_t);
extern void flush_core_tlb_asid(uint32_t);
