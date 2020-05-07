/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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
 *      Below is a diagram of the caching system. This design is based of the
 * paper "Magazines and Vmem: Extending the Slab Allocator to Many CPUs and
 * Arbitrary Resources" by Jeff Bonwick and Jonathan Adams. It is divided into 3
 * layers: the Per-cpu Layer, the Depot Layer, and the Zone Allocator. The
 * Per-CPU and Depot layers store elements using arrays we call magazines.
 *
 *      Magazines function like a stack (we push and pop elements) and can be
 *  moved around for bulk operations.
 *  _________         _________         _________
 * |  CPU 1  |       |  CPU 2  |       |  CPU 3  |
 * |  _   _  |       |  _   _  |       |  _   _  |
 * | |#| | | |       | | | |#| |       | |#| |#| |        Per-CPU Layer
 * | |#| |_| |       | |_| |#| |       | |#| |#| |
 * |_________|       |_________|       |_________|
 *
 *  ______________________________________________
 * |            _   _   _   _   _   _             |
 * |           |#| |#| |#| | | | | | |            |     Depot Layer
 * |           |#| |#| |#| |_| |_| |_|            |
 * |______________________________________________|
 *
 *  _______________________________________________
 * | # | # | # | # | # | # | # | # | # | # | # | # |   Zone Allocator
 * |_______________________________________________|
 *
 *	The top layer is the per-cpu cache and consists of a current and
 * previous magazine for each CPU. The current magazine is the one we always try
 * to allocate from and free to first. Only if we are unable, do we check the
 * previous magazine. If the previous magazine can satisfy the allocate or free,
 * then we switch the two and allocate from the new current magazine. This layer
 * requires no locking, so we can access multiple CPU's caches concurrently.
 * This is the main source of the speedup.
 *
 *      We have two magazines here to prevent thrashing when swapping magazines
 * with the depot layer. If a certain pattern of alloc and free are called we
 * can waste a lot of time swapping magazines to and from the depot layer. We
 * prevent this by dividing the per-cpu cache into two separate magazines.
 *
 *	The middle layer is the magazine depot. This layer consists of a
 * collection of full and empty magazines. These are used to reload the per-cpu
 * caches when needed. This is implemented as an array of magazines which are
 * initially all empty and as we fill up magazines we increment the index to
 * point at the first empty magazine. Since this layer is per-zone, it allows us
 *  to balance the cache between cpus, but does require taking a lock.
 *
 *      When neither the current nor previous magazine for a given CPU can
 * satisfy the free or allocation, we look to the depot layer. If there are
 * magazines in the depot that can satisfy the free or allocation we swap
 * that magazine into the current position. In the example below, to allocate on
 * the given CPU we must lock the depot layer and swap magazine A with magazine
 * B and decrement the depot index.
 *
 *      _____________________       _______________________________________
 *     |    Per-CPU Cache    |     |              Depot Layer              |
 *     |                     |     |                                       |
 *     |   A___      ____    |     |   ____      B___      ____      ____  |
 *     |  |    |    |    |   |     |  | ## |    | ## |    |    |    |    | |
 *     |  |    |    |    |   |     |  | ## |    | ## |    |    |    |    | |
 *     |  |    |    |    |   |     |  | ## |    | ## |    |    |    |    | |
 *     |  |    |    |    |   |     |  | ## |    | ## |    |    |    |    | |
 *     |  |____|    |____|   |     |  |_##_|    |_##_|    |____|    |____| |
 *     | Current   Previous  |     |                                       |
 *     |_____________________|     |_______________________________________|
 *
 *	The bottom layer is the Zone Allocator. This is already implemented in
 *  XNU and will remain mostly unchanged. Implementation for this can be found
 * in zalloc.c and zalloc.h. We will only use the zone if all other layers are
 * unable to satisfy the allocation or free. When we do use the zone, we will
 * try to allocate an entire magazine of elements or free an entire magazine of
 * elements at once.
 *
 *      Caching must be enabled explicitly, by calling zone_change() with the
 * Z_CACHING_ENABLED flag, for every zone you want to cache elements for. Zones
 * which are good candidates for this are ones with highly contended zone locks.
 *
 * Some good potential candidates are kalloc.16, kalloc.48, Vm objects, VM map
 * entries, ipc vouchers, and ipc ports.
 *
 *
 * Some factors can be tuned by boot-arg:
 *  zcc_enable_for_zone_name    name of a single zone to enable caching for
 *				(replace space characters with '.')
 *
 *  zcc_magazine_element_count	integer value for magazine size used for all
 *				zones (default 8 is used if not specified)
 *
 *  zcc_depot_element_count	integer value for how many full and empty
 *				magazines to store in the depot, if N specified
 *				depot will have N full and N empty magazines
 *				(default 16 used if not specified)
 */
#include <kern/kern_types.h>
#include <vm/vm_kern.h>


/*
 * zcache_ready
 *
 * Description: returns whether or not the zone caches are ready to use
 *
 */
bool            zcache_ready(void);


/*
 * zcache_bootstrap
 *
 * Description: initializes zone to allocate magazines from
 *
 */
void            zcache_bootstrap(void);


/*
 * zcache_init
 *
 * Description: Initializes all parts of the per-cpu caches for a given zone
 *
 * Parameters:	zone	pointer to zone on which to iniitalize caching
 *
 */
void            zcache_init(zone_t zone);


/*
 * zcache_free_to_cpu_cache
 *
 * Description: Checks per-cpu caches to free element there if possible
 *
 * Parameters:	zone	pointer to zone for which element comes from
 *		addr	pointer to element to free
 *
 * Returns: TRUE if successfull, FALSE otherwise
 *
 * Precondition: check that caching is enabled for zone
 */
bool            zcache_free_to_cpu_cache(zone_t zone, void *addr);


/*
 * zcache_alloc_from_cpu_cache
 *
 * Description: Checks per-cpu caches to allocate element from there if possible
 *
 * Parameters:	zone	pointer to zone for which element will come from
 *
 * Returns: pointer to usable element
 *
 * Precondition: check that caching is enabled for zone
 */
vm_offset_t     zcache_alloc_from_cpu_cache(zone_t zone);

/*
 * zcache_drain_depot
 *
 * Description: Frees all the full magazines from the depot layer to the zone allocator
 *              Invoked by zone_gc()
 *
 * Parameters:  zone    pointer to zone for which the depot layer needs to be drained
 *
 * Returns: None
 *
 */
void            zcache_drain_depot(zone_t zone);
