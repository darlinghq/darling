/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */


#ifndef __NANO_ZONE_H
#define __NANO_ZONE_H

#if CONFIG_NANOZONE

/*********************	DEFINITIONS	************************/

#define NANO_MAG_INDEX(nz)		(_os_cpu_number() >> nz->hyper_shift)

#define MAX_RECORDER_BUFFER		256

/*************          nanozone address field layout        ******************/

#if defined(__x86_64)
#define NANO_SIGNATURE_BITS		20
#define NANOZONE_SIGNATURE		0x00006ULL			// 0x00006nnnnnnnnnnn the address range devoted to us.
#define NANO_MAG_BITS			5
#define NANO_BAND_BITS			18
#define NANO_SLOT_BITS			4
#define NANO_OFFSET_BITS		17

#else
#error Unknown Architecture
#endif

// clang-format really dislikes the bitfields here
// clang-format off
#if defined(__BIG_ENDIAN__)
struct nano_blk_addr_s {
    uint64_t
	nano_signature:NANO_SIGNATURE_BITS,	// 0x00006nnnnnnnnnnn the address range devoted to us.
	nano_mag_index:NANO_MAG_BITS,		// the core that allocated this block
	nano_band:NANO_BAND_BITS,
	nano_slot:NANO_SLOT_BITS,		// bucket of homogenous quanta-multiple blocks
	nano_offset:NANO_OFFSET_BITS;		// locates the block
};
#else
// least significant bits declared first
struct nano_blk_addr_s {
    uint64_t
	nano_offset:NANO_OFFSET_BITS,		// locates the block
	nano_slot:NANO_SLOT_BITS,		// bucket of homogenous quanta-multiple blocks
	nano_band:NANO_BAND_BITS,
	nano_mag_index:NANO_MAG_BITS,		// the core that allocated this block
	nano_signature:NANO_SIGNATURE_BITS;	// 0x00006nnnnnnnnnnn the address range devoted to us.
};
#endif
// clang-format on

typedef union  {
    uint64_t			addr;
    struct nano_blk_addr_s	fields;
} nano_blk_addr_t;

#define NANO_MAX_SIZE			256 /* Buckets sized {16, 32, 48, 64, 80, 96, 112, ...} */
#define SHIFT_NANO_QUANTUM		4
#define NANO_REGIME_QUANTA_SIZE		(1 << SHIFT_NANO_QUANTUM)	// 16
#define NANO_QUANTA_MASK		0xFULL				// NANO_REGIME_QUANTA_SIZE - 1

#define SLOT_IN_BAND_SIZE 	(1 << NANO_OFFSET_BITS)
#define SLOT_KEY_LIMIT 		(1 << NANO_SLOT_BITS) /* Must track nano_slot width */
#define BAND_SIZE 		(1 << (NANO_SLOT_BITS + NANO_OFFSET_BITS)) /*  == Number of bytes covered by a page table entry */
#define NANO_MAG_SIZE 		(1 << NANO_MAG_BITS)
#define NANO_SLOT_SIZE 		(1 << NANO_SLOT_BITS)

/****************************** zone itself ***********************************/

/*
 * Note that objects whose adddress are held in pointers here must be pursued
 * individually in the nano_in_use_enumeration() routines.
 */

typedef struct chained_block_s {
    uintptr_t			double_free_guard;
    struct chained_block_s	*next;
} *chained_block_t;

typedef struct nano_meta_s {
    OSQueueHead			slot_LIFO MALLOC_NANO_CACHE_ALIGN;
    unsigned int		slot_madvised_log_page_count;
    volatile uintptr_t		slot_current_base_addr;
    volatile uintptr_t		slot_limit_addr;
    volatile size_t		slot_objects_mapped;
    volatile size_t		slot_objects_skipped;
    bitarray_t			slot_madvised_pages;
    // position on cache line distinct from that of slot_LIFO
    volatile uintptr_t		slot_bump_addr MALLOC_NANO_CACHE_ALIGN;
    volatile boolean_t		slot_exhausted;
    unsigned int		slot_bytes;
    unsigned int		slot_objects;
} *nano_meta_admin_t;

// vm_allocate()'d, so page-aligned to begin with.
typedef struct nanozone_s {
    // first page will be given read-only protection
    malloc_zone_t		basic_zone;
    uint8_t			pad[PAGE_MAX_SIZE - sizeof(malloc_zone_t)];

    // remainder of structure is R/W (contains no function pointers)
    // page-aligned
    // max: NANO_MAG_SIZE cores x NANO_SLOT_SIZE slots for nano blocks {16 .. 256}
    struct nano_meta_s		meta_data[NANO_MAG_SIZE][NANO_SLOT_SIZE];
    _malloc_lock_s			band_resupply_lock[NANO_MAG_SIZE];
    uintptr_t           band_max_mapped_baseaddr[NANO_MAG_SIZE];
    size_t			core_mapped_size[NANO_MAG_SIZE];

    unsigned			debug_flags;
    unsigned			our_signature;
    unsigned			phys_ncpus;
    unsigned			logical_ncpus;
    unsigned			hyper_shift;

    /* security cookie */
    uintptr_t			cookie;

    /*
     * The nano zone constructed by create_nano_zone() would like to hand off tiny, small, and large
     * allocations to the default scalable zone. Record the latter as the "helper" zone here.
     */
    malloc_zone_t		*helper_zone;
} nanozone_t;

#define NANOZONE_PAGED_SIZE	((sizeof(nanozone_t) + vm_page_size - 1) & ~ (vm_page_size - 1))

#endif // CONFIG_NANOZONE

#endif // __NANO_ZONE_H
