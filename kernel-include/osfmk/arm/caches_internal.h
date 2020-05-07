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
#ifndef _ARM_CACHES_INTERNAL
#define _ARM_CACHES_INTERNAL    1

#include <arm/proc_reg.h>

#include <kern/kern_types.h>

extern void flush_dcache_syscall( vm_offset_t addr, unsigned length);

#ifdef MACH_KERNEL_PRIVATE
extern void flush_dcache(vm_offset_t addr, unsigned count, int phys);
extern void flush_dcache64(addr64_t addr, unsigned count, int phys);
extern void invalidate_icache(vm_offset_t addr, unsigned cnt, int phys);
extern void invalidate_icache64(addr64_t addr, unsigned cnt, int phys);

#if     __ARM_SMP__ && defined(ARMA7)
#define LWFlush 1
#define LWClean 2
extern void cache_xcall(unsigned int op);
extern void cache_xcall_handler(unsigned int op);
#endif
#endif
extern void clean_dcache(vm_offset_t addr, unsigned count, int phys);
extern void clean_dcache64(addr64_t addr, unsigned count, int phys);

extern void CleanPoC_Dcache(void);
extern void CleanPoU_Dcache(void);

/*
 * May not actually perform a flush on platforms
 * where AP caches are snooped by all agents on SoC.
 *
 * This is the one you need unless you really know what
 * you're doing.
 */
extern void CleanPoC_DcacheRegion(vm_offset_t va, unsigned length);

/*
 * Always actually flushes the cache, even on platforms
 * where AP caches are snooped by all agents.  You
 * probably don't need to use this.  Intended for use in
 * panic save routine (where caches will be yanked by reset
 * and coherency doesn't help).
 */
extern void CleanPoC_DcacheRegion_Force(vm_offset_t va, unsigned length);

extern void CleanPoU_DcacheRegion(vm_offset_t va, unsigned length);

extern void FlushPoC_Dcache(void);
extern void FlushPoU_Dcache(void);
extern void FlushPoC_DcacheRegion(vm_offset_t va, unsigned length);

#ifdef  __arm__
extern void invalidate_mmu_cache(void);
extern void invalidate_mmu_dcache(void);
extern void invalidate_mmu_dcache_region(vm_offset_t va, unsigned length);
#endif

extern void InvalidatePoU_Icache(void);
extern void InvalidatePoU_IcacheRegion(vm_offset_t va, unsigned length);

extern void cache_sync_page(ppnum_t pp);

extern void platform_cache_init(void);
extern void platform_cache_idle_enter(void);
extern void platform_cache_idle_exit(void);
extern void platform_cache_flush(void);
extern boolean_t platform_cache_batch_wimg(unsigned int new_wimg, unsigned int size);
extern void platform_cache_flush_wimg(unsigned int new_wimg);
extern void platform_cache_clean(void);
extern void platform_cache_shutdown(void);
extern void platform_cache_disable(void);

#endif /* #ifndef _ARM_CACHES_INTERNAL */
