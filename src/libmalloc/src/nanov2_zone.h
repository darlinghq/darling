/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#ifndef __NANOV2_ZONE_H
#define __NANOV2_ZONE_H

#if CONFIG_NANOZONE

#pragma mark -
#pragma mark Address Structure

#if TARGET_OS_OSX || TARGET_OS_SIMULATOR || TARGET_OS_DRIVERKIT

#define NANOV2_REGION_BITS		15
#define NANOV2_ARENA_BITS		3
#define NANOV2_BLOCK_BITS		12
#define NANOV2_OFFSET_BITS		14

#else // TARGET_OS_OSX || TARGET_OS_SIMULATOR || TARGET_OS_DRIVERKIT

#define NANOV2_REGION_BITS		0
#define NANOV2_ARENA_BITS		3
#define NANOV2_BLOCK_BITS		12
#define NANOV2_OFFSET_BITS		14

#endif // TARGET_OS_OSX || TARGET_OS_SIMULATOR || TARGET_OS_DRIVERKIT

#if NANOV2_REGION_BITS > 0
#define NANOV2_MULTIPLE_REGIONS	1
#else	// NANOV2_REGION_BITS > 0
#define NANOV2_MULTIPLE_REGIONS	0
#endif	// NANOV2_REGION_BITS > 0

// Size of a block (currently 16KB)
#define NANOV2_BLOCK_SIZE		(1 << NANOV2_OFFSET_BITS)

// Size of an arena (currently 64MB)
#define NANOV2_ARENA_SIZE		(64 * 1024 * 1024)

// Size of a region (currently 512MB)
#define NANOV2_REGION_SIZE		(512 * 1024 * 1024)

// Number of blocks per arena (currently 4096)
#define NANOV2_BLOCKS_PER_ARENA	(NANOV2_ARENA_SIZE/NANOV2_BLOCK_SIZE)

// Number of arenas per region (currently 8)
#define NANOV2_ARENAS_PER_REGION	(NANOV2_REGION_SIZE/NANOV2_ARENA_SIZE)

// Maximum number of slots per block
#define NANOV2_MAX_SLOTS_PER_BLOCK	(NANOV2_BLOCK_SIZE/NANO_REGIME_QUANTA_SIZE)

// Highest region number.
#if NANOV2_MULTIPLE_REGIONS
#define NANOV2_MAX_REGION_NUMBER	((1 << NANOV2_REGION_BITS) - 1)
#else 	// NANOV2_MULTIPLE_REGIONS
#define NANOV2_MAX_REGION_NUMBER	0
#endif	// NANOV2_MULTIPLE_REGIONS

// clang-format really dislikes the bitfields here
// clang-format off
#if defined(__BIG_ENDIAN__)

// Nano V2 address structure.
struct nanov2_addr_s {
    uintptr_t nano_signature : NANOZONE_SIGNATURE_BITS;
#if NANOV2_MULTIPLE_REGIONS
    uintptr_t nano_region: NANOV2_REGION_BITS;
#endif // NANOV2_MULTIPLE_REGIONS
	uintptr_t nano_arena : NANOV2_ARENA_BITS;
	uintptr_t nano_block : NANOV2_BLOCK_BITS;
	uintptr_t nano_offset : NANOV2_OFFSET_BITS;
};
MALLOC_STATIC_ASSERT(sizeof(struct nanov2_addr_s) == sizeof(uintptr_t),
		"Wrong size for nanov2_addr_s");

#else	// defined(__BIG_ENDIAN__)

// least significant bits declared first
struct nanov2_addr_s {
    uintptr_t nano_offset : NANOV2_OFFSET_BITS;
	uintptr_t nano_block : NANOV2_BLOCK_BITS;
	uintptr_t nano_arena : NANOV2_ARENA_BITS;
#if NANOV2_MULTIPLE_REGIONS
	uintptr_t nano_region: NANOV2_REGION_BITS;
#endif // NANOV2_MULTIPLE_REGIONS
	uintptr_t nano_signature : NANOZONE_SIGNATURE_BITS;
};
MALLOC_STATIC_ASSERT(sizeof(struct nanov2_addr_s) == sizeof(uintptr_t),
		"Wrong size for nanov2_addr_s");

#endif	// defined(__BIG_ENDIAN__)

// clang-format on

// Union that allows easy extraction of the fields in a Nano V2 address.
typedef union  {
    void *addr;
    struct nanov2_addr_s fields;
} nanov2_addr_t;

// Typedef that tags a size class value. Range is 0 to NANO_SIZE_CLASSES - 1.
typedef unsigned nanov2_size_class_t;

#pragma mark -
#pragma mark Block Definitions

// A block is a chunk of NANOV2_BLOCK_SIZE bytes of memory.
typedef struct {
	unsigned char content[NANOV2_BLOCK_SIZE];
} nanov2_block_t;

MALLOC_STATIC_ASSERT(sizeof(nanov2_block_t) == NANOV2_BLOCK_SIZE,
		"nanov2_block_t must be the same size as a block");

#pragma mark -
#pragma mark Arena and Block Definitions

// An arena is an array of NANOV2_BLOCKS_PER_ARENA blocks.
typedef struct {
	nanov2_block_t blocks[NANOV2_BLOCKS_PER_ARENA];
} nanov2_arena_t;

MALLOC_STATIC_ASSERT(sizeof(nanov2_arena_t) == NANOV2_BLOCK_SIZE * NANOV2_BLOCKS_PER_ARENA,
		"nanov2_arena_t must be the same size as its blocks");

// Per-block header structure, embedded in the arena metadata block.
typedef struct {
    uint32_t next_slot : 11;	// Next slot on free list, 1-based.
    uint32_t free_count : 10;	// Free slots in this block - 1
    uint32_t gen_count : 10;	// A-B-A count
    uint32_t in_use : 1;		// Being used for allocations.
} nanov2_block_meta_t;
MALLOC_STATIC_ASSERT(sizeof(nanov2_block_meta_t) == sizeof(uint32_t),
		"Incorrect size for nanov2_block_meta_t");

// Distinguished values of next_slot
#define SLOT_NULL			0		// Slot has never been used.
#define SLOT_BUMP 			0x7fb	// Marks the end of the free list
#define SLOT_FULL			0x7fc	// Slot is full (no free slots)
#define SLOT_CAN_MADVISE 	0x7fd	// Block can be madvised (and in_use == 0)
#define SLOT_MADVISING		0x7fe	// Block is being madvised. Do not touch
#define SLOT_MADVISED 		0x7ff	// Block has been madvised.

// View of the per-block header structure that allows it to be used where a
// primitive type is required.
typedef union {
	nanov2_block_meta_t meta;
	uint32_t bits;
} nanov2_block_meta_view_t;

// Structure overlaid onto an arena's metadata block. This must be exactly
// the same size as a block.
typedef struct {
	nanov2_block_meta_t arena_block_meta[NANOV2_BLOCKS_PER_ARENA];
} nanov2_arena_metablock_t;

MALLOC_STATIC_ASSERT(sizeof(nanov2_arena_metablock_t) == NANOV2_BLOCK_SIZE,
		"nanov2_arena_metablock_t must be the same size as a block");

// Structure overlaid on slots that are on the block freelist.
typedef struct {
    uint64_t double_free_guard;
    uint16_t next_slot;
} nanov2_free_slot_t;

MALLOC_STATIC_ASSERT(
		sizeof(nanov2_free_slot_t) <= NANO_REGIME_QUANTA_SIZE,
		"nanov2_free_slot_t too large");

// Type for the index of a block in its hosting arena.
typedef unsigned nanov2_block_index_t;

// Type for the index of a block meta structure in its hosting metadata block.
typedef unsigned nanov2_meta_index_t;

#pragma mark -
#pragma mark Region Definitions

// A region is an array of NANOV2_ARENAS_PER_REGION arenas.
typedef struct {
	nanov2_arena_t arenas[NANOV2_ARENAS_PER_REGION];
} nanov2_region_t;

// Linkage between regions. Overlays the nanov2_block_meta_t that corresponds
// to the arena metadata block, so must be the same size as nanov2_block_meta_t.
typedef struct {
    uint16_t next_region_offset;	// Offset to next region in 512MB blocks
	uint16_t unused;
} nanov2_region_linkage_t;

MALLOC_STATIC_ASSERT(
		sizeof (nanov2_block_meta_t) == sizeof(nanov2_region_linkage_t),
		"nanov2_block_meta_t must be the same size as nanov2_region_linkage_t");

#pragma mark -
#pragma mark Statistics

typedef struct {
	uint64_t	total_allocations;
	uint64_t	total_frees;
	uint64_t	madvised_blocks;	// Does not reduce when reused.
	uint64_t	madvise_races;		// Reused while being madvised.
} nanov2_size_class_statistics;

typedef struct {
	// Number of allocated regions
	unsigned allocated_regions;

	// Number of times a region could not be placed at its preferred location
	unsigned region_address_clashes;

	// Statistics collected by size class
	nanov2_size_class_statistics size_class_statistics[NANO_SIZE_CLASSES];
} nanov2_statistics_t;

#pragma mark -
#pragma mark Zone Definitions

// Maximum number of currently active allocation blocks per size class.
// Initially, the default is for each physical CPU to have a dedicated block.
#define MAX_CURRENT_BLOCKS 64
#define MAX_CURRENT_BLOCKS_MASK (MAX_CURRENT_BLOCKS - 1)
MALLOC_STATIC_ASSERT(MAX_CURRENT_BLOCKS > 1 &&
		!(MAX_CURRENT_BLOCKS & MAX_CURRENT_BLOCKS_MASK),
		"MAX_CURRENT_BLOCKS must be a power of 2");

typedef struct nanozonev2_s {
    // first page will be given read-only protection
    malloc_zone_t		basic_zone;
    uint8_t				pad[PAGE_MAX_SIZE - sizeof(malloc_zone_t)];

	// Current allocation blocks. Indexed by a factor that may change in the
	// future (and may be tuneable). Initially indexed by physical CPU number.
	nanov2_block_meta_t *current_block[NANO_SIZE_CLASSES][MAX_CURRENT_BLOCKS];

	// Locks for the current allocation blocks.
	_malloc_lock_s		current_block_lock[NANO_SIZE_CLASSES][MAX_CURRENT_BLOCKS];

	// Lock for delegate_allocations.
	_malloc_lock_s		delegate_allocations_lock;

	// Mask of size classes for which allocation should be delegated when a new
	// block is needed and the class has become full.
	uint16_t			delegate_allocations;

	// Zone debug flags.
    unsigned			debug_flags;
	
	// Cookie used for ASLR within an arena.
	uint64_t			aslr_cookie;
	uint64_t			aslr_cookie_aligned;

    // cookie used to protect linkage on the block freelist
    uintptr_t			slot_freelist_cookie;

	// The zone to which allocations that cannot be satisfied by Nano V2
	// will be handed off.
	malloc_zone_t		*helper_zone;
	
	// Lock used to serialize access to current_block.
	_malloc_lock_s		blocks_lock;

	// Lock used to protect current_region_base, current_region_next_arena and
	// current_region_limit.
	_malloc_lock_s		regions_lock;
	
	// Base address of the first region. Fixed once set.
	nanov2_region_t 	*first_region_base;
	
	// Base address of the current region. Always the most recently allocated
	// region and therefore the one with the highest base address.
	nanov2_region_t 	*current_region_base;
	
	// Address to use for the next arena. Always between current_region_base
	// and current_region_limit.
	nanov2_arena_t		*current_region_next_arena;
	
 	// Limit address of the current region (first byte after the region).
 	void				*current_region_limit;
	
	// Lock used when madvising.
	_malloc_lock_s		madvise_lock;

	// Global and per-size class statistics
	nanov2_statistics_t	statistics;
} nanozonev2_t;

#define NANOZONEV2_ZONE_PAGED_SIZE	mach_vm_round_page(sizeof(nanozonev2_t))

#pragma mark -
#pragma mark Address Manipulation

#define NANOV2_BLOCK_ADDRESS_MASK ~((1ULL << (NANOV2_OFFSET_BITS)) - 1)
#define NANOV2_ARENA_ADDRESS_MASK \
		~((1ULL << (NANOV2_BLOCK_BITS + NANOV2_OFFSET_BITS)) - 1)
#define NANOV2_REGION_ADDRESS_MASK \
		~((1ULL << (NANOV2_ARENA_BITS + NANOV2_BLOCK_BITS + NANOV2_OFFSET_BITS)) - 1)

#endif // CONFIG_NANOZONE

#endif // __NANOV2_ZONE_H

