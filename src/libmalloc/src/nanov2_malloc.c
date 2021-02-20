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
#include "resolver.h"
#include "internal.h"

#if CONFIG_NANOZONE

#pragma mark -
#pragma mark Forward Declarations

#if OS_VARIANT_NOTRESOLVED
static kern_return_t
nanov2_statistics_task_printer(task_t task, vm_address_t zone_address,
							   memory_reader_t reader, print_task_printer_t printer,
							   malloc_statistics_t *stats);
static kern_return_t
nanov2_statistics_task(task_t task, vm_address_t zone_address,
					   memory_reader_t reader, malloc_statistics_t *stats);
#endif // OS_VARIANT_NOTRESOLVED

#pragma mark -
#pragma mark Externals for resolved functions

MALLOC_NOEXPORT extern void *nanov2_allocate(nanozonev2_t *nanozone, size_t rounded_size,
		boolean_t clear);
MALLOC_NOEXPORT extern void nanov2_free_to_block(nanozonev2_t *nanozone, void *ptr,
		nanov2_size_class_t size_class);
MALLOC_NOEXPORT extern boolean_t nanov2_madvise_block(nanozonev2_t *nanozone,
		nanov2_block_meta_t *block_metap, nanov2_block_t *blockp,
		nanov2_size_class_t size_class);
MALLOC_NOEXPORT extern size_t nanov2_pointer_size(nanozonev2_t *nanozone, void *ptr,
		boolean_t allow_inner);
MALLOC_NOEXPORT extern size_t nanov2_pressure_relief(nanozonev2_t *nanozone, size_t goal);

#if OS_VARIANT_RESOLVED
MALLOC_NOEXPORT extern boolean_t nanov2_allocate_new_region(nanozonev2_t *nanozone);
#endif // OS_VARIANT_RESOLVED

#pragma mark -
#pragma mark Global Allocator State

// -- Block scanning
typedef enum {
	NANO_SCAN_FIRST_FIT = 0,
	NANO_SCAN_CAPACITY_BASED,
} nanov2_block_scan_policy_t;

// Minimum occupancy percentage for an ideal block.
#define DEFAULT_SCAN_MIN_CAPACITY 20

// Maximum occupancy percentage for an ideal block.
#define DEFAULT_SCAN_MAX_CAPACITY 80

// Maximum number of blocks to scan while looking for a best fit once a
// candidate block has been found. Value 0 means no limit.
#define DEFAULT_SCAN_LIMIT 10

// -- Madvise policy
typedef enum {
	NANO_MADVISE_IMMEDIATE = 0,
	NANO_MADVISE_WARNING_PRESSURE,
	NANO_MADVISE_CRITICAL_PRESSURE,
} nanov2_madvise_policy_t;

typedef struct nanov2_policy_config_s {
	// Determines the algorithm for scanning for the next allocation block.
	// Used in conjunction with nanov2_block_scan_capacity_min,
	// nanov2_block_scan_capacity_max and nanov2_block_scan_limit. Set from the
	// MallocNanoScanPolicy environment variable.
	nanov2_block_scan_policy_t block_scan_policy;

	// Minimum occupancy percentage for an ideal block.
	int block_scan_min_capacity;

	// Maximum occupancy percentage for an ideal block.
	int block_scan_max_capacity;

	// Maximum number of blocks to scan while looking for a best fit once a
	// candidate block has been found. Value 0 means no limit.
	int block_scan_limit;

	// Bitmask for size classes that are only allowed a single arena. Set from
	// the MallocNanoSingleArena environment variable.
	uint16_t single_arena_size_classes;

	// Madvise policy. Set from the MallocNanoMadvisePolicy environment variable
	nanov2_madvise_policy_t madvise_policy;
} nanov2_policy_config_t;

#if OS_VARIANT_NOTRESOLVED

// Madvise policy. Set from the MallocNanoMadvisePolicy environment variable.
MALLOC_NOEXPORT nanov2_madvise_policy_t nanov2_madvise_policy;

MALLOC_NOEXPORT nanov2_policy_config_t nanov2_policy_config = {
	.block_scan_policy = NANO_SCAN_CAPACITY_BASED,
	.block_scan_min_capacity = DEFAULT_SCAN_MIN_CAPACITY,
	.block_scan_max_capacity = DEFAULT_SCAN_MAX_CAPACITY,
	.block_scan_limit = DEFAULT_SCAN_LIMIT,
	.single_arena_size_classes = 0,
	.madvise_policy = NANO_MADVISE_IMMEDIATE,
};

#else  // OS_VARIANT_NOTRESOLVED

MALLOC_NOEXPORT extern nanov2_policy_config_t nanov2_policy_config;
MALLOC_NOEXPORT extern nanov2_madvise_policy_t nanov2_madvise_policy;

#endif // OS_VARIANT_NOTRESOLVED

// BLOCKS_PER_UNIT must be a power of two to make it possible to get the size
// class from a pointer reasonably cheaply. Do not change the value without
// fixing the code that depends on it.
#define BLOCKS_PER_UNIT_SHIFT 6
#define BLOCKS_PER_UNIT (1 << BLOCKS_PER_UNIT_SHIFT)

#if OS_VARIANT_NOTRESOLVED
// Number of units of each size class in an arena. The numbers here must add
// up to 64. One unit corresponds to BLOCKS_PER_UNIT blocks in the corresponding
// size class, so 64 units maps to a total of 64 * 64 = 4096 blocks and each
// block is 16K, making a total of 64MB, which is the size of an arena.
static int block_units_by_size_class[] = {
	2,	// 16-byte allocations (less 1 for the metadata block)
	10,	// 32-byte allocations
	11,	// 48-byte allocations
	10,	// 64-byte allocations
	5,	// 80-byte allocations
	3,	// 96-byte allocations
	3,	// 112-byte allocations
	4,	// 128-byte allocations
	3,	// 144-byte allocations
	2,	// 160-byte allocations
	2,	// 176-byte allocations
	2,	// 192-byte allocations
	2,	// 208-byte allocations
	2,	// 224-byte allocations
	1,	// 240-byte allocations
	2,	// 256-byte allocations
};

MALLOC_STATIC_ASSERT(
		sizeof(block_units_by_size_class)/sizeof(block_units_by_size_class[0])
				== NANO_SIZE_CLASSES,
		"Size of block_units_by_size_class is incorrect");

// Total of the number of blocks in all size classes. Currently this is 64.
#define TOTAL_BLOCK_UNITS (NANOV2_BLOCKS_PER_ARENA/BLOCKS_PER_UNIT)

// Offsets to the first and last blocks for each size class within an arena, in
// the logical address space. These tables are constructed from the values in
// the block_units_by_size_class table.
MALLOC_NOEXPORT int first_block_offset_by_size_class[NANO_SIZE_CLASSES];
MALLOC_NOEXPORT int last_block_offset_by_size_class[NANO_SIZE_CLASSES];

// Table mapping the part of a logical address that depends on size class to
// the size class. Also built from the block_units_by_size_class table.
MALLOC_NOEXPORT int ptr_offset_to_size_class[TOTAL_BLOCK_UNITS];

// Number of slots in a block, indexed by size class. Note that there is a small
// amount of wastage in some size classes because the block size is not always
// exactly divisible by the allocation size. The number of wasted bytes is shown
// in parentheses in the comments below.
MALLOC_NOEXPORT const int slots_by_size_class[] = {
	NANOV2_BLOCK_SIZE/(1 * NANO_REGIME_QUANTA_SIZE),  	// 16 bytes: 1024	(0)
	NANOV2_BLOCK_SIZE/(2 * NANO_REGIME_QUANTA_SIZE),	// 32 bytes: 512	(0)
	NANOV2_BLOCK_SIZE/(3 * NANO_REGIME_QUANTA_SIZE),	// 48 bytes: 341	(16)
	NANOV2_BLOCK_SIZE/(4 * NANO_REGIME_QUANTA_SIZE),	// 64 bytes: 256	(0)
	NANOV2_BLOCK_SIZE/(5 * NANO_REGIME_QUANTA_SIZE),	// 80 bytes: 204	(64)
	NANOV2_BLOCK_SIZE/(6 * NANO_REGIME_QUANTA_SIZE),	// 96 bytes: 170	(64)
	NANOV2_BLOCK_SIZE/(7 * NANO_REGIME_QUANTA_SIZE),	// 112 bytes: 146	(32)
	NANOV2_BLOCK_SIZE/(8 * NANO_REGIME_QUANTA_SIZE),	// 128 bytes: 128	(0)
	NANOV2_BLOCK_SIZE/(9 * NANO_REGIME_QUANTA_SIZE),	// 144 bytes: 113	(112)
	NANOV2_BLOCK_SIZE/(10 * NANO_REGIME_QUANTA_SIZE),	// 160 bytes: 102	(64)
	NANOV2_BLOCK_SIZE/(11 * NANO_REGIME_QUANTA_SIZE),	// 176 bytes: 93	(16)
	NANOV2_BLOCK_SIZE/(12 * NANO_REGIME_QUANTA_SIZE),	// 192 bytes: 85	(64)
	NANOV2_BLOCK_SIZE/(13 * NANO_REGIME_QUANTA_SIZE),	// 208 bytes: 78	(160)
	NANOV2_BLOCK_SIZE/(14 * NANO_REGIME_QUANTA_SIZE),	// 224 bytes: 73	(32)
	NANOV2_BLOCK_SIZE/(15 * NANO_REGIME_QUANTA_SIZE),	// 240 bytes: 68	(64)
	NANOV2_BLOCK_SIZE/(16 * NANO_REGIME_QUANTA_SIZE),	// 256 bytes: 64	(0)
};
#else // OS_VARIANT_NOTRESOLVED

extern int block_units_by_size_class[];
extern int ptr_offset_to_size_class[];
extern int first_block_offset_by_size_class[];
extern int last_block_offset_by_size_class[];
extern const int slots_by_size_class[];

#endif // OS_VARIANT_NOTRESOLVED

#pragma mark -
#pragma mark Conversion and Mapping Inlines

// nanov2_block_index_to_meta_index() and nanov2_meta_index_to_block_index()
// map between the index of a block in its arena and the index of the meta data
// header for that block in the metadata block. The mapping is not direct
// to avoid false sharing caused by CPUs that are using adjacent blocks
// writing to what would otherwise be adjacent meta data headers. The effect of
// these functions is to separate the meta data headers for adjacent blocks by
// at least the size of a cache line (assumed to be 64 bytes).
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_meta_index_t
nanov2_block_index_to_meta_index(nanov2_block_index_t block_index)
{
	return ((block_index >> 6) | (block_index << 6)) & 0xFFF;
}

static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_index_t
nanov2_meta_index_to_block_index(nanov2_meta_index_t block_meta_index)
{
	return ((block_meta_index >> 6) | (block_meta_index << 6)) & 0xFFF;
}

static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_meta_index_t
nanov2_metablock_meta_index(nanozonev2_t *nanozone)
{
	return nanov2_block_index_to_meta_index((nanov2_block_index_t)
			nanozone->aslr_cookie);
}

// Given a block metadata pointer, returns whether the block is active (that is,
// it is being used for allocations, it has allocations that have not been freed,
// or is waiting to be madvised).
static MALLOC_ALWAYS_INLINE MALLOC_INLINE boolean_t
nanov2_is_block_active(nanov2_block_meta_t block_meta)
{
	return block_meta.next_slot != SLOT_NULL
			&& block_meta.next_slot != SLOT_MADVISING
			&& block_meta.next_slot != SLOT_MADVISED;
}

#if OS_VARIANT_RESOLVED
// Given a block metadata pointer, returns whether an allocation could be
// attempted from it. Allocations are not allowed from blocks that have not yet
// been used (since such a block has not been assigned), is full or has been
// madvised.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE boolean_t
nanov2_can_allocate_from_block(nanov2_block_meta_t block_meta)
{
	return block_meta.in_use && block_meta.next_slot != SLOT_FULL;
}

// Given a pointer, returns whether it has the correct signature to be a
// Nano V2 address.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE boolean_t
nanov2_has_valid_signature(void *ptr)
{
	return (((uintptr_t)ptr) >> SHIFT_NANO_SIGNATURE) == NANOZONE_SIGNATURE;
}
#endif // OS_VARIANT_RESOLVED

// Converts a Nano V2 logical address to the corresponding real address.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE void *
nanov2_logical_address_to_ptr(nanozonev2_t *nanozone, void *laddr)
{
	return (void *)(((uintptr_t)laddr) ^ nanozone->aslr_cookie_aligned);
}

// Gets the maximum allocation size for a given size class.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE int
nanov2_size_from_size_class(nanov2_size_class_t size_class)
{
	return (size_class + 1) * NANO_REGIME_QUANTA_SIZE;
}

#if OS_VARIANT_RESOLVED
// Given an allocation size, returns the corresponding size class. It is the
// responsibility of the caller to ensure that the size is valid. Returned
// value is zero-based.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_size_class_t
nanov2_size_class_from_size(size_t size)
{
	return (nanov2_size_class_t)howmany(size, NANO_REGIME_QUANTA_SIZE) - 1;
}
#endif // OS_VARIANT_RESOLVED

#if OS_VARIANT_RESOLVED
// Given a pointer that is assumed to be in the Nano zone, returns the address
// of its containing block. Works for both real and logical pointers and returns
// a pointer of the same type.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_t *
nanov2_block_address_for_ptr(void *ptr)
{
	return (void *)(((uintptr_t)ptr) & NANOV2_BLOCK_ADDRESS_MASK);
}
#endif // OS_VARIANT_RESOLVED

// Given a pointer that is assumed to be in the Nano zone, returns the address
// of its containing arena. Works for both real and logical pointers.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_arena_t *
nanov2_arena_address_for_ptr(void *ptr)
{
	return (void *)(((uintptr_t)ptr) & NANOV2_ARENA_ADDRESS_MASK);
}

#if OS_VARIANT_RESOLVED
// Given a pointer that is assumed to be in the Nano zone, returns the address
// of its containing region. Works for both real and logical pointers.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_region_t *
nanov2_region_address_for_ptr(void *ptr)
{
	return (nanov2_region_t *)(((uintptr_t)ptr) & NANOV2_REGION_ADDRESS_MASK);
}
#endif // OS_VARIANT_RESOLVED

// Given a pointer that is assumed to be in the Nano zone, returns the real
// address of its metadata block. Works for both real and logical pointers.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_arena_metablock_t *
nanov2_metablock_address_for_ptr(nanozonev2_t *nanozone, void *ptr)
{
	// The metadata block is the first logical block in the arena, so its
	// logical address is that of the arena. To get a real pointer, we map it
	// through nanov2_logical_address_to_ptr().
	return (nanov2_arena_metablock_t *)nanov2_logical_address_to_ptr(nanozone,
			nanov2_arena_address_for_ptr(ptr));
}

#if OS_VARIANT_RESOLVED
// Given a pointer to a block_metap_t for a block, returns a pointer to the
// block itself.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_t *
nanov2_block_address_from_meta_ptr(nanozonev2_t *nanozone,
		nanov2_block_meta_t *block_metap)
{
	nanov2_block_t *meta_block = nanov2_block_address_for_ptr(block_metap);
	nanov2_arena_t *arena = nanov2_arena_address_for_ptr(block_metap);

	// Get the block's index and use that to get the address of the block.
	nanov2_meta_index_t meta_index =
		(nanov2_meta_index_t)(block_metap - (nanov2_block_meta_t *)meta_block);
	nanov2_block_index_t block_index = nanov2_meta_index_to_block_index(meta_index);
	return &arena->blocks[block_index];
}
#endif // OS_VARIANT_RESOLVED

// Given the index of a block_metap_t for a block, returns a pointer to the
// block itself.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_t *
nanov2_block_address_from_meta_index(nanozonev2_t *nanozone,
		nanov2_arena_t *arena, nanov2_meta_index_t meta_index)
{
	nanov2_block_index_t block_index = nanov2_meta_index_to_block_index(meta_index);
	return &arena->blocks[block_index];
}

// Given a pointer that is assumed to be in the nanozone, returns the index
// of its containing block within its hosting arena. Works for both logical and
// real pointers and returns an index in the corresponding address space.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_index_t
nanov2_block_index_for_ptr(void *ptr)
{
	return (nanov2_block_index_t)(((uintptr_t)ptr) >> NANOV2_OFFSET_BITS)
			& ((1 << NANOV2_BLOCK_BITS) - 1);
}

#if OS_VARIANT_RESOLVED
// Given a pointer that is assumed to be in the nanozone, returns a pointer to
// the meta data for its containing block. Expects ptr be a real address.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_meta_t *
nanov2_meta_ptr_for_ptr(nanozonev2_t *nanozone, void *ptr)
{
	nanov2_arena_metablock_t *meta_block = nanov2_metablock_address_for_ptr(
			nanozone, ptr);
	nanov2_block_index_t block_index = nanov2_block_index_for_ptr(ptr);
	nanov2_meta_index_t meta_index = nanov2_block_index_to_meta_index(block_index);
	return &meta_block->arena_block_meta[meta_index];
}
#endif // OS_VARIANT_RESOLVED

// Given a region pointer, returns the address of the first arena in the region.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_arena_t *
nanov2_first_arena_for_region(nanov2_region_t *region)
{
	// The first arena is colocated with the region itself.
	return (nanov2_arena_t *)region;
}

// Given a region pointer, returns a pointer to the arena after the last
// active arena in the region.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_arena_t *
nanov2_limit_arena_for_region(nanozonev2_t *nanozone, nanov2_region_t *region)
{
	// The first arena is colocated with the region itself.
	nanov2_arena_t *limit_arena;
	if (region == nanozone->current_region_base) {
		limit_arena = nanozone->current_region_next_arena;
	} else {
		limit_arena = nanov2_first_arena_for_region(region + 1);
	}
	return limit_arena;
}

// Given a region pointer, returns the address of the linkage structure for
// that region. The linkage structure is stored in the first entry of the
// metadata block of the first arena in the region.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_region_linkage_t *
nanov2_region_linkage_for_region(nanozonev2_t *nanozone, nanov2_region_t *region)
{
	nanov2_arena_metablock_t *first_metadata_block =
			nanov2_metablock_address_for_ptr(nanozone, region);
	return (nanov2_region_linkage_t *)&first_metadata_block->arena_block_meta[
			nanov2_metablock_meta_index(nanozone)];
}

#if OS_VARIANT_RESOLVED
// Given a pointer to a region, returns a pointer to the region that follows it,
// or NULL if there isn't one.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_region_t *
nanov2_next_region_for_region(nanozonev2_t *nanozone, nanov2_region_t *region)
{
	nanov2_region_linkage_t *linkage =
			nanov2_region_linkage_for_region(nanozone, region);
	int offset = linkage->next_region_offset;
	return offset ? region + offset : NULL;
}
#endif // OS_VARIANT_RESOLVED

#if OS_VARIANT_NOTRESOLVED
// Given a pointer to a region, returns a pointer to the region that follows it,
// or NULL if there isn't one. This variant is used when mapping the nanozone
// for another process.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_region_t *
nanov2_next_region_for_region_offset(nanozonev2_t *nanozone,
        nanov2_region_t *region, off_t region_offset)
{
    nanov2_region_linkage_t *linkage =
            nanov2_region_linkage_for_region(nanozone, region);
    nanov2_region_linkage_t *mapped_linkage = (nanov2_region_linkage_t *)
        ((uintptr_t)linkage + region_offset);
    int offset = mapped_linkage->next_region_offset;
    return offset ? region + offset : NULL;
}
#endif // OS_VARIANT_NOTRESOLVED

// Given the index of a slot in a block of a given size and the base address of
// the block, returns a pointer to the start of the slot. This works for both
// real and logical block pointers and returns a pointer of the same type.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE void *
nanov2_slot_in_block_ptr(nanov2_block_t *block, nanov2_size_class_t size_class,
		int slot_index)
{
	return (void *)((uintptr_t)block +
			nanov2_size_from_size_class(size_class) * slot_index);
}

#if OS_VARIANT_RESOLVED
// Given the base address of a block, the size class for the block and a pointer,
// returns the index of the slot represented by the pointer. It is assumed that
// the pointer is slot-aligned and is within the bounds of the block.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE int
nanov2_slot_index_in_block(nanov2_block_t *block, nanov2_size_class_t size_class,
		void *ptr)
{
	return (int)((uintptr_t)ptr - (uintptr_t)block)/
			(nanov2_size_from_size_class(size_class));
}
#endif // OS_VARIANT_RESOLVED

// Given a (real) pointer, gets the size class of its containing block. Assumes
// that the pointer is in a valid region, arena and block.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_size_class_t
nanov2_size_class_for_ptr(nanozonev2_t *nanozone, void *ptr)
{
	// To get the size class, we need to convert the block number from
	// physical to logical, since the ptr_offset_to_size_class table is
	// indexed by logical block.
	nanov2_block_index_t block =
			(int)(nanov2_block_index_for_ptr(ptr) ^ nanozone->aslr_cookie);
	return ptr_offset_to_size_class[block >> BLOCKS_PER_UNIT_SHIFT];
}

#if OS_VARIANT_NOTRESOLVED

// Given a meta data index, gets the size class of the corresponding block.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_size_class_t
nanov2_size_class_for_meta_index(nanozonev2_t *nanozone, nanov2_meta_index_t meta_index)
{
	// To get the size class, we need to get the block index from meta index
	// and then convert it from real to logical, since the
	// ptr_offset_to_size_class table is indexed by logical block.
	nanov2_block_index_t block_index = nanov2_meta_index_to_block_index(meta_index);
	int logical_block_index = (int)(block_index ^ nanozone->aslr_cookie);
	return ptr_offset_to_size_class[logical_block_index >> BLOCKS_PER_UNIT_SHIFT];
}
#endif // OS_VARIANT_NOTRESOLVED

#if OS_VARIANT_RESOLVED
// Given a size class and an arena, returns a pointer to the metadata for the
// first block for that size class in the arena, ignoring the metadata block.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_meta_t *
nanov2_first_block_for_size_class_in_arena(nanozonev2_t *nanozone,
		nanov2_size_class_t size_class, nanov2_arena_t *arena)
{
	int block_offset = first_block_offset_by_size_class[size_class];
	nanov2_arena_metablock_t *meta_blockp =
			nanov2_metablock_address_for_ptr(nanozone, arena);
	nanov2_block_index_t block_index =
			(nanov2_block_index_t)(block_offset ^ nanozone->aslr_cookie);
	nanov2_meta_index_t meta_index = nanov2_block_index_to_meta_index(block_index);
	return &meta_blockp->arena_block_meta[meta_index];
}

// Given a pointer to the metadata for a block in a given size class, returns
// a pointer to the metadata for the next block, wrapping from the last block
// to the first if necessary.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_meta_t *
nanov2_next_block_for_size_class(nanozonev2_t *nanozone,
		nanov2_size_class_t size_class, nanov2_block_meta_t *meta_blockp,
		boolean_t *wrapped)
{
	// To find the next block, get the index of the current block, which is in
	// the real address space, unscramble it to get a logical block number,
	// add 1 to it, wrapping if necessary, then scramble the result.
	nanov2_block_meta_t *base_meta_blockp =
			(nanov2_block_meta_t *)(((uintptr_t)meta_blockp) & (NANOV2_BLOCK_ADDRESS_MASK));
	nanov2_meta_index_t meta_index = (int)(meta_blockp - base_meta_blockp);
	nanov2_block_index_t block_index = nanov2_meta_index_to_block_index(meta_index);
	block_index ^= nanozone->aslr_cookie;  // Unscramble
	int last_offset = last_block_offset_by_size_class[size_class];
	if (wrapped) *wrapped = block_index == last_offset;
	block_index = block_index == last_offset ?
			first_block_offset_by_size_class[size_class] : block_index + 1;
	block_index = (nanov2_block_index_t)(block_index ^ nanozone->aslr_cookie);
	meta_index = nanov2_block_index_to_meta_index(block_index);
	return &base_meta_blockp[meta_index];
}

// Given a pointer to the metadata for a block in a given size class, returns
// a pointer to the metadata for the previous block, wrapping from the first
// block to the last if necessary.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_meta_t *
nanov2_previous_block_for_size_class(nanozonev2_t *nanozone,
		nanov2_size_class_t size_class, nanov2_block_meta_t *meta_blockp,
		boolean_t *wrapped)
{
	// To find the previous block, get the index of the current block, which is
	// in the real address space, unscramble it to get a logical block number,
	// subtract 1 from it, wrapping if necessary, then scramble the result.
	nanov2_block_meta_t *base_meta_blockp =
			(nanov2_block_meta_t *)(((uintptr_t)meta_blockp) & (NANOV2_BLOCK_ADDRESS_MASK));
	nanov2_meta_index_t meta_index = (int)(meta_blockp - base_meta_blockp);
	nanov2_block_index_t block_index = nanov2_meta_index_to_block_index(meta_index);
	block_index ^= nanozone->aslr_cookie;  // Unscramble
	int first_offset = first_block_offset_by_size_class[size_class];
	if (wrapped) *wrapped = block_index == first_offset;
	block_index = block_index == first_offset ?
			last_block_offset_by_size_class[size_class] : block_index - 1;
	block_index = (nanov2_block_index_t)(block_index ^ nanozone->aslr_cookie);
	meta_index = nanov2_block_index_to_meta_index(block_index);
	return &base_meta_blockp[meta_index];
}

// Turns off the in-use bit in the meta data for a given block.
static MALLOC_ALWAYS_INLINE MALLOC_INLINE void
nanov2_turn_off_in_use(nanov2_block_meta_t *block_metap)
{
	// TODO: find a way to do this that is more efficient and readable.
	static nanov2_block_meta_t mask = {
		.in_use = 0,
		.next_slot = ~0,
		.free_count = ~0,
		.gen_count = ~0,
	};
	os_atomic_and((uint32_t *)block_metap, *(uint32_t *)&mask, relaxed);
}

#pragma mark -
#pragma mark Policy Functions

// Gets the index of the block in the zone's current_block array from which
// allocations should be made. This function should be replaced to implement
// a different strategy (e.g. for E- vs P-cores).
static MALLOC_ALWAYS_INLINE MALLOC_INLINE int
nanov2_get_allocation_block_index(void)
{
#if CONFIG_NANO_USES_HYPER_SHIFT
	if (os_likely(nano_common_max_magazines_is_ncpu)) {
		// Default case is max magazines == physical number of CPUs, which
		// must be > _os_cpu_number() >> hyper_shift, so the modulo
		// operation is not required.
		return _os_cpu_number() >> hyper_shift;
	}
#else // CONFIG_NANO_USES_HYPER_SHIFT
	if (os_likely(nano_common_max_magazines_is_ncpu)) {
		// Default case is max magazines == logical number of CPUs, which
		// must be > _os_cpu_number() so the modulo operation is not required.
		return _os_cpu_number();
	}
#endif // CONFIG_NANO_USES_HYPER_SHIFT

	unsigned int shift = 0;
#if CONFIG_NANO_USES_HYPER_SHIFT
	shift = hyper_shift;
#endif // CONFIG_NANO_USES_HYPER_SHIFT

	if (os_likely(_os_cpu_number_override == -1)) {
		return (_os_cpu_number() >> shift) % nano_common_max_magazines;
	}
	return (_os_cpu_number_override >> shift) % nano_common_max_magazines;
}
#endif // OS_VARIANT_RESOLVED

#pragma mark -
#pragma mark Allocator Initialization

#if OS_VARIANT_NOTRESOLVED

static const char madvise_policy_env[] = "MallocNanoMadvisePolicy";
static const char madvise_policy_bootarg[] = "nanov2_madvise_policy";
static const char madvise_immediate[] = "immediate";
static const char madvise_warning[] = "warning";
static const char madvise_critical[] = "critical";

static const char single_arena_env[] = "MallocNanoSingleArena";
static const char single_arena_bootarg[] = "nanov2_single_arena";

static const char scan_policy_env[] = "MallocNanoScanPolicy";
static const char scan_policy_bootarg[] = "nanov2_scan_policy";

static const char size_class_blocks_env[] = "MallocNanoSizeClassBlocks";
static const char size_class_blocks_bootarg[] = "nanov2_size_class_blocks";

// Parse and set the madvise policy setting. If ptr is NULL, sets the default
// policy.
static void
nanov2_set_madvise_policy(const char *name, const char *ptr)
{
	nanov2_madvise_policy_t madvise_policy = NANO_MADVISE_IMMEDIATE;
	if (ptr) {
		if (!strncmp(ptr, madvise_immediate, sizeof(madvise_immediate) - 1)) {
			madvise_policy = NANO_MADVISE_IMMEDIATE;
		} else if (!strncmp(ptr, madvise_warning, sizeof(madvise_warning) - 1)) {
			madvise_policy = NANO_MADVISE_WARNING_PRESSURE;
		} else if (!strncmp(ptr, madvise_critical, sizeof(madvise_critical) - 1)) {
			madvise_policy = NANO_MADVISE_CRITICAL_PRESSURE;
		} else {
			malloc_report(ASL_LEVEL_ERR,
					"%s value (%s) invalid - ignored.\n", name, ptr);
		}
	}
	nanov2_madvise_policy = madvise_policy;
}

// Parse and set the list of size classes that are allowed only one arena. If
// ptr is NULL, no size classes are restricted to a single arena,
// Format is a list of sizes separated by colons (e.g. 16:240). Each size must
// be a multiple of NANO_REGIME_QUANTA_SIZE and must be between 16 and 256.
static void
nanov2_set_single_arena_size_classes(const char *name, const char *ptr)
{
	uint16_t single_arena_size_classes = 0;
	if (ptr) {
		const char *value = ptr;
		const char *endp;
		boolean_t failed = FALSE;
		while (*ptr) {
			long size = malloc_common_convert_to_long(ptr, &endp);
			if (endp != ptr) {
				if (*endp && *endp != ':') {
					failed = TRUE;
					break;
				}
				if (size > NANO_MAX_SIZE || size < NANO_REGIME_QUANTA_SIZE ||
						(size % NANO_REGIME_QUANTA_SIZE) != 0) {
					failed = TRUE;
					break;
				}
				single_arena_size_classes |=
						1 << ((size/NANO_REGIME_QUANTA_SIZE) - 1);
			} else {
				failed = true;
				break;
			}
			if (!*endp) {
				break;
			}
			ptr = endp + 1;
		}
		if (failed) {
			malloc_report(ASL_LEVEL_ERR,
					"%s value (%s) invalid - ignored.\n", name, value);
			single_arena_size_classes = 0;
		}
	}
	nanov2_policy_config.single_arena_size_classes = single_arena_size_classes;
}

// Parse and set the block scan policy setting. If ptr is NULL, the default
// policy is used. Format is either "firstfit" or "minXX:maxYY:limZZ", where
// XX, YY and ZZ are numbers, XX and YY must be between 0 and 100 and XX must
// not be greater than YY. min, max and lim may appear in any order or may be
// omitted to get default values.
static void
nanov2_set_block_scan_policy(const char *name, const char *ptr)
{
	static char first_fit_key[] = "firstfit";
	static char min_key[] = "min";
	static char max_key[] = "max";
	static char lim_key[] = "lim";

	nanov2_block_scan_policy_t block_scan_policy = NANO_SCAN_CAPACITY_BASED;
	int scan_min_capacity = DEFAULT_SCAN_MIN_CAPACITY;
	int scan_max_capacity = DEFAULT_SCAN_MAX_CAPACITY;
	int scan_limit = DEFAULT_SCAN_LIMIT;
	const char *endp;
	boolean_t failed = FALSE;
	boolean_t min_found = FALSE;
	boolean_t max_found = FALSE;
	boolean_t lim_found = FALSE;
	const char *value = ptr;

	if (ptr) {
		if (!strcmp(ptr, first_fit_key)) {
			block_scan_policy = NANO_SCAN_FIRST_FIT;
		} else {
			while (!failed && ptr && *ptr) {
				if (!strncmp(ptr, min_key, sizeof(min_key) - 1) && !min_found) {
					min_found = TRUE;
					ptr += sizeof(min_key) - 1;
					long value = malloc_common_convert_to_long(ptr, &endp);
					if (ptr != endp && value >= 0 && value <= 100) {
						scan_min_capacity = (int)value;
						ptr = endp;
					} else {
						failed = TRUE;
					}
				} else if (!strncmp(ptr, max_key, sizeof(max_key) - 1)
						&& !max_found) {
					max_found = TRUE;
					ptr += sizeof(max_key) - 1;
					long value = malloc_common_convert_to_long(ptr, &endp);
					if (ptr != endp && value >= 0 && value <= 100) {
						scan_max_capacity = (int)value;
						ptr = endp;
					} else {
						failed = TRUE;
					}
				} else if (!strncmp(ptr, lim_key, sizeof(lim_key) - 1)
						&& !lim_found) {
					lim_found = TRUE;
					ptr += sizeof(lim_key) - 1;
					long value = malloc_common_convert_to_long(ptr, &endp);
					if (ptr != endp && value >= 0) {
						scan_limit = (int)value;
						ptr = endp;
					} else {
						failed = TRUE;
					}
				} else {
					failed = TRUE;
				}
				if (*ptr) {
					if (*ptr == ':') {
						ptr++;
					} else {
						failed = TRUE;
					}
				}
			}

			if (!failed && scan_min_capacity > scan_max_capacity) {
				failed = TRUE;
			}
		}
	}

	if (!failed) {
		nanov2_policy_config.block_scan_policy = block_scan_policy;
		nanov2_policy_config.block_scan_min_capacity = scan_min_capacity;
		nanov2_policy_config.block_scan_max_capacity = scan_max_capacity;
		nanov2_policy_config.block_scan_limit = scan_limit;
	} else {
		malloc_report(ASL_LEVEL_ERR, "%s value (%s) invalid - ignored.\n",
				name, value);
	}
}

// Configures the nanov2_blocks_by_size_class array. If ptr is not NULL and
// *ptr is not empty, it is expected to be a list of 16 positive integers
// separated by commas that sum to TOTAL_BLOCK_UNITS (which is currently 64).
// For example, as an environment variable:
// 	MallocNanoSizeClassBlocks=2,7,6,6,6,5,5,5,5,2,2,2,2,2,6,1
// or as a boot argument:
//  nanov2_size_class_blocks=2,7,6,6,6,5,5,5,5,2,2,2,2,2,6,1
static void
nanov2_set_blocks_by_size_class(const char *name, const char *ptr)
{
	int new_total_block_units = 0;
	int new_blocks_by_size_class[NANO_SIZE_CLASSES];
	MALLOC_STATIC_ASSERT(
			sizeof(new_blocks_by_size_class) == sizeof(block_units_by_size_class),
			"Size mismatch in nanov2_set_blocks_by_size_class()");
	const char *endp;
	const char *sptr = ptr;
	for (int i = 0; i < NANO_SIZE_CLASSES; i++) {
		int count = (int)malloc_common_convert_to_long(ptr, &endp);
		char separator = i == NANO_SIZE_CLASSES - 1 ? '\0' : ',';
		if (endp == ptr || *endp != separator || count > TOTAL_BLOCK_UNITS) {
			malloc_report(ASL_LEVEL_ERR,
					"%s value invalid: [%s] - ignored.\n", name, sptr);
			return;
		}
		new_blocks_by_size_class[i] = count;
		new_total_block_units += count;
		ptr = endp + 1;
	}

	if (new_total_block_units != TOTAL_BLOCK_UNITS) {
		malloc_report(ASL_LEVEL_ERR,
				"%s value invalid - values must sum to %d, not %d - ignored.\n",
				name, TOTAL_BLOCK_UNITS, new_total_block_units);
	} else {
		memcpy(block_units_by_size_class, new_blocks_by_size_class,
				sizeof(block_units_by_size_class));
	}
}

// First stage initialization. Called during libSystem initialization.
// Reads environment variables and boot arguments and sets the madvise policy,
// single arena list and the block scan policy. Environment variables override
// boot arguments.
void
nanov2_init(const char *envp[], const char *apple[], const char *bootargs)
{
	// Get and process the boot args and environment variables.
	char value_buf[256];
	const char *value = _simple_getenv(envp, madvise_policy_env);
	const char *name = madvise_policy_env;
	if (!value) {
		value = malloc_common_value_for_key(bootargs, madvise_policy_bootarg);
		if (value) {
			name = madvise_policy_bootarg;
		}
	}
	nanov2_set_madvise_policy(name, value);

	name = single_arena_env;
	value = _simple_getenv(envp, single_arena_env);
	if (!value) {
		value = malloc_common_value_for_key_copy(bootargs, single_arena_bootarg,
				value_buf, sizeof(value_buf));
		if (value) {
			name = single_arena_bootarg;
		}
	}
	nanov2_set_single_arena_size_classes(name, value);

	name = scan_policy_env;
	value = _simple_getenv(envp, scan_policy_env);
	if (!value) {
		value = malloc_common_value_for_key_copy(bootargs, scan_policy_bootarg,
				value_buf, sizeof(value_buf));
		if (value) {
			name = scan_policy_bootarg;
		}
	}
	nanov2_set_block_scan_policy(name, value);

	name = size_class_blocks_env;
	value = _simple_getenv(envp, size_class_blocks_env);
	if (!value) {
		value = malloc_common_value_for_key_copy(bootargs, size_class_blocks_bootarg,
				value_buf, sizeof(value_buf));
		if (value) {
			name = size_class_blocks_bootarg;
		}
	}
	if (value) {
		nanov2_set_blocks_by_size_class(name, value);
	}
}

static void
nanov2_configure_once(void *context MALLOC_UNUSED)
{
	// Check that the block_units_by_size_class table is consistent.
	int total_blocks = 0;
	for (int i = 0; i < NANO_SIZE_CLASSES; i++) {
		total_blocks += block_units_by_size_class[i] * BLOCKS_PER_UNIT;
	}
	MALLOC_ASSERT(total_blocks == NANOV2_BLOCKS_PER_ARENA);

	// Build the first_block_offset_by_size_class and
	// last_block_offset_by_size_class tables. The first entry is special
	// because block 0 is reserved for the metadata block, so the first offset
	// is 1 and the number of blocks allocated is reduced by 1.
	int next_offset = 1;
	first_block_offset_by_size_class[0] = next_offset;
	next_offset = block_units_by_size_class[0] * BLOCKS_PER_UNIT;
	last_block_offset_by_size_class[0] = next_offset - 1;

	for (int i = 1; i < NANO_SIZE_CLASSES; i++) {
		first_block_offset_by_size_class[i] = next_offset;
		next_offset += block_units_by_size_class[i] * BLOCKS_PER_UNIT;
		last_block_offset_by_size_class[i] = next_offset - 1;
	}
	MALLOC_ASSERT(next_offset == NANOV2_BLOCKS_PER_ARENA);

	// Construct the ptr_offset_to_size_class map, which maps the part of the
	// logical address that depends on size class to the corresponding size
	// class. This would be a simple mask operation if all size classes were of
	// equal size, but sadly they are not.
	int next_index = 0;
	for (int i = 0; i < NANO_SIZE_CLASSES; i++) {
		int block_units = block_units_by_size_class[i];
		for (int j = 0; j < block_units; j++) {
			ptr_offset_to_size_class[next_index++] = i;
		}
	}
	MALLOC_ASSERT(next_index == NANOV2_BLOCKS_PER_ARENA/BLOCKS_PER_UNIT);
}

static os_once_t nanov2_config_predicate;

void
nanov2_configure(void)
{
	os_once(&nanov2_config_predicate, NULL, nanov2_configure_once);
}
#endif // OS_VARIANT_NOTRESOLVED

#pragma mark -
#pragma mark Zone Functions

#if OS_VARIANT_RESOLVED
// Returns the allocation size for a pointer. Uses nanov2_pointer_size() to
// determine whether the pointer is for a Nano V2 allocation and, if not,
// delegates to the helper zone. Returns 0 if the pointer is not to memory
// allocated by Nano V2 or attributable to the helper zone.
MALLOC_NOEXPORT size_t
nanov2_size(nanozonev2_t *nanozone, const void *ptr)
{
	size_t size = nanov2_pointer_size(nanozone, (void *)ptr, FALSE);
	return size ? size : nanozone->helper_zone->size(nanozone->helper_zone, ptr);
}

MALLOC_NOEXPORT void *
nanov2_malloc(nanozonev2_t *nanozone, size_t size)
{
	size_t rounded_size = _nano_common_good_size(size);
	if (rounded_size <= NANO_MAX_SIZE) {
		void *ptr = nanov2_allocate(nanozone, rounded_size, FALSE);
		if (ptr) {
			if (os_unlikely(size && (nanozone->debug_flags & MALLOC_DO_SCRIBBLE))) {
				memset(ptr, SCRIBBLE_BYTE, size);
			}
			return ptr;
		}
	}

	// If we reach this point, we couldn't allocate, so delegate to the
	// helper zone.
	return nanozone->helper_zone->malloc(nanozone->helper_zone, size);
}

MALLOC_NOEXPORT void
nanov2_free_definite_size(nanozonev2_t *nanozone, void *ptr, size_t size)
{
	// Check whether it's a Nano pointer and get the size. We should only get
	// here if it is and furthermore we already know that "size" is the actual
	// rounded size, so don't waste time rechecking that. This is just a
	// sanity check.
	if (ptr && nanov2_has_valid_signature(ptr)) {
		if (os_unlikely(nanozone->debug_flags & MALLOC_DO_SCRIBBLE)) {
			memset(ptr, SCRABBLE_BYTE, size);
		}
		nanov2_free_to_block(nanozone, ptr, nanov2_size_class_from_size(size));
		return;
	}
	return nanozone->helper_zone->free_definite_size(nanozone->helper_zone, ptr,
			size);
}

MALLOC_NOEXPORT void
nanov2_free(nanozonev2_t *nanozone, void *ptr)
{
	if (ptr && nanov2_has_valid_signature(ptr)) {
		// Check whether it's a Nano pointer and get the size. If it's not
		// Nano, pass it to the helper zone.
		size_t size = nanov2_pointer_size(nanozone, ptr, FALSE);
		if (size) {
			if (os_unlikely(nanozone->debug_flags & MALLOC_DO_SCRIBBLE)) {
				memset(ptr, SCRABBLE_BYTE, size);
			}
			nanov2_free_to_block(nanozone, ptr, nanov2_size_class_from_size(size));
			return;
		}
	}
	return nanozone->helper_zone->free(nanozone->helper_zone, ptr);
}

MALLOC_NOEXPORT void *
nanov2_calloc(nanozonev2_t *nanozone, size_t num_items, size_t size)
{
	size_t total_bytes;
	if (calloc_get_size(num_items, size, 0, &total_bytes)) {
		return NULL;
	}
	size_t rounded_size = _nano_common_good_size(total_bytes);
	if (total_bytes <= NANO_MAX_SIZE) {
		void *ptr = nanov2_allocate(nanozone, rounded_size, TRUE);
		if (ptr) {
			return ptr;
		}
	}

	// If we reach this point, we couldn't allocate, so delegate to the
	// helper zone.
	return nanozone->helper_zone->calloc(nanozone->helper_zone, 1, total_bytes);
}
#endif // OS_VARIANT_RESOLVED

#if OS_VARIANT_NOTRESOLVED
static void *
nanov2_valloc(nanozonev2_t *nanozone, size_t size)
{
	// Always delegate this to the helper zone.
	return nanozone->helper_zone->valloc(nanozone->helper_zone, size);
}
#endif // OS_VARIANT_NOTRESOLVED

#if OS_VARIANT_RESOLVED
MALLOC_NOEXPORT void *
nanov2_realloc(nanozonev2_t *nanozone, void *ptr, size_t new_size)
{
	// If we are given a NULL pointer, just allocate memory of the requested
	// size.
	if (ptr == NULL) {
		return nanov2_malloc(nanozone, new_size);
	}

	size_t old_size = nanov2_pointer_size(nanozone, ptr, FALSE);
	if (!old_size) {
		// Not a Nano pointer - let the helper deal with it
		return nanozone->helper_zone->realloc(nanozone->helper_zone, ptr, new_size);
	}

	void *new_ptr;
	if (new_size > NANO_MAX_SIZE) {
		// Too large for Nano. Try to allocate from the helper zone.
		new_ptr = nanozone->helper_zone->malloc(nanozone->helper_zone, new_size);
		if (!new_ptr) {
			// Failed to allocate - leave the existing allocation alone.
			return NULL;
		}
	} else if (!new_size) {
		// Resizing to zero. Free the existing memory and explicitly allocate
		// zero bytes.
		nanov2_free(nanozone, ptr);
		return nanov2_malloc(nanozone, 0);
	} else {
		size_t new_good_size = _nano_common_good_size(new_size);
		if (new_good_size > old_size || new_good_size <= old_size/2) {
			// Growing or shrinking to less than half size - we need to
			// reallocate.
			new_ptr = nanov2_malloc(nanozone, new_good_size);
			if (!new_ptr) {
				// Failed to allocate - leave the existing allocation alone.
				return NULL;
			}
		} else {
			// Same size or shrinking by less than half size. Keep the same
			// allocation and clear the area that's being released.
			if (new_size != old_size) {
				MALLOC_ASSERT(new_size < old_size);
				if (os_unlikely(nanozone->debug_flags & MALLOC_DO_SCRIBBLE)) {
					memset(ptr + new_size, SCRABBLE_BYTE, old_size - new_size);
				}
			}
			return ptr;
		}
	}

	// If we reach this point, we allocated new memory. Copy the existing
	// content to the new location and release the old allocation.
	MALLOC_ASSERT(new_ptr);
	memcpy(new_ptr, ptr, MIN(old_size, new_size));
	nanov2_free(nanozone, ptr);

	return new_ptr;
}
#endif // OS_VARIANT_RESOLVED

#if OS_VARIANT_NOTRESOLVED
static void
nanov2_destroy(nanozonev2_t *nanozone)
{
	nanozone->helper_zone->destroy(nanozone->helper_zone);
	nano_common_deallocate_pages((void *)nanozone, NANOZONEV2_ZONE_PAGED_SIZE,
			nanozone->debug_flags);
}
#endif // OS_VARIANT_NOTRESOLVED

#if OS_VARIANT_RESOLVED
MALLOC_NOEXPORT boolean_t
nanov2_claimed_address(nanozonev2_t *nanozone, void *ptr)
{
	return nanov2_pointer_size(nanozone, ptr, TRUE)
			|| malloc_zone_claimed_address(nanozone->helper_zone, ptr);
}

MALLOC_NOEXPORT unsigned
nanov2_batch_malloc(nanozonev2_t *nanozone, size_t size, void **results,
		unsigned count)
{
	unsigned allocated = 0;
	size_t rounded_size = _nano_common_good_size(size);
	if (rounded_size <= NANO_MAX_SIZE) {
		while (allocated < count) {
			void *ptr = nanov2_allocate(nanozone, rounded_size, FALSE);
			if (!ptr) {
				break;
			}

			*results++ = ptr;
			allocated++;
		}
		if (allocated == count) {
			// Allocated everything.
			return allocated;
		}
	}

	// We could not allocate everything. Let the helper zone do the rest.
	return allocated + nanozone->helper_zone->batch_malloc(
			nanozone->helper_zone, size, results, count - allocated);
}

MALLOC_NOEXPORT void
nanov2_batch_free(nanozonev2_t *nanozone, void **to_be_freed, unsigned count)
{
	if (count) {
		while (count--) {
			void *ptr = to_be_freed[count];
			if (ptr) {
				nanov2_free(nanozone, ptr);
			}
		}
	}
}
#endif // OS_VARIANT_RESOLVED

#if OS_VARIANT_NOTRESOLVED
static void *
nanov2_memalign(nanozonev2_t *nanozone, size_t alignment, size_t size)
{
	// Always delegate this to the helper zone.
	return nanozone->helper_zone->memalign(nanozone->helper_zone, alignment,
			size);
}
#endif // OS_VARIANT_NOTRESOLVED

#if OS_VARIANT_RESOLVED

size_t
nanov2_pressure_relief(nanozonev2_t *nanozone, size_t goal)
{
	if (nanov2_madvise_policy != NANO_MADVISE_WARNING_PRESSURE
			&& nanov2_madvise_policy != NANO_MADVISE_CRITICAL_PRESSURE) {
		// In the current implementation, we only get called on warning, so
		// act if the policy is either warning or critical. We would need to
		// add a new zone entry point to respond to critical.
		return 0;
	}
	const char *name = nanozone->basic_zone.zone_name;
	MAGMALLOC_PRESSURERELIEFBEGIN((void *)nanozone, name, (int)goal);
	MALLOC_TRACE(TRACE_nano_memory_pressure | DBG_FUNC_START,
			(uint64_t)nanozone, goal, 0, 0);
	size_t total = 0;

	// Loop over all arenas madvising blocks that are marked as madvisable,
	// until we reach our goal.
	nanov2_region_t *region = nanozone->first_region_base;
	nanov2_meta_index_t metablock_meta_index = nanov2_metablock_meta_index(nanozone);
	while (region) {
		nanov2_arena_t *arena = nanov2_first_arena_for_region(region);
		nanov2_arena_t *arena_after_region = nanov2_limit_arena_for_region(nanozone, region);
		while (arena < arena_after_region) {
			// Scan all of the blocks in the arena, skipping the metadata block.
			nanov2_arena_metablock_t *meta_blockp =
					nanov2_metablock_address_for_ptr(nanozone, arena);
			nanov2_block_meta_t *block_metap = &meta_blockp->arena_block_meta[0];

			// We need to hold the zone madvise lock to madvise. We could take
			// it for the duration of this function, but that might hold up
			// ongoing allocation and free operations for too long. So just
			// lock and unlock for each arena.
			_malloc_lock_lock(&nanozone->madvise_lock);
			for (nanov2_meta_index_t i = 0; i < NANOV2_BLOCKS_PER_ARENA;
					i++, block_metap++) {
				if (i != metablock_meta_index) {
					nanov2_block_meta_t meta = os_atomic_load(block_metap, relaxed);
					if (meta.next_slot == SLOT_CAN_MADVISE) {
						nanov2_block_t *blockp = nanov2_block_address_from_meta_index(
								nanozone, arena, i);
						if (nanov2_madvise_block(nanozone, block_metap,
								blockp, nanov2_size_class_for_ptr(nanozone, blockp))) {
							total += NANOV2_BLOCK_SIZE;
						}
					}
				}
			}
			_malloc_lock_unlock(&nanozone->madvise_lock);
			if (goal && total >= goal) {
				goto done;
			}
			arena++;
		}
		region = nanov2_next_region_for_region(nanozone, region);
	}

done:
	MAGMALLOC_PRESSURERELIEFEND((void *)nanozone, name, (int)goal, (int)total);
	MALLOC_TRACE(TRACE_nano_memory_pressure | DBG_FUNC_END,
			(uint64_t)nanozone, goal, total, 0);

	return total;
}
#endif // OS_VARIANT_RESOLVED

#pragma mark -
#pragma mark Zone Introspection

#if OS_VARIANT_NOTRESOLVED

// NOTE: in the code that follows, address that we obtain from the Nano
// structures are relative to the target process. They need to be translated
// before they can be used to read the mapping in this process.

#define NANOV2_ZONE_PTR_TO_MAPPED_PTR(type, zone_ptr, offset) \
		(type)((mach_vm_address_t)zone_ptr - (mach_vm_offset_t)offset)
#define NANOV2_MAPPED_PTR_TO_ZONE_PTR(type, mapped_ptr, offset) \
		(type)((mach_vm_address_t)mapped_ptr + (mach_vm_offset_t)offset)

static kern_return_t
nanov2_ptr_in_use_enumerator(task_t task, void *context, unsigned type_mask,
		vm_address_t zone_address, memory_reader_t reader,
		vm_range_recorder_t recorder)
{
	// Ensure that we have configured enough of the allocator to be able to
	// examine its data structures. In tools that do not directly use Nano, we
	// won't have done this yet. nanov2_configure() runs the initialization
	// only once.
	nanov2_configure();

	// Only MALLOC_PTR_IN_USE_RANGE_TYPE and MALLOC_PTR_REGION_RANGE_TYPE have
	// meaning for Nano. Anything else returns immediately.
	if (!(type_mask & (MALLOC_PTR_IN_USE_RANGE_TYPE|MALLOC_PTR_REGION_RANGE_TYPE))) {
		return 0;
	}

	// Read the zone data.
	nanozonev2_t *nanozone;
	nanozonev2_t zone_copy;
	kern_return_t kr;
	bitarray_t slots;

	if (!reader) {
		reader = nano_common_default_reader;
	}

	kr = reader(task, zone_address, sizeof(nanozonev2_t), (void **)&nanozone);
	if (kr) {
		return kr;
	}
	boolean_t self_zone = (nanozonev2_t *)zone_address == nanozone;
	memcpy(&zone_copy, nanozone, sizeof(zone_copy));
	nanozone = &zone_copy;
	nanov2_meta_index_t metablock_meta_index = nanov2_metablock_meta_index(nanozone);

	// Process the zone one region at a time. Report each in-use block as a
	// pointer range and each in-use slot as a pointer.
	nanov2_region_t *region = nanozone->first_region_base;
	while (region) {
		mach_vm_address_t vm_addr = (mach_vm_address_t)NULL;
		kern_return_t kr = reader(task, (vm_address_t)region, NANOV2_REGION_SIZE, (void **)&vm_addr);
		if (kr) {
			return kr;
		}

		// ptr_offset is the difference between an address in the target process
		// and its mapped address in this process.
		mach_vm_offset_t ptr_offset = (mach_vm_address_t)region - vm_addr;
		nanov2_arena_t *arena = nanov2_first_arena_for_region(region);
		nanov2_arena_t *limit_arena = nanov2_limit_arena_for_region(nanozone, region);
		vm_range_t ptr_range;
		while (arena < limit_arena) {
			// Find the metadata block and process every entry, apart from the
			// one for the metadata block itself.
			nanov2_arena_metablock_t *arena_meta_blockp =
					NANOV2_ZONE_PTR_TO_MAPPED_PTR(nanov2_arena_metablock_t *,
					nanov2_metablock_address_for_ptr(nanozone, arena),
					ptr_offset);
			nanov2_block_meta_t *block_metap = &arena_meta_blockp->arena_block_meta[0];

			for (nanov2_meta_index_t i = 0; i < NANOV2_BLOCKS_PER_ARENA; i++, block_metap++) {
				if (i == metablock_meta_index) {
					// Skip the metadata block.
					continue;
				}
				nanov2_block_meta_t meta = os_atomic_load(block_metap, relaxed);
				if (!nanov2_is_block_active(meta)) {
					continue;
				}

				nanov2_block_t *blockp = nanov2_block_address_from_meta_index(
						nanozone, arena, i);
				if (type_mask & MALLOC_PTR_REGION_RANGE_TYPE) {
					// Report this block as an in-use range.
					ptr_range.address = (vm_address_t)blockp;
					ptr_range.size = NANOV2_BLOCK_SIZE;
					recorder(task, context, MALLOC_PTR_REGION_RANGE_TYPE, &ptr_range, 1);
				}
				if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
					// Report all of the pointers in the block that are not on
					// the free list.
					nanov2_size_class_t size_class = nanov2_size_class_for_ptr(
							nanozone, blockp);
					int slot_size = nanov2_size_from_size_class(size_class);
					int slot_count = slots_by_size_class[size_class];
					vm_range_t ranges[NANOV2_MAX_SLOTS_PER_BLOCK];
					int range_count = 0;
					if (meta.next_slot == SLOT_BUMP || meta.next_slot == SLOT_FULL) {
						// Either the block is full or the freelist is empty. If
						// it's full, everything is in use. If the free list is
						// empty, everything up to slot_count - meta.free_count - 1
						// is in use.
						range_count = meta.next_slot == SLOT_BUMP ?
								slot_count - meta.free_count - 1 : slot_count;
						for (int i = 0; i < range_count; i++) {
							ranges[i].address = (vm_address_t)nanov2_slot_in_block_ptr(blockp, size_class, i);
							ranges[i].size = slot_size;
						}
					} else {
						// We need to scan the freelist to see what's in use.
						int log_size = 64 - __builtin_clzl(slot_count);
						if (self_zone) {
							// Don't allocate from ourselves!
							slots = nanozone->helper_zone->calloc(nanozone->helper_zone,
									1, bitarray_size(log_size));
						} else {
							slots = bitarray_create(log_size);
						}
						for (int i = 0; i < slot_count; i++) {
							bitarray_set(slots, log_size, i);
						}

						int next_slot = meta.next_slot;
						int free_list_count = 0;
						while (next_slot != SLOT_BUMP) {
							next_slot--;   // meta.next_slot is 1-based.
							if (next_slot < 0 || next_slot >= slot_count ||
									!bitarray_get(slots, log_size, next_slot)) {
								// Out of range or already seen?? We may have
								// snapshotted the block while it was updating.
								// Don't go any further to avoid an infinite loop.
								break;
							}
							bitarray_zap(slots, log_size, next_slot);
							void *ptr = nanov2_slot_in_block_ptr(blockp, size_class, next_slot);
							nanov2_free_slot_t *slotp = NANOV2_ZONE_PTR_TO_MAPPED_PTR(nanov2_free_slot_t *, ptr, ptr_offset);
							next_slot = slotp->next_slot;
							free_list_count++;
						}
						// Add a range for each slot that is not on the freelist,
						// unless that slot has never been allocated.
						int block_free_count = meta.free_count + 1; // actual free count.
						int in_use_count = slot_count - block_free_count;
						int slots_used_count = in_use_count + free_list_count;
						index_t index;
						while (bitarray_zap_first_set(slots, log_size, &index)) {
							if (index >= slots_used_count) {
								// Reached the end of the slots that have been
								// allocated at some point.
								break;
							}
							ranges[range_count].address = (vm_address_t)nanov2_slot_in_block_ptr(blockp, size_class, index);
							ranges[range_count].size = slot_size;
							range_count++;
						}
						free(slots);
					}
					if (range_count) {
						// Notify the in-use pointers that we found.
						recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE, ranges, range_count);
					}
				}
			}
			arena++;
		}

		// We have to manually handle the linkage to the next region because
		// of the address slide between this process and the target.
		nanov2_region_linkage_t *region_linkagep =
						nanov2_region_linkage_for_region(nanozone, region);
		nanov2_region_linkage_t *mapped_region_linkagep =
				NANOV2_ZONE_PTR_TO_MAPPED_PTR(nanov2_region_linkage_t *,
				region_linkagep, ptr_offset);
		int offset = mapped_region_linkagep->next_region_offset;
		region = offset ? region + offset : NULL;
	}
	return 0;
}

static size_t
nanov2_good_size(nanozonev2_t *nanozone, size_t size)
{
	if (size <= NANO_MAX_SIZE) {
		return _nano_common_good_size(size);
	}
	return nanozone->helper_zone->introspect->good_size(nanozone->helper_zone,
			size);
}

static boolean_t
nanov2_check(nanozonev2_t *nanozone)
{
	// Does nothing, just like Nano V1.
	return 1;
}

static void
nanov2_print(task_t task, unsigned level, vm_address_t zone_address,
		memory_reader_t reader, print_task_printer_t printer)
{
    // Ensure that we have configured enough of the allocator to be able to
    // examine its data structures. In tools that do not directly use Nano, we
    // won't have done this yet. nanov2_configure() runs the initialization
    // only once.
    nanov2_configure();

	nanozonev2_t *mapped_nanozone;
	if (reader(task, (vm_address_t)zone_address, sizeof(nanozonev2_t),
            (void **)&mapped_nanozone)) {
        printer("Failed to map nanozonev2_s at %p\n", zone_address);
        return;
    }

	// Zone-wide statistics
	malloc_statistics_t stats;
	nanov2_statistics_task_printer(task, zone_address, reader, printer, &stats);
	nanov2_statistics_t *nano_stats = &mapped_nanozone->statistics;
	printer("Nanozonev2 %p: blocks in use: %llu, size in use: %llu "
			"allocated size: %llu, allocated regions: %d, region holes: %d\n",
			zone_address, (uint64_t)stats.blocks_in_use,
			(uint64_t)stats.size_in_use, (uint64_t)stats.size_allocated,
			nano_stats->allocated_regions, nano_stats->region_address_clashes);

#if DEBUG_MALLOC
	// Per-size class statistics
	printer("\nPer size-class statistics:\n");
	for (int i = 0; i < NANO_SIZE_CLASSES; i++) {
		nanov2_size_class_statistics *cs = &nano_stats->size_class_statistics[i];
		printer("  Class %d: ", i);
		printer("total alloc: %llu, total frees: %llu, madvised blocks: %llu, "
				"madvise races: %llu",
				cs->total_allocations, cs->total_frees, cs->madvised_blocks,
				cs->madvise_races);
		printer("\n");
	}
#endif // DEBUG_MALLOC

	// Per-context block pointers.
	printer("Current Allocation Blocks By Size Class/Context [CPU]\n");
	for (int i = 0; i < NANO_SIZE_CLASSES; i++) {
		printer("  Class %d: ", i);
		for (int j = 0; j < MAX_CURRENT_BLOCKS; j++) {
			if (mapped_nanozone->current_block[i][j]) {
				printer("%d: %p; ", j, mapped_nanozone->current_block[i][j]);
			}
		}
		printer("\n");
	}

	nanov2_meta_index_t metablock_meta_index =
			nanov2_metablock_meta_index(mapped_nanozone);
	nanov2_region_t *region = mapped_nanozone->first_region_base;
	int region_index = 0;
	while (region) {
		printer("\nRegion %d: base address %p\n", region_index, region);
		nanov2_region_t *mapped_region;
		if (reader(task, (vm_address_t)region, sizeof(nanov2_region_t),
				(void **)&mapped_region)) {
			printer("Failed to map nanov2 region at %p\n", region);
			return;
		}
        off_t region_offset = (uintptr_t)mapped_region - (uintptr_t)region;

		nanov2_arena_t *arena = nanov2_first_arena_for_region(region);
		nanov2_arena_t *limit_arena = nanov2_limit_arena_for_region(
				mapped_nanozone, region);
		int arena_index = 0;
		while (arena < limit_arena) {
			// Find the metadata block and process every entry, apart from the
			// one for the metadata block itself.
			nanov2_arena_metablock_t *arena_meta_blockp =
					nanov2_metablock_address_for_ptr(mapped_nanozone, arena);
			nanov2_arena_metablock_t *mapped_arena_meta_blockp =
				(nanov2_arena_metablock_t *)((uintptr_t)arena_meta_blockp + region_offset);
			nanov2_block_meta_t *block_metap = &arena_meta_blockp->arena_block_meta[0];
			nanov2_block_meta_t *mapped_block_metap = &mapped_arena_meta_blockp->arena_block_meta[0];

			int active_blocks = 0;
			int madvisable_blocks = 0;
			int unused_blocks = 0;
			int madvised_blocks = 0;
			int madvising_blocks = 0;
			for (nanov2_meta_index_t i = 0; i < NANOV2_BLOCKS_PER_ARENA; i++) {
				if (i == metablock_meta_index) {
					// Skip the metadata block.
					continue;
				}
				nanov2_block_meta_t meta = mapped_block_metap[i];
				switch (meta.next_slot) {
				case SLOT_NULL:
					unused_blocks++;
					break;
				case SLOT_MADVISED:
					madvised_blocks++;
					break;
				case SLOT_MADVISING:
					madvising_blocks++;
					break;
				case SLOT_CAN_MADVISE:
					madvisable_blocks++;
					break;
				default:
					active_blocks++;
					break;
				}
			}
			printer("Arena #%d: base address %p. Blocks - active: %d, "
					"madvisable: %d, madvising: %d, madvised: %d, unused: %d\n",
					arena_index, arena, active_blocks, madvisable_blocks,
					madvising_blocks, madvised_blocks, unused_blocks);

			// Print which size classes have blocks allocated in this arena.
			int non_empty_size_classes[NANO_SIZE_CLASSES];
			for (int i = 0; i < NANO_SIZE_CLASSES; i++) {
				non_empty_size_classes[i] = 0;
			}
			for (nanov2_meta_index_t i = 0; i < NANOV2_BLOCKS_PER_ARENA; i++) {
				if (i == metablock_meta_index) {
					// Skip the metadata block.
					continue;
				}
				nanov2_block_meta_t meta = mapped_block_metap[i];
				nanov2_size_class_t size_class =
						nanov2_size_class_for_meta_index(mapped_nanozone, i);
				switch (meta.next_slot) {
				case SLOT_FULL:
				case SLOT_BUMP:
				default:
					non_empty_size_classes[size_class]++;
					break;
				case SLOT_NULL:
				case SLOT_CAN_MADVISE:
				case SLOT_MADVISING:
				case SLOT_MADVISED:
					// Do not count these.
					break;
				}
			}
			printer("Size classes with allocated blocks: ");
			for (int i = 0; i < NANO_SIZE_CLASSES; i++) {
				if (non_empty_size_classes[i]) {
					printer("%d ", i);
				}
			}
			printer("\n");

			if (level >= MALLOC_VERBOSE_PRINT_LEVEL) {
				for (nanov2_meta_index_t i = 0; i < NANOV2_BLOCKS_PER_ARENA; i++) {
					if (i == metablock_meta_index) {
						// Skip the metadata block.
						continue;
					}
					nanov2_block_meta_t meta = mapped_block_metap[i];
					if (!nanov2_is_block_active(meta)) {
						continue;
					}
					nanov2_size_class_t size_class =
							nanov2_size_class_for_meta_index(mapped_nanozone, i);
					char *slot_text;
					switch (meta.next_slot) {
					case SLOT_NULL:
						slot_text = "NOT USED";
						break;
					case SLOT_FULL:
						slot_text = "FULL";
						break;
					case SLOT_CAN_MADVISE:
						slot_text = "CAN MADVISE";
						break;
					case SLOT_MADVISING:
						slot_text = "MADVISING";
						break;
					case SLOT_MADVISED:
						slot_text = "MADVISED";
						break;
					default:
						slot_text = NULL;
						break;
					}
					printer("    Block %d: base %p; metadata: %p, size %d "
							"(class %d) in-use: %d ",
							i, nanov2_block_address_from_meta_index(mapped_nanozone, arena, i),
							&block_metap[i], nanov2_size_from_size_class(size_class),
							size_class, meta.in_use);
					if (slot_text) {
						printer("%s\n", slot_text);
					} else {
						int allocated = slots_by_size_class[size_class] - meta.free_count - 1;
						if (meta.next_slot == SLOT_BUMP) {
							printer("BUMP (free list empty)");
						} else {
							printer("next_slot (1-based) = %d", meta.next_slot);

					}
						printer(", allocated slots: %d, free slots = %d, "
								"occupancy: %d%%\n",
								allocated, meta.free_count + 1,
								(100 * allocated)/slots_by_size_class[size_class]);
					}
				}
			}
			arena++;
			arena_index++;
		}

		region = nanov2_next_region_for_region_offset(mapped_nanozone, region,
                region_offset);
		region_index++;
	}
}

static void
nanov2_print_self(nanozonev2_t *nanozone, boolean_t verbose)
{
	nanov2_print(mach_task_self(), verbose ? MALLOC_VERBOSE_PRINT_LEVEL : 0,
			(vm_address_t)nanozone, _malloc_default_reader, malloc_report_simple);
}

static void
nanov2_print_task(task_t task, unsigned level, vm_address_t zone_address,
		memory_reader_t reader, print_task_printer_t printer)
{
	nanov2_print(task, level, zone_address, reader, printer);
}

static void
nanov2_log(malloc_zone_t *zone, void *log_address)
{
	// Does nothing, just like Nano V1.
}

static void
nanov2_force_lock(nanozonev2_t *nanozone)
{
	// Nothing to do - Nano V2 does not have a zone lock.
}

static void
nanov2_force_unlock(nanozonev2_t *nanozone)
{
	// Nothing to do - Nano V2 does not have a zone lock.
}

static void
nanov2_reinit_lock(nanozonev2_t *nanozone)
{
	// Nothing to do - Nano V2 does not have a zone lock.
}

static boolean_t
nanov2_locked(nanozonev2_t *nanozone)
{
	// Nothing to do - Nano V2 does not have a zone lock.
	return FALSE;
}

static void
null_printer(const char __unused *fmt, ...)
{
}

static kern_return_t
nanov2_statistics(task_t task, vm_address_t zone_address,
		memory_reader_t reader, print_task_printer_t printer,
		malloc_statistics_t *stats)
{
	printer = printer ? printer : null_printer;
	reader = !reader && task == mach_task_self() ? _malloc_default_reader : reader;

	kern_return_t err;

    // Ensure that we have configured enough of the allocator to be able to
    // examine its data structures. In tools that do not directly use Nano, we
    // won't have done this yet. nanov2_configure() runs the initialization
    // only once.
    nanov2_configure();

	memset(stats, '\0', sizeof(*stats));

	nanozonev2_t *mapped_nanozone;
	err = reader(task, (vm_address_t)zone_address, sizeof(nanozonev2_t),
				 (void **)&mapped_nanozone);
	if (err) {
        printer("Failed to map nanozonev2_s at %p\n", zone_address);
        return err;
    }

	nanov2_region_t *region;
	nanov2_arena_t *arena;
	nanov2_meta_index_t metadata_block_index =
			nanov2_metablock_meta_index(mapped_nanozone);

	// Iterate over each arena in each region. Within each region, add
	// statistics for each slot in each block, excluding the meta data block.
	for (region = mapped_nanozone->first_region_base; region;) {
        nanov2_region_t *mapped_region;
		err = reader(task, (vm_address_t)region, sizeof(nanov2_region_t), (void **)&mapped_region);
        if (err) {
            printer("Failed to map nanov2 region at %p\n", region);
            return err;
        }
        off_t region_offset = (uintptr_t)mapped_region - (uintptr_t)region;
		for (arena = nanov2_first_arena_for_region(region);
				arena < nanov2_limit_arena_for_region(mapped_nanozone, region);
				arena++) {
			nanov2_arena_metablock_t *meta_block =
					nanov2_metablock_address_for_ptr(mapped_nanozone, arena);
			nanov2_arena_metablock_t *mapped_meta_block =
				(nanov2_arena_metablock_t *)((uintptr_t)meta_block + region_offset);
			for (nanov2_meta_index_t i = 0; i < NANOV2_BLOCKS_PER_ARENA; i++) {
				if (i == metadata_block_index) {
					// Skip the metadata block.
					continue;
				}

				nanov2_block_meta_t *mapped_block_metap = &mapped_meta_block->arena_block_meta[i];
				nanov2_size_class_t size_class =
						nanov2_size_class_for_meta_index(mapped_nanozone, i);
				int slot_size = nanov2_size_from_size_class(size_class);

				nanov2_block_meta_t meta =
						os_atomic_load(mapped_block_metap, relaxed);
				int slots_in_use = 0;
				switch (meta.next_slot) {
				case SLOT_NULL:
					// FALLTHRU
				case SLOT_CAN_MADVISE:
					// FALLTHRU
				case SLOT_MADVISING:
					// FALLTHRU
				case SLOT_MADVISED:
					// These blocks have no active content.
					break;
				case SLOT_FULL:
					slots_in_use = slots_by_size_class[size_class];
					break;
				case SLOT_BUMP:
					// FALLTHRU
				default:
					slots_in_use = slots_by_size_class[size_class] - meta.free_count - 1;
					break;
				}

				// We can't report max_size_in_use because we don't have the
				// metadata to do so.
				if (slots_in_use) {
					stats->blocks_in_use += slots_in_use;
					stats->size_in_use += slots_in_use * slot_size;
					stats->size_allocated += NANOV2_BLOCK_SIZE;
				}
			}
		}
        region = nanov2_next_region_for_region_offset(mapped_nanozone,
                region, region_offset);
	}
	return KERN_SUCCESS;
}

static void
nanov2_statistics_self(nanozonev2_t *nanozone, malloc_statistics_t *stats)
{
	nanov2_statistics(mach_task_self(), (vm_address_t)nanozone,
			_malloc_default_reader, malloc_report_simple, stats);
}

static kern_return_t
nanov2_statistics_task_printer(task_t task, vm_address_t zone_address,
		memory_reader_t reader, print_task_printer_t printer,
		malloc_statistics_t *stats)
{
	return nanov2_statistics(task, zone_address, reader, printer, stats);
}

static kern_return_t
nanov2_statistics_task(task_t task, vm_address_t zone_address, memory_reader_t reader, malloc_statistics_t *stats)
{
	return nanov2_statistics(task, zone_address, reader, NULL, stats);
}


static const struct malloc_introspection_t nanov2_introspect = {
	.enumerator = 	(void *)nanov2_ptr_in_use_enumerator,
	.good_size =	(void *)nanov2_good_size,
	.check = 		(void *)nanov2_check,
	.print =		(void *)nanov2_print_self,
	.log = 			(void *)nanov2_log,
	.force_lock = 	(void *)nanov2_force_lock,
	.force_unlock =	(void *)nanov2_force_unlock,
	.statistics = 	(void *)nanov2_statistics_self,
	.zone_locked =	(void *)nanov2_locked,
	.enable_discharge_checking = NULL,
	.disable_discharge_checking = NULL,
#ifdef __BLOCKS__
	.enumerate_discharged_pointers = NULL,
#else // __BLOCKS__
	.enumerate_unavailable_without_blocks = NULL,
#endif // __BLOCKS__
	.reinit_lock = 	(void *)nanov2_reinit_lock,
	.print_task = 	(void *)nanov2_print_task,
	.task_statistics = (void*)nanov2_statistics_task,
};

#endif // OS_VARIANT_NOTRESOLVED

#pragma mark -
#pragma mark Utility Functions

#if OS_VARIANT_RESOLVED

// Given a pointer that may be to Nano V2-allocated memory, returns the size of
// the allocation, or 0 if the pointer does not correspond to an active
// allocation. If allow_inner is true, the pointer need not point to the start
// of the allocation.
size_t
nanov2_pointer_size(nanozonev2_t *nanozone, void *ptr, boolean_t allow_inner)
{
	// First check the address signature.
	if (!nanov2_has_valid_signature((void *)ptr)) {
		return 0;
	}

	// Check for proper alignment, unless we could have an inner pointer.
	if (!allow_inner && ((uintptr_t)ptr) & NANO_QUANTA_MASK) {
		return 0;
	}

	// Bounds check against the active address space.
	if (ptr < (void *)nanozone->first_region_base ||
			ptr > (void *)nanozone->current_region_next_arena) {
		return 0;
	}

#if NANOV2_MULTIPLE_REGIONS
	// Need to check that the region part is valid because there could be holes.
	// Do this only if we know there is a hole.
	// NOTE: in M2 convergence, use a hashed structure to make this more
	// efficient.
	if (nanozone->statistics.region_address_clashes) {
		nanov2_region_t *ptr_region = nanov2_region_address_for_ptr(ptr);
		nanov2_region_t *region = nanozone->first_region_base;
		while (region) {
			if (ptr_region == region) {
				break;
			}
			region = nanov2_next_region_for_region(nanozone, region);
		}
		if (!region) {
			// Reached the end of the region list without matching - not a
			// valid Nano V2 pointer.
			return 0;
		}
	}
#endif // NANOV2_MULTIPLE_REGIONS

	// Get the size class for the pointer and the address of its meta block
	// header.
	nanov2_size_class_t size_class = nanov2_size_class_for_ptr(nanozone, ptr);
	nanov2_block_meta_t *block_metap = nanov2_meta_ptr_for_ptr(nanozone, ptr);

	// Reject if the block is not active, or it doesn't have any allocations.
	nanov2_block_meta_t meta = os_atomic_load(block_metap, relaxed);
	if (!nanov2_is_block_active(meta) || (meta.next_slot != SLOT_FULL &&
			meta.free_count == slots_by_size_class[size_class] - 1)) {
		return 0;
	}

	size_t size = nanov2_size_from_size_class(size_class);
	nanov2_addr_t addr = { .addr = ptr };
	if (!allow_inner && (addr.fields.nano_offset % size)) {
		return 0;
	}

	// The only reasonable way to check whether the pointer is free is to
	// inspect the canary value at the start of the slot, since we cannot take
	// a huge hit for walking the free list.
	nanov2_free_slot_t *slotp = (nanov2_free_slot_t *)ptr;
	uintptr_t guard = os_atomic_load(&slotp->double_free_guard, relaxed);
	if ((guard ^ nanozone->slot_freelist_cookie) == (uintptr_t)ptr) {
		return 0;
	}

	return size;
}

#pragma mark -
#pragma mark Madvise Management

// Given a pointer to a block and its metadata, calls madvise() on that block
// if it is in state SLOT_CAN_MADVISE. Returns true on success, false if the
// block is not in the correct state or if the state changed during the
// operation.
//
// This function must be called with the zone's madvise_lock held
boolean_t
nanov2_madvise_block(nanozonev2_t *nanozone, nanov2_block_meta_t *block_metap,
		nanov2_block_t *blockp, nanov2_size_class_t size_class)
{
	_malloc_lock_assert_owner(&nanozone->madvise_lock);

	boolean_t madvised = FALSE;
	nanov2_block_meta_t old_meta = os_atomic_load(block_metap, relaxed);
	if (old_meta.next_slot == SLOT_CAN_MADVISE) {
		// Nobody raced with us. We can safely madvise this block. First change
		// the state to SLOT_MADVISING so that other threads don't try to
		// grab the block for new allocations.
		nanov2_block_meta_t new_meta = {
			.next_slot = SLOT_MADVISING,
			.gen_count = old_meta.gen_count + 1,
		};
		if (!os_atomic_cmpxchgv(block_metap, old_meta, new_meta, &old_meta,
								relaxed)) {
			// Somebody else tampered with this block. This can happen if
			// another thread raced with us to allocate in this block. Count
			// the contended access.
			nanozone->statistics.size_class_statistics[size_class].madvise_races++;
			return false;
		}

		if (mvm_madvise_free(nanozone, nanov2_region_address_for_ptr(blockp),
				(uintptr_t)blockp, (uintptr_t)(blockp + 1), NULL, FALSE)) {
			malloc_zone_error(0, false, "Failed to madvise block at blockp: %p, error: %d\n", blockp, errno);
		} else {
			nanozone->statistics.size_class_statistics[size_class].madvised_blocks++;
			madvised = TRUE;
		}

		nanov2_block_meta_t final_meta = {
			.next_slot = SLOT_MADVISED,
			.gen_count = new_meta.gen_count + 1,
		};

		if (!os_atomic_cmpxchgv(block_metap, new_meta, final_meta, &old_meta,
				relaxed)) {
			// This should not happen since we should have exclusive interest
			// in this block.
			malloc_zone_error(nanozone->debug_flags, false,
					"Failed when changing state from MADVISING to MADVISED, "
					"block_metap = %p, blockp = %p\n", block_metap, blockp);
		}
	}
	return madvised;
}

#endif // OS_VARIANT_RESOLVED

#pragma mark -
#pragma mark Region Management

#if OS_VARIANT_NOTRESOLVED

#if NANOV2_MULTIPLE_REGIONS
static nanov2_addr_t nanov2_max_region_base = {
	.fields.nano_signature = NANOZONE_SIGNATURE,
	.fields.nano_region = NANOV2_MAX_REGION_NUMBER
};
#endif // NANOV2_MULTIPLE_REGIONS

// Attempts to allocate VM space for a region at a given address and returns
// whether the allocation succeeded.
static boolean_t
nanov2_allocate_region(nanov2_region_t *region)
{
	MALLOC_TRACE(TRACE_nanov2_region_allocation | DBG_FUNC_START,
			(uint64_t)region, 0, 0, 0);
	boolean_t result = nano_common_allocate_vm_space((mach_vm_address_t)region,
			NANOV2_REGION_SIZE);
	MALLOC_TRACE(TRACE_nanov2_region_allocation | DBG_FUNC_END,
			(uint64_t)region, result, 0, 0);
	return result;
}

// Allocates a new region adjacent to the current one. If the allocation fails,
// keep sliding up by the size of a region until we either succeed or run out of
// address space. The caller must own the Nanozone regions lock.
MALLOC_NOEXPORT boolean_t
nanov2_allocate_new_region(nanozonev2_t *nanozone)
{
#if NANOV2_MULTIPLE_REGIONS
	boolean_t result = FALSE;

	_malloc_lock_assert_owner(&nanozone->regions_lock);
	nanov2_region_t *current_region = nanozone->current_region_base;
	nanov2_region_t *next_region = (nanov2_region_t *)nanozone->current_region_limit;
	while ((void *)next_region <= nanov2_max_region_base.addr) {
		if (nanov2_allocate_region(next_region)) {
			nanozone->current_region_base = next_region;
			nanozone->current_region_next_arena = (nanov2_arena_t *)next_region;
			nanozone->current_region_limit = next_region + 1;
			nanozone->statistics.allocated_regions++;
			result = TRUE;
			break;
		}
		next_region++;
		nanozone->statistics.region_address_clashes++;
	}

	if (result) {
		// Link this region to the previous one.
		nanov2_region_linkage_t *current_region_linkage =
				nanov2_region_linkage_for_region(nanozone, current_region);
		nanov2_region_linkage_t *next_region_linkage =
				nanov2_region_linkage_for_region(nanozone, next_region);
		uint16_t offset = next_region - current_region;
		current_region_linkage->next_region_offset = offset;
		next_region_linkage->next_region_offset = 0;
	}

	return result;
#else // NANOV2_MULTIPLE_REGIONS
	// On iOS, only one region is supported, so we fail since the first
	// region is allocated separately.
	return FALSE;
#endif // CONFIG_NANOV2_MULTIPLE_REGIONS
}
#endif // OS_VARIANT_NOTRESOLVED

#pragma mark -
#pragma mark Allocation

#if OS_VARIANT_RESOLVED

// Allocates memory from the block that corresponds to a given block meta data
// pointer. The memory is taken from the free list if possible, or from the
// unused region of the block if not. If the block is no longer in use or is
// full, NULL is returned and the caller is expected to find another block to
// allocate from.
MALLOC_NOEXPORT
void *
nanov2_allocate_from_block(nanozonev2_t *nanozone,
		nanov2_block_meta_t *block_metap, nanov2_size_class_t size_class)
{
	nanov2_block_meta_view_t old_meta_view;
	old_meta_view.meta = os_atomic_load(block_metap, relaxed);

	// Calculating blockp and ptr is relatively expensive. Do both lazily to
	// minimize the time in the block starting with "again:" and ending with the
	// atomic update so that we lose at little time as possible if we have to
	// repeat that loop due to contention. This should also reduce the risk of
	// contention.
	nanov2_block_t *blockp = NULL;

again:
	if (!nanov2_can_allocate_from_block(old_meta_view.meta)) {
		// Move along, nothing to allocate here...
		return NULL;
	}

	int slot;
	void *ptr = NULL;
	boolean_t from_free_list = FALSE;
	nanov2_block_meta_t new_meta = {
		.in_use = 1,
		.free_count = old_meta_view.meta.free_count - 1,
		.gen_count = old_meta_view.meta.gen_count + 1
	};

	// Grab a slot from the free list or get the next unused slot. We know there
	// should be one because the block is not full.
	boolean_t slot_full = old_meta_view.meta.free_count == 0;
	if (old_meta_view.meta.next_slot == SLOT_BUMP
				|| old_meta_view.meta.next_slot == SLOT_CAN_MADVISE) {
		// Free list empty, grab the next unused slot.
		new_meta.next_slot = slot_full ? SLOT_FULL : SLOT_BUMP;
		slot = slots_by_size_class[size_class] - old_meta_view.meta.free_count - 1;
	} else {
		// Grab the first item from the free list.
		from_free_list = TRUE;
		if (!blockp) {
			blockp = nanov2_block_address_from_meta_ptr(nanozone, block_metap);
		}
		slot = old_meta_view.meta.next_slot - 1; // meta.next_slot is 1-based.
		ptr = nanov2_slot_in_block_ptr(blockp, size_class, slot);
		nanov2_free_slot_t *slotp = (nanov2_free_slot_t *)ptr;
		new_meta.next_slot = slot_full ? SLOT_FULL : slotp->next_slot;
	}

	// Write the updated meta data; try again if we raced with another thread.
	if (!os_atomic_cmpxchgv(block_metap, old_meta_view.meta, new_meta,
				&old_meta_view.meta, dependency)) {
		if (old_meta_view.meta.next_slot == SLOT_CAN_MADVISE ||
				old_meta_view.meta.next_slot == SLOT_MADVISING ||
				old_meta_view.meta.next_slot == SLOT_MADVISED) {
			_malloc_lock_lock(&nanozone->madvise_lock);
			if (old_meta_view.meta.next_slot == SLOT_MADVISED) {
				// We raced against another thread madvising this block. We need
				// to redo the madvise because we may have touched it when
				// reading the next pointer in the freelist.
				if (!blockp) {
					blockp = nanov2_block_address_from_meta_ptr(nanozone, block_metap);
				}
				if (mvm_madvise_free(nanozone, nanov2_region_address_for_ptr(blockp),
						(uintptr_t)blockp, (uintptr_t)(blockp + 1), NULL, FALSE)) {
					malloc_zone_error(0, false,
							"Failed to remadvise block at blockp: %p, error: %d\n", blockp, errno);
				}
			}
			_malloc_lock_unlock(&nanozone->madvise_lock);
		}
		goto again;
	}

	if (!ptr) {
		if (!blockp) {
			blockp = nanov2_block_address_from_meta_ptr(nanozone, block_metap);
		}
		ptr = nanov2_slot_in_block_ptr(blockp, size_class, slot);
	}

	nanov2_free_slot_t *slotp =
			(nanov2_free_slot_t *)os_atomic_force_dependency_on(ptr,
			(unsigned long)old_meta_view.bits);
	if (from_free_list) {
		// We grabbed the item from the free list. Check the free list canary
		// and crash if it's not valid. We can't do this check before the
		// cmpxchgv because another thread may race with us, claim the slot and
		// write to it.
		uintptr_t guard = os_atomic_load(&slotp->double_free_guard, relaxed);
		if ((guard ^ nanozone->slot_freelist_cookie) != (uintptr_t)ptr) {
			malloc_zone_error(MALLOC_ABORT_ON_CORRUPTION, false,
					"Heap corruption detected, free list is damaged at %p\n"
					"*** Incorrect guard value: %lu\n", ptr, guard);
			__builtin_unreachable();
		}
	}
	
#if DEBUG_MALLOC
	nanozone->statistics.size_class_statistics[size_class].total_allocations++;
#endif // DEBUG_MALLOC

	return ptr;
}

// Finds a block for allocation in an arena and returns a pointer to its
// metadata header. The search begins from the block with metadata pointer
// start_block (which must not be NULL). If no acceptable block was found,
// NULL is returned and it is expected that the caller will take appropriate
// action (typically allocate a new arena).
//
// The search starts with start_block. If this is in-use and not full, that
// block is returned. Otherwise, a scan for a usable block is initiated. The
// search starts from start_block and initially works backward towards the
// start of the arena. If this does not succeed, a forward search from
// start_block is made.
//
// A block is considered a candidate if it is not in use. As the scan proceeds,
// we remember blocks which have been madvisable, blocks which have been
// madvised or never used and those blocks which still have allocated slots
// but which fall within the reuse criteria (i.e. their occupancy is within the
// max/min occupancy range).
//
// If the scan policy is NANO_SCAN_FIRST_FIT, we just return the first block
// from the above list that we find. This is the fastest option, but likely
// maximises fragmentation.
//
// Otherwise, the scan policy is NANO_SCAN_CAPACITY_BASED. If we find a block
// that fits the reuse criteria, we return it immediately. Otherwise, we
// continue to scan until we find such a block, or we find a less ideal block
// and we reach the scan limit or exhaust the arena. At that point, we return
// one of the candidate blocks that we found, choosing based on the state of
// that block:
// - blocks that have allocations that are greater than the minimum capacity
//   are preferred.
// - failing that, return an unused or madvise'd block.
// - failing that, return a block that is waiting to be madvised.
//
// In order to avoid races, this function must be called with the
// current_block_lock for the calling context [CPU] and size class locked.
// On return, the selected block has been marked as in-use, so the caller must
// either assign it as the active allocation block for the calling context or
// clear the in-use bit.
//
MALLOC_ALWAYS_INLINE MALLOC_INLINE nanov2_block_meta_t *
nanov2_find_block_in_arena(nanozonev2_t *nanozone,
		nanov2_arena_t *arena, nanov2_size_class_t size_class,
		nanov2_block_meta_t *start_block)
{
	// If we don't have a starting point, start with the first block in the
	// arena for the given size class. This is the case where we are looking for
	// the first allocation block for a new context (i.e probably a new CPU, so
	// take the first fit to avoid having to scan the whole size class for this
	// very common start up case.)
	boolean_t use_first_fit = !start_block ||
			nanov2_policy_config.block_scan_policy == NANO_SCAN_FIRST_FIT;
	nanov2_block_meta_t *first_block = nanov2_first_block_for_size_class_in_arena(
			nanozone, size_class, arena);
	boolean_t scanning_backwards;
	if (!start_block) {
		start_block = first_block;
	}
	int slots_in_block = slots_by_size_class[size_class];
	nanov2_block_meta_t old_meta;
	nanov2_block_meta_t *this_block;
	nanov2_block_meta_t *found_block;
	nanov2_block_meta_t *madvisable_block;
	nanov2_block_meta_t *free_block;
	nanov2_block_meta_t *fallback_block;
	boolean_t fallback_below_max;
	int scan_limit;

	// Check all of the blocks in the size class until we find one that we can
	// use, based on nanov2_block_scan_policy.
retry:
	this_block = start_block;
	found_block = NULL;
	madvisable_block = NULL;
	free_block = NULL;
	fallback_block = NULL;
	fallback_below_max = FALSE;
	scan_limit = nanov2_policy_config.block_scan_limit;
	scanning_backwards = TRUE;

	do {
		old_meta = os_atomic_load(this_block, relaxed);
		if (!old_meta.in_use && old_meta.next_slot != SLOT_FULL
				&& old_meta.next_slot != SLOT_MADVISING) {
			if (old_meta.next_slot == SLOT_CAN_MADVISE) {
				if (!madvisable_block) {
					// We can use this block as a last-ditch fallback.
					madvisable_block = this_block;
				}
			} else if (old_meta.next_slot == SLOT_NULL
					|| old_meta.next_slot == SLOT_MADVISED) {
				if (!free_block) {
					free_block = this_block;
				}
			} else if (use_first_fit) {
				found_block = this_block;
			} else {
				MALLOC_ASSERT(nanov2_policy_config.block_scan_policy == NANO_SCAN_CAPACITY_BASED);
				int percent_used = (100 * old_meta.free_count)/slots_in_block;
				if (percent_used >= nanov2_policy_config.block_scan_min_capacity
						&& percent_used <= nanov2_policy_config.block_scan_max_capacity) {
					// Within specified limits -- take this one.
					found_block = this_block;
				} else if (percent_used >= nanov2_policy_config.block_scan_min_capacity) {
					if (!fallback_block || fallback_below_max) {
						// More full than we want, but still acceptable as a
						// fallback.
						fallback_block = this_block;
					}
				} else if (!fallback_block
						&& percent_used < nanov2_policy_config.block_scan_min_capacity) {
					// Less full than we want. Keep it as a backup, but set
					// fallback_below_max to allow a block that's above max to
					// be preferred. The rationale behind this is to allow
					// blocks that have low occupancy to drain so that they can
					// be madvised.
					fallback_block = this_block;
					fallback_below_max = TRUE;
				} else if (!free_block) {
					// Not ideal, but we could use it.
					free_block = this_block;
				}
			}
			if (use_first_fit && (found_block || fallback_block || free_block)) {
				// Take whatever we got.
				break;
			}
		}

		if (scan_limit > 0) {
			// Only enforce the scan limit once we have a candidate.
			if ((fallback_block || free_block) && --scan_limit == 0) {
				break;
			}
		}

		if (scanning_backwards) {
			boolean_t wrapped;
			nanov2_block_meta_t *prev_block = nanov2_previous_block_for_size_class(
					nanozone, size_class, this_block, &wrapped);
			if (wrapped) {
				// We wrapped. Scan forward from the start block instead.
				scan_limit = nanov2_policy_config.block_scan_limit;
				scanning_backwards = FALSE;
				this_block = start_block;
			} else {
				this_block = prev_block;
			}
		} else {
			// Move to the next block, wrapping when we reach the last one for
			// this size class. Stop once we get to the block where we started.
			this_block = nanov2_next_block_for_size_class(nanozone, size_class,
					this_block, NULL);
			if (this_block == start_block) {
				break;
			}
		}
	} while (!found_block);

	if (!found_block) {
		if (fallback_block) {
			found_block = fallback_block;
		} else if (free_block) {
			found_block = free_block;
		} else if (madvisable_block) {
			found_block = madvisable_block;
		}
	}

	if (found_block) {
		// Now we need to activate the block. If this fails, we look for
		// another block.
		// If we are bringing a block that is draining back into use, we
		// just need to set in_use to 1. Otherwise, we fully initialize it.
		old_meta = os_atomic_load(found_block, relaxed);
		if (old_meta.next_slot == SLOT_MADVISING) {
			goto retry;
		}
		boolean_t reset_slot = old_meta.next_slot == SLOT_NULL
				|| old_meta.next_slot == SLOT_CAN_MADVISE
				|| old_meta.next_slot == SLOT_MADVISED;
		nanov2_block_meta_t new_meta = {
			.in_use = 1,
			.free_count = reset_slot ? slots_in_block - 1 : old_meta.free_count,
			.next_slot = reset_slot ? SLOT_BUMP : old_meta.next_slot,
			.gen_count = reset_slot ? 0 : old_meta.gen_count + 1,
		};
		if (!os_atomic_cmpxchgv(found_block, old_meta, new_meta, &old_meta,
				relaxed)) {
			goto retry;
		}
	}

	return found_block;
}

// Finds a block to allocate from and allocates memory from it. The search
// for a block starts from *block_metapp if not NULL, otherwise from the first
// arena in the first block (which is the case when the first block is allocated
// for a size class for a CPU).
// If none of the blocks for a size class in the current arena can be used, a
// new arena is allocated and, if necessary, a new region is added.
//
// The address of the allocated memory is returned and its metadata pointer is
// stored in *block_metapp. If a new region is required and it can't be
// allocated, NULL is returned and *block_metapp is unmodified.
//
// On success, the returned block is marked as in-use and the block originally
// pointed to by *block_metapp has its in-use bit cleared.
//
// In order to avoid races, this function must be called with the
// current_block_lock for the calling context [CPU] and size class locked.
MALLOC_NOEXPORT MALLOC_NOINLINE void *
nanov2_find_block_and_allocate(nanozonev2_t *nanozone,
		nanov2_size_class_t size_class, nanov2_block_meta_t **block_metapp)
{
	nanov2_arena_t *arena;
	nanov2_block_meta_t *start_block = os_atomic_load(block_metapp, relaxed);
	nanov2_block_meta_t *orig_block = start_block;
	if (start_block) {
		// Use the arena for the starting block.
		arena = nanov2_arena_address_for_ptr(start_block);
	} else {
		// Start from the first arena.
		arena = nanov2_arena_address_for_ptr(nanozone->first_region_base);
	}

	nanov2_region_t *start_region;
retry:
	start_region = nanov2_region_address_for_ptr(arena);
	nanov2_arena_t *start_arena = arena;
	nanov2_region_t *region = start_region;
	nanov2_arena_t *limit_arena = nanov2_limit_arena_for_region(nanozone, start_region);
	nanov2_arena_t *initial_region_next_arena = nanozone->current_region_next_arena;
	do {
		nanov2_block_meta_t *block_metap = nanov2_find_block_in_arena(nanozone,
				arena, size_class, start_block);
		if (block_metap) {
			// Try to allocate from this block and return if it succeeds. Note
			// that the block is now marked as in-use, so effectively belongs
			// to the calling context.
			void *ptr = nanov2_allocate_from_block(nanozone, block_metap, size_class);
			if (ptr) {
				// Make the new block the current one for the calling context.
				os_atomic_store(block_metapp, block_metap, relaxed);

				// Turn off in-use in old block_metap, if there is one.
				if (orig_block) {
					// Turn off in-use in the original current block.
					nanov2_turn_off_in_use(orig_block);
				}
				return ptr;
			}

			// We found a block but failed to allocate from it, probably because
			// it became full. Look for a new block, using the one that we just
			// failed with as the starting point. First, we need to turn off the
			// in-use bit for the block that we just failed to allocate from.
			nanov2_turn_off_in_use(block_metap);

			start_block = block_metap;
			goto retry;
		}

		// Try the next arena. If this is the last arena in the region, try the
		// next region.
		start_block = NULL;
		arena++;
		if (arena >= limit_arena) {
			region = nanov2_next_region_for_region(nanozone, region);
			if (!region) {
				// Reached the last region -- loop back to the first.
				region = nanozone->first_region_base;
			}
			arena = nanov2_first_arena_for_region(region);
			limit_arena = nanov2_limit_arena_for_region(nanozone, region);
		}
	} while (arena != start_arena);

	// If we get to this point, we need to allocate a new arena and possibly
	// a new region. If we are not permitted to do so by policy, return NULL.
	if (nanov2_policy_config.single_arena_size_classes & (1 << size_class)) {
		return NULL;
	}

	// Allocate a new arena and maybe a new region. To do either of those
	// things, we need to take the regions_lock. After doing so, check that
	// the state is unchanged. If it has, just assume that we might have some
	// new space to allocate into and try again.
	boolean_t failed = FALSE;
	arena = initial_region_next_arena;
	_malloc_lock_lock(&nanozone->regions_lock);
	if (nanozone->current_region_next_arena == arena) {
		if ((void *)arena >= nanozone->current_region_limit) {
			// Reached the end of the region. Allocate a new one, if we can.
			if (nanov2_allocate_new_region(nanozone)) {
				arena = nanozone->current_region_next_arena++;
			} else {
				failed = TRUE;
			}
		} else {
			// Assign the new arena, in the same region.
			nanozone->current_region_next_arena = arena + 1;
		}
	}
	_malloc_lock_unlock(&nanozone->regions_lock);

	if (!failed) {
		// Now allocate from the new arena. Since we updated the nanozone, it's
		// possible that some other thread has already raced with us to allocate
		// some space from it, so just use the normal allocation path to avoid
		// assumptions. It's a little more expensive, but this path is rare.
		start_block = NULL;
		goto retry;
	}

	// We need more space and we can't get it. We'll delegate to the helper.
	return NULL;
}

// Allocates memory of a given size (which must be a multiple of the Nano
// quantum size) and optionally clears it (for calloc).
//
// Allocation is attempted first from the block last used for the caller's
// context (which is initially the physical CPU by default). If there is no
// last block, or the block is full or now out of use, find another one, if
// possible. See the comments for nanov2_get_allocation_block() for the details.
//
// If the allocation fails, NULL is returned.
void *
nanov2_allocate(nanozonev2_t *nanozone, size_t rounded_size, boolean_t clear)
{
	void *ptr = NULL;
	nanov2_size_class_t size_class = nanov2_size_class_from_size(rounded_size);
	MALLOC_ASSERT(size_class < NANO_SIZE_CLASSES);
	MALLOC_ASSERT(rounded_size != 0);
	nanov2_block_meta_t *block_metap;
	nanov2_block_meta_t **block_metapp;

	// Get the index of the pointer to the block from which we are should be
	// allocating. This currently depends on the physical CPU number.
	int allocation_index = nanov2_get_allocation_block_index() & MAX_CURRENT_BLOCKS_MASK;

	// Get the current allocation block meta data pointer. If this is NULL,
	// we need to find a new allocation block.
	block_metapp = &nanozone->current_block[size_class][allocation_index];
	block_metap = os_atomic_load(block_metapp, relaxed);
	if (block_metap) {
		// Fast path: we have a block -- try to allocate from it.
		ptr = nanov2_allocate_from_block(nanozone, block_metap, size_class);
		if (ptr) {
			goto done;
		}
	}

	// No current allocation block, or we were unable to allocate. We need to
	// get a new block. Before doing so, delegate to the helper allocator if
	// the size class was full and has not released enough memory yet.
	if (nanozone->delegate_allocations & (1 << size_class)) {
		ptr = nanozone->helper_zone->malloc(nanozone->helper_zone, rounded_size);
		goto done;
	}

	// Before we try to get another block, lock and try another allocation,
	// which may succeed because another thread may have beaten us to it, or
	// some space may have freed up in the current block.
	_malloc_lock_s *lock = &nanozone->current_block_lock[size_class][allocation_index];
	_malloc_lock_lock(lock);

	block_metap = os_atomic_load(block_metapp, relaxed);
	if (block_metap) {
		ptr = nanov2_allocate_from_block(nanozone, block_metap, size_class);
		if (ptr) {
			// Good to go - keep the current block.
			goto unlock;
		}
	}

	// At this point, we do not have a current allocation block and the old one,
	// if there was one, has been marked as not in use. We need to find and
	// assign a new block. Since we have the lock, nobody else can change the
	// current_block pointer.
	ptr = nanov2_find_block_and_allocate(nanozone, size_class, block_metapp);

unlock:
	_malloc_lock_unlock(lock);

	if (!ptr) {
		// We could not find a block to allocate from -- make future
		// allocations for this size class go to the helper zone until
		// we have enough free space.
		_malloc_lock_lock(&nanozone->delegate_allocations_lock);
		nanozone->delegate_allocations |= 1 << size_class;
		_malloc_lock_unlock(&nanozone->delegate_allocations_lock);
	}

done:
	if (ptr) {
		if (clear) {
			memset(ptr, '\0', rounded_size);
		} else {
			// Always clear the double-free guard so that we can recognize that
			// this block is not on the free list.
			nanov2_free_slot_t *slotp = (nanov2_free_slot_t *)ptr;
			os_atomic_store(&slotp->double_free_guard, 0, relaxed);
		}
	}
	return ptr;
}

#pragma mark -
#pragma mark Freeing

// Frees an allocation to its owning block and updates the block's state.
// If the block becomes empty, it is marked as SLOT_CAN_MADVISE and is
// madvised immediately if the policy is NANO_MADVISE_IMMEDIATE.
void
nanov2_free_to_block(nanozonev2_t *nanozone, void *ptr,
		nanov2_size_class_t size_class)
{
	nanov2_block_t *blockp = nanov2_block_address_for_ptr(ptr);
	nanov2_block_meta_t *block_metap = nanov2_meta_ptr_for_ptr(nanozone, ptr);

	// Release the slot memory onto the block's freelist.
	nanov2_block_meta_t old_meta = os_atomic_load(block_metap, relaxed);
	int slot_count = slots_by_size_class[size_class];
	nanov2_block_meta_t new_meta;
	boolean_t was_full;

again:
	was_full = old_meta.next_slot == SLOT_FULL;
	new_meta.free_count = old_meta.free_count + 1;
	new_meta.in_use = old_meta.in_use;
	new_meta.gen_count = old_meta.gen_count + 1;
	boolean_t freeing_last_active_slot = !was_full &&
			new_meta.free_count == slots_by_size_class[size_class] - 1;
	if (freeing_last_active_slot) {
		// Releasing the last active slot onto the free list. Mark the block as
		// ready to be madvised if it's not in use, otherwise reset next_slot
		// to SLOT_BUMP.
		new_meta.next_slot = new_meta.in_use ? SLOT_BUMP : SLOT_CAN_MADVISE;
		// Write the updated meta data; try again if we raced with another thread.
		if (!os_atomic_cmpxchgv(block_metap, old_meta, new_meta, &old_meta, relaxed)) {
			goto again;
		}

		// If the block is now empty and it's not in use, madvise it if the policy
		// is to do so immediately.
		if (new_meta.next_slot == SLOT_CAN_MADVISE &&
				nanov2_madvise_policy == NANO_MADVISE_IMMEDIATE) {
			_malloc_lock_lock(&nanozone->madvise_lock);
			nanov2_madvise_block(nanozone, block_metap, blockp, size_class);
			_malloc_lock_unlock(&nanozone->madvise_lock);
		}
	} else {
		int slot_index = nanov2_slot_index_in_block(blockp, size_class, ptr);
		new_meta.next_slot = slot_index + 1;  // meta.next_slot is 1-based
		nanov2_free_slot_t *slotp = (nanov2_free_slot_t *)ptr;
		slotp->next_slot = was_full ? SLOT_BUMP : old_meta.next_slot;
		os_atomic_store(&slotp->double_free_guard,
				nanozone->slot_freelist_cookie ^ (uintptr_t)ptr, relaxed);

		// The double_free_guard change must be visible when the os_atomic_cmpxchgv
		// completes.
		// Write the updated meta data; try again if we raced with another thread.
		if (!os_atomic_cmpxchgv(block_metap, old_meta, new_meta, &old_meta, release)) {
			goto again;
		}
	}

	// If this size class has been marked as full and this block is below an
	// acceptable level of occupancy, turn off delegation to the helper. Do this
	// only if the block is not in-use, because an in-use block cannot be a
	// candidate when searching for a new block.
	uint16_t class_mask = 1 << size_class;
	if (!new_meta.in_use && (nanozone->delegate_allocations & class_mask) &&
			(new_meta.free_count >= 0.75 * slot_count)) {
		_malloc_lock_lock(&nanozone->delegate_allocations_lock);
		nanozone->delegate_allocations &= ~class_mask;
		_malloc_lock_unlock(&nanozone->delegate_allocations_lock);
	}

#if DEBUG_MALLOC
	nanozone->statistics.size_class_statistics[size_class].total_frees++;
#endif // DEBUG_MALLOC
}

#endif // OS_VARIANT_RESOLVED

#if OS_VARIANT_NOTRESOLVED

#pragma mark -
#pragma mark Zone Operations

malloc_zone_t *
nanov2_create_zone(malloc_zone_t *helper_zone, unsigned debug_flags)
{
	// Note: It is important that nanov2_create_zone resets _malloc_engaged_nano
	// if it is unable to enable the nanozone (and chooses not to abort). As
	// several functions rely on _malloc_engaged_nano to determine if they
	// should manipulate the nanozone, and these should not run if we failed
	// to create the zone.
	MALLOC_ASSERT(_malloc_engaged_nano == NANO_V2);

	// Get memory for the zone and disable Nano if we fail.
	nanozonev2_t *nanozone = nano_common_allocate_based_pages(
			NANOZONEV2_ZONE_PAGED_SIZE, 0, 0, VM_MEMORY_MALLOC, 0);
	if (!nanozone) {
		_malloc_engaged_nano = NANO_NONE;
		return NULL;
	}

	// Set up the basic_zone portion of the nanozonev2 structure
	nanozone->basic_zone.version = 12;
	nanozone->basic_zone.size = OS_RESOLVED_VARIANT_ADDR(nanov2_size);
	nanozone->basic_zone.malloc = OS_RESOLVED_VARIANT_ADDR(nanov2_malloc);
	nanozone->basic_zone.calloc = OS_RESOLVED_VARIANT_ADDR(nanov2_calloc);
	nanozone->basic_zone.valloc = (void *)nanov2_valloc;
	nanozone->basic_zone.free = OS_RESOLVED_VARIANT_ADDR(nanov2_free);
	nanozone->basic_zone.realloc = OS_RESOLVED_VARIANT_ADDR(nanov2_realloc);
	nanozone->basic_zone.destroy = (void *)nanov2_destroy;
	nanozone->basic_zone.batch_malloc = OS_RESOLVED_VARIANT_ADDR(nanov2_batch_malloc);
	nanozone->basic_zone.batch_free = OS_RESOLVED_VARIANT_ADDR(nanov2_batch_free);
	nanozone->basic_zone.introspect =
			(struct malloc_introspection_t *)&nanov2_introspect;
	nanozone->basic_zone.memalign = (void *)nanov2_memalign;
	nanozone->basic_zone.free_definite_size = OS_RESOLVED_VARIANT_ADDR(nanov2_free_definite_size);
	nanozone->basic_zone.pressure_relief = OS_RESOLVED_VARIANT_ADDR(nanov2_pressure_relief);
	nanozone->basic_zone.claimed_address = OS_RESOLVED_VARIANT_ADDR(nanov2_claimed_address);

	// Set these both to zero as required by CFAllocator.
	nanozone->basic_zone.reserved1 = 0;
	nanozone->basic_zone.reserved2 = 0;

	// Prevent overwriting the function pointers in basic_zone.
	mprotect(nanozone, sizeof(nanozone->basic_zone), PROT_READ);

	// Nano V2 zone does not support MALLOC_ADD_GUARD_PAGES
	if (debug_flags & MALLOC_ADD_GUARD_PAGES) {
		malloc_report(ASL_LEVEL_INFO, "nano does not support guard pages\n");
		debug_flags &= ~MALLOC_ADD_GUARD_PAGES;
	}

	// Set up the remainder of the nanozonev2 structure
	nanozone->debug_flags = debug_flags;
	nanozone->helper_zone = helper_zone;

	// Initialize the cookies used to detect double freeing and for the ASLR
	// scramble mapping.
#define COOKIE_ENTROPY_MASK		0x0000ffffffff0000ULL
#define DEFAULT_ENTROPY_BITS	0x0000DEADDEAD0000ULL
	uintptr_t cookie = (uintptr_t)malloc_entropy[0] & COOKIE_ENTROPY_MASK;
	if (!cookie) {
		cookie = malloc_entropy[1] & COOKIE_ENTROPY_MASK;
		if (!cookie) {
			// The cookie can't be zero, because it's used to compute the guard
			// value in free slots, so make sure we have a non-zero value. Using
			// a fixed value allows us to recognize that it isn't real entropy.
			cookie = DEFAULT_ENTROPY_BITS;
		}
	}
	nanozone->slot_freelist_cookie = cookie;

	// For the ASLR cookie, we take the top 12 bits of malloc_entropy[1] and
	// align it to the block field of a Nano address.
	nanozone->aslr_cookie = malloc_entropy[1] >> (64 - NANOV2_BLOCK_BITS);
	nanozone->aslr_cookie_aligned = nanozone->aslr_cookie << NANOV2_OFFSET_BITS;

	_malloc_lock_init(&nanozone->blocks_lock);
	_malloc_lock_init(&nanozone->regions_lock);
	_malloc_lock_init(&nanozone->madvise_lock);

	// Allocate the initial region. If this does not succeed, we disable Nano.
	nanov2_addr_t p = {.fields.nano_signature = NANOZONE_SIGNATURE};
	nanov2_region_t *region = (nanov2_region_t *)p.addr;
	boolean_t result = nanov2_allocate_region(region);
	if (!result) {
		nano_common_deallocate_pages(nanozone, NANOZONEV2_ZONE_PAGED_SIZE, 0);
		_malloc_engaged_nano = NANO_NONE;
		malloc_report(ASL_LEVEL_NOTICE, "nano zone abandoned due to inability "
				"to preallocate reserved vm space.\n");
		return NULL;
	}
	nanov2_region_linkage_t *region_linkage =
			nanov2_region_linkage_for_region(nanozone, region);
	region_linkage->next_region_offset = 0;

	// Install the first region and pre-allocate the first arena.
	nanozone->first_region_base = region;
	nanozone->current_region_base = region;
	nanozone->current_region_next_arena = ((nanov2_arena_t *)region) + 1;
	nanozone->current_region_limit = region + 1;
	nanozone->statistics.allocated_regions = 1;

	return (malloc_zone_t *)nanozone;
}
#endif // OS_VARIANT_NOTRESOLVED

#pragma mark -
#pragma mark Zone Fork Handling

// Nanomalloc assumes that after a fork, it would be dangerous to rely on
// the integrity of the zone data. During a fork, some of the zone handlers are
// switched to the versions below, which do the following:
// 1. Delegate all new allocation to the helper zone.
// 2. Do nothing when asked to free memory that Nano allocated. There will be a
// leak, but this is better than possibly crashing.

#if OS_VARIANT_RESOLVED
MALLOC_NOEXPORT void *
nanov2_forked_malloc(nanozonev2_t *nanozone, size_t size)
{
	// Just hand to the helper zone.
	return nanozone->helper_zone->malloc(nanozone->helper_zone, size);
}
#endif // OS_VARIANT_RESOLVED

#if OS_VARIANT_NOTRESOLVED

static void *
nanov2_forked_calloc(nanozonev2_t *nanozone, size_t num_items, size_t size)
{
	// Just hand to the helper zone.
	return nanozone->helper_zone->calloc(nanozone->helper_zone, num_items,
			size);
}

#endif // OS_VARIANT_NOTRESOLVED

#if OS_VARIANT_RESOLVED

MALLOC_NOEXPORT void
nanov2_forked_free(nanozonev2_t *nanozone, void *ptr)
{
	if (!ptr) {
		return; // Protect against malloc_zone_free() passing NULL.
	}

	// <rdar://problem/26481467> exhausting a slot may result in a pointer with
	// the nanozone prefix being given to nano_free via malloc_zone_free. Calling
	// vet_and_size here, instead of in _nano_free_check_scribble means we can
	// early-out into the helper_zone if it turns out nano does not own this ptr.
	size_t sz = nanov2_pointer_size(nanozone, ptr, FALSE);
	if (sz || nanov2_has_valid_signature(ptr)) {
		/* Drop it on the floor as nanozone metadata could be fouled by fork. */
		return;
	} else {
		nanozone->helper_zone->free(nanozone->helper_zone, ptr);
		return;
	}
	/* NOTREACHED */
}

MALLOC_NOEXPORT void
nanov2_forked_free_definite_size(nanozonev2_t *nanozone, void *ptr, size_t size)
{
	nanov2_forked_free(nanozone, ptr);
}

MALLOC_NOEXPORT void *
nanov2_forked_realloc(nanozonev2_t *nanozone, void *ptr, size_t new_size)
{
	// could occur through malloc_zone_realloc() path
	if (!ptr) {
		// If ptr is a null pointer, realloc() shall be equivalent to malloc()
		// for the specified size.
		return nanov2_forked_malloc(nanozone, new_size);
	}

	size_t old_size = nanov2_pointer_size(nanozone, ptr, FALSE);
	if (!old_size) {
		// not-nano pointer, hand down to helper zone
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		return zone->realloc(zone, ptr, new_size);
	} else {
		if (!new_size) {
			// If size is 0 and ptr is not a null pointer, the object pointed to
			// is freed. However as nanozone metadata could be fouled by fork,
			// we'll intentionally leak it.

			// If size is 0, either a null pointer or a unique pointer that can
			// be successfully passed to free() shall be returned.
			return nanov2_forked_malloc(nanozone, 1);
		}

		void *new_ptr = nanozone->helper_zone->malloc(nanozone->helper_zone,
				new_size);
		if (new_ptr) {
			size_t valid_size = MIN(old_size, new_size);
			memcpy(new_ptr, ptr, valid_size);
			// Original pointer is intentionally leaked as nanozone metadata
			// could be fouled by fork.
			return new_ptr;
		} else {
			// Original ptr is left intact
			return NULL;
		}
		/* NOTREACHED */
	}
	/* NOTREACHED */
}
#endif // OS_VARIANT_RESOLVED

#if OS_VARIANT_NOTRESOLVED

static unsigned
nanov2_forked_batch_malloc(nanozonev2_t *nanozone, size_t size, void **results,
		unsigned count)
{
	// Just pass this to the helper zone.
	return nanozone->helper_zone->batch_malloc(nanozone->helper_zone, size,
			results, count);
}
#endif // OS_VARIANT_NOTRESOLVED

#if OS_VARIANT_RESOLVED

MALLOC_NOEXPORT void
nanov2_forked_batch_free(nanozonev2_t *nanozone, void **to_be_freed,
		unsigned count)
{
	if (!count) {
		return;
	}

	while (count--) {
		void *ptr = to_be_freed[count];
		if (ptr) {
			nanov2_forked_free(nanozone, ptr);
		}
	}
}
#endif // OS_VARIANT_RESOLVED

#if OS_VARIANT_NOTRESOLVED

static boolean_t
nanov2_forked_claimed_address(struct _malloc_zone_t *zone, void *ptr)
{
	// This does not operate after fork - default to true to avoid
	// false negatives.
	return true;
}

void
nanov2_forked_zone(nanozonev2_t *nanozone)
{
	// Hobble the nano zone in the child of a fork prior to an exec since
	// the state of the zone can be made inconsistent by a parent thread while
	// the fork is underway. All new allocations will be referred to the helper
	// zone (which is more stable.) All free()'s of existing nano objects will
	// be leaked.
	mprotect(nanozone, sizeof(nanozone->basic_zone), PROT_READ | PROT_WRITE);

	nanozone->basic_zone.size = OS_RESOLVED_VARIANT_ADDR(nanov2_size);	// Unchanged
	nanozone->basic_zone.malloc = OS_RESOLVED_VARIANT_ADDR(nanov2_forked_malloc);
	nanozone->basic_zone.calloc = (void *)nanov2_forked_calloc;
	nanozone->basic_zone.valloc = (void *)nanov2_valloc;		// Unchanged
	nanozone->basic_zone.free = OS_RESOLVED_VARIANT_ADDR(nanov2_forked_free);
	nanozone->basic_zone.realloc = OS_RESOLVED_VARIANT_ADDR(nanov2_forked_realloc);
	nanozone->basic_zone.destroy = (void *)nanov2_destroy;		// Unchanged
	nanozone->basic_zone.batch_malloc = (void *)nanov2_forked_batch_malloc;
	nanozone->basic_zone.batch_free = OS_RESOLVED_VARIANT_ADDR(nanov2_forked_batch_free);
	nanozone->basic_zone.introspect =
			(struct malloc_introspection_t *)&nanov2_introspect;// Unchanged
	nanozone->basic_zone.memalign = (void *)nanov2_memalign; 	// Unchanged
	nanozone->basic_zone.free_definite_size =
			OS_RESOLVED_VARIANT_ADDR(nanov2_forked_free_definite_size);
	nanozone->basic_zone.claimed_address = nanov2_forked_claimed_address;
	mprotect(nanozone, sizeof(nanozone->basic_zone), PROT_READ);
}

#endif // OS_VARIANT_NOTRESOLVED

#endif // CONFIG_NANOZONE
