/*
 * Copyright (c) 2000-2010 Apple Inc. All rights reserved.
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

#ifndef _PMAP_PCID_
#define _PMAP_PCID_     1
#if defined(__x86_64__)
void pmap_pcid_initialize(pmap_t);
void pmap_pcid_initialize_kernel(pmap_t);
pcid_t  pmap_pcid_allocate_pcid(int);
void    pmap_pcid_deallocate_pcid(int, pmap_t);
void    pmap_destroy_pcid_sync_action(void *);
void    pmap_destroy_pcid_sync(pmap_t);
void    pmap_pcid_lazy_flush(pmap_t);
void    pmap_pcid_activate(pmap_t, int, boolean_t, boolean_t);
pcid_t  pcid_for_pmap_cpu_tuple(pmap_t, thread_t, int);

#define PMAP_INVALID ((pmap_t)0xDEAD7347)
#define PMAP_PCID_INVALID_PCID  (0xDEAD)
#define PMAP_PCID_MAX_REFCOUNT (0xF0)
#define PMAP_PCID_MIN_PCID (1)

extern uint32_t pmap_pcid_ncpus;

static inline void
pmap_pcid_invalidate_all_cpus(pmap_t tpmap)
{
	unsigned i;

	pmap_assert((sizeof(tpmap->pmap_pcid_coherency_vector) >= real_ncpus) && (!(sizeof(tpmap->pmap_pcid_coherency_vector) & 7)));

	for (i = 0; i < real_ncpus; i += 8) {
		*(uint64_t *)(uintptr_t)&tpmap->pmap_pcid_coherency_vector[i] = (~0ULL);
	}
}

static inline void
pmap_pcid_validate_current(void)
{
	int     ccpu = cpu_number();
	volatile uint8_t *cptr = cpu_datap(ccpu)->cpu_pmap_pcid_coherentp;
#ifdef  PMAP_MODULE
	pmap_assert(cptr == &(current_thread()->map->pmap->pmap_pcid_coherency_vector[ccpu]));
#endif
	if (cptr) {
		*cptr = 0;
	}
}

static inline void
pmap_pcid_invalidate_cpu(pmap_t tpmap, int ccpu)
{
	tpmap->pmap_pcid_coherency_vector[ccpu] = 0xFF;
}

static inline void
pmap_pcid_validate_cpu(pmap_t tpmap, int ccpu)
{
	tpmap->pmap_pcid_coherency_vector[ccpu] = 0;
}
#endif /* x86_64 */
#endif
