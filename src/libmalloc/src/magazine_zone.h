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

#ifndef __MAGAZINE_ZONE_H
#define __MAGAZINE_ZONE_H

/*********************	DEFINITIONS	************************/

// Out-of-band free list entry. Out-of-band free list entries are used
// in specific cases where a free-list entry is the *only* data on a given page,
// and the presence of that entry causes the page to stay dirty.
//
// `ptr` is all 16-bit quantum-sized index and packed, as that references a
// block address inside the current region. `next` and `prev` have to be pointer
// sized references, as these values can point to entries outside the current
// region, so it's not safe to compact them.
typedef struct {
	uintptr_t prev;
	uintptr_t next;
	uint16_t ptr;
} MALLOC_PACKED oob_free_entry_s, *oob_free_entry_t;

// In-place free list entry. Unlike the out-of-band entry, the in-place entries
// are stored at the start of the range that has been freed.
typedef struct _inplace_free_entry_s *inplace_free_entry_t;

typedef struct {
	void *ptr;
	uint8_t checksum;
} inplace_linkage_s;

typedef union {
	inplace_free_entry_t p;
	uintptr_t u;
} inplace_union;

typedef struct _inplace_free_entry_s {
	inplace_union previous;
	inplace_union next;
} inplace_free_entry_s, *inplace_free_entry_t;

#ifdef __LP64__
MALLOC_STATIC_ASSERT(sizeof(inplace_free_entry_s) == 16, "inplace free list must be 16-bytes long");
#else
MALLOC_STATIC_ASSERT(sizeof(inplace_free_entry_s) == 8, "inplace free list must be 8-bytes long");
#endif

typedef struct _small_inplace_free_entry_s {
	inplace_linkage_s previous;
	inplace_linkage_s next;
} small_inplace_free_entry_s, *small_inplace_free_entry_t;

typedef union {
	small_inplace_free_entry_t small_inplace;
	inplace_free_entry_t inplace;
	oob_free_entry_t oob;
	void *p;
} free_list_t;

typedef struct {
	inplace_union previous;
	inplace_union next;
} tiny_free_list_t;

typedef unsigned int grain_t; // N.B. wide enough to index all free slots

typedef int mag_index_t;

#define CHECK_REGIONS (1 << 31)
#define DISABLE_ASLR (1 << 30)

#define MAX_RECORDER_BUFFER 256

/*********************	DEFINITIONS for tiny	************************/

/*
 * Memory in the Tiny range is allocated from regions (heaps) pointed to by the
 * szone's hashed_regions pointer.
 *
 * Each region is laid out as a heap, followed by a header block, all within
 * a 1MB (2^20) block.  This means there are 64520 16-byte blocks and the header
 * is 16138 bytes, making the total 1048458 bytes, leaving 118 bytes unused.
 *
 * The header block is arranged as in struct tiny_region defined just below, and
 * consists of two bitfields (or bit arrays) interleaved 32 bits by 32 bits.
 *
 * Each bitfield comprises NUM_TINY_BLOCKS bits, and refers to the corresponding
 * TINY_QUANTUM block within the heap.
 *
 * The bitfields are used to encode the state of memory within the heap.  The header bit indicates
 * that the corresponding quantum is the first quantum in a block (either in use or free).  The
 * in-use bit is set for the header if the block has been handed out (allocated).  If the header
 * bit is not set, the in-use bit is invalid.
 *
 * The szone maintains an array of NUM_TINY_SLOTS freelists, each of which is used to hold
 * free objects of the corresponding quantum size.
 *
 * A free block is laid out depending on its size, in order to fit all free
 * blocks in 16 bytes, on both 32 and 64 bit platforms.  One quantum blocks do
 * not store their size in the block, instead relying on the header information
 * to determine their size.  Blocks of two or more quanta have room to store
 * their size in the block, and store it both after the 'next' pointer, and in
 * the last 2 bytes of the block.
 *
 * 1-quantum block
 * Offset (32-bit mode)	(64-bit mode)
 * 0x0          0x0      : previous
 * 0x4          0x08     : next
 * end          end
 *
 * >1-quantum block
 * Offset (32-bit mode)	(64-bit mode)
 * 0x0          0x0      : previous
 * 0x4          0x08     : next
 * 0x8          0x10     : size (in quantum counts)
 * end - 2      end - 2  : size (in quantum counts)
 * end          end
 *
 * All fields are pointer-sized, except for the size which is an unsigned short.
 *
 */

#define FOLLOWING_TINY_PTR(ptr, msize) (((unsigned char *)(ptr)) + ((msize) << SHIFT_TINY_QUANTUM))

#define TINY_BLOCKS_ALIGN (SHIFT_TINY_CEIL_BLOCKS + SHIFT_TINY_QUANTUM) // 20

#define TINY_ENTROPY_BITS 15
#define TINY_ENTROPY_MASK ((1 << TINY_ENTROPY_BITS) - 1)

/*
 * Avoid having so much entropy that the end of a valid tiny allocation
 * might overrun the end of the tiny region.
 */
#if TINY_ENTROPY_MASK + NUM_TINY_SLOTS > NUM_TINY_BLOCKS
#error Too many entropy bits for tiny region requested
#endif

/*
 * Enough room for the data, followed by the bit arrays (2-bits per block)
 * plus rounding to the nearest page.
 */
#define CEIL_NUM_TINY_BLOCKS_WORDS (((NUM_TINY_BLOCKS + 31) & ~31) >> 5)
#define TINY_METADATA_SIZE (sizeof(region_trailer_t) + sizeof(tiny_header_inuse_pair_t) * CEIL_NUM_TINY_BLOCKS_WORDS)
#define TINY_REGION_SIZE ((NUM_TINY_BLOCKS * TINY_QUANTUM + TINY_METADATA_SIZE + PAGE_MAX_SIZE - 1) & ~(PAGE_MAX_SIZE - 1))

#define TINY_METADATA_START (NUM_TINY_BLOCKS * TINY_QUANTUM)

/*
 * Beginning and end pointers for a region's heap.
 */
#define TINY_REGION_ADDRESS(region) ((void *)(region))
#define TINY_REGION_END(region) ((void *)(((uintptr_t)(region)) + (NUM_TINY_BLOCKS * TINY_QUANTUM)))

/*
 * Locate the heap base for a pointer known to be within a tiny region.
 */
#define TINY_REGION_FOR_PTR(_p) ((void *)((uintptr_t)(_p) & ~((1 << TINY_BLOCKS_ALIGN) - 1)))

/*
 * Convert between byte and msize units.
 */
#define TINY_BYTES_FOR_MSIZE(_m) ((_m) << SHIFT_TINY_QUANTUM)
#define TINY_MSIZE_FOR_BYTES(_b) ((_b) >> SHIFT_TINY_QUANTUM)

#if MALLOC_TARGET_64BIT
#define TINY_FREE_SIZE(ptr) (((msize_t *)(ptr))[8])
#else // MALLOC_TARGET_64BIT
#define TINY_FREE_SIZE(ptr) (((msize_t *)(ptr))[4])
#endif // MALLOC_TARGET_64BIT
#define TINY_PREVIOUS_MSIZE(ptr) ((msize_t *)(ptr))[-1]

/*
 * Layout of a tiny region
 */
typedef uint32_t tiny_block_t[4]; // assert(TINY_QUANTUM == sizeof(tiny_block_t))

typedef struct tiny_header_inuse_pair {
	uint32_t header;
	uint32_t inuse;
} tiny_header_inuse_pair_t;

typedef struct region_trailer {
	struct region_trailer *prev;
	struct region_trailer *next;
	boolean_t recirc_suitable;
	volatile int pinned_to_depot;
	unsigned bytes_used;
	mag_index_t mag_index;
} region_trailer_t;

typedef struct tiny_region {
	tiny_block_t blocks[NUM_TINY_BLOCKS];

	region_trailer_t trailer;

	// The interleaved bit arrays comprising the header and inuse bitfields.
	// The unused bits of each component in the last pair will be initialized to sentinel values.
	tiny_header_inuse_pair_t pairs[CEIL_NUM_TINY_BLOCKS_WORDS];

	uint8_t pad[TINY_REGION_SIZE - (NUM_TINY_BLOCKS * sizeof(tiny_block_t)) - TINY_METADATA_SIZE];
} * tiny_region_t;

/*
 * Per-region meta data for tiny allocator
 */
#define REGION_TRAILER_FOR_TINY_REGION(r) (&(((tiny_region_t)(r))->trailer))
#define MAGAZINE_INDEX_FOR_TINY_REGION(r) (REGION_TRAILER_FOR_TINY_REGION(r)->mag_index)
#define BYTES_USED_FOR_TINY_REGION(r) (REGION_TRAILER_FOR_TINY_REGION(r)->bytes_used)

/*
 * Locate the block header for a pointer known to be within a tiny region.
 */
#define TINY_BLOCK_HEADER_FOR_PTR(_p) ((void *)&(((tiny_region_t)TINY_REGION_FOR_PTR(_p))->pairs))

/*
 * Locate the inuse map for a given block header pointer.
 */
#define TINY_INUSE_FOR_HEADER(_h) ((void *)&(((tiny_header_inuse_pair_t *)(_h))->inuse))

/*
 * Compute the bitmap index for a pointer known to be within a tiny region.
 */
#define TINY_INDEX_FOR_PTR(_p) (((uintptr_t)(_p) >> SHIFT_TINY_QUANTUM) & (NUM_TINY_CEIL_BLOCKS - 1))

#if !CONFIG_TINY_CACHE
#warning CONFIG_TINY_CACHE turned off
#endif

#define TINY_REGION_PAYLOAD_BYTES (NUM_TINY_BLOCKS * TINY_QUANTUM)

/*********************	DEFINITIONS for small	************************/

/*
 * Memory in the Small range is allocated from regions (heaps) pointed to by the szone's hashed_regions
 * pointer.
 *
 * Each region is laid out as a heap, followed by the metadata array, all within an 8MB (2^23) block.
 * The array is arranged as an array of shorts, one for each SMALL_QUANTUM in the heap. There are 
 * 16319 512-blocks and the array is 16319*2 bytes, which totals 8387966, leaving 642 bytes unused. 
 * Once the region trailer is accounted for, there is room for 61 out-of-band free list entries in 
 * the remaining padding (or 6, if the region was split into 16320 blocks, not 16319).
 *
 * The 16-bit shorts in the region are used for allocation metadata. The MSB bit marks a block as
 * either free, or not. The remaining 15-bits give the size of the allocation, defined in "msize", the
 * quantum-shifted size of the allocation.
 *
 * The metadata table either:
 *
 *    1. Stores the allocation size in the first short for the block, with the MSB cleared to indicate
 *       that the block is allocated and in-use, or,
 *
 *    2. Stores the free-allocation size in the first and last shorts for the block, with the MSB set
 *       in both places to indicate that the block is freed. (Storing the range in last block allows
 *       for coalescing of adjacent free entries).
 *
 *    3. Zero, or "middle", meaning that this block in the region is not the start or end of an
 *       allocated block.
 *
 * The small zone represents the free list in one of two ways:
 *
 *    1. In-line free list entries. These are stored at the starting address of the just-freed memory
 *       and both the previous and next pointer are checksummed to attempt to detect use-after-free
 *       writes.
 *
 *       An in-line free list entry is laid out as:
 *           |prev (uintptr_t)|checksum (uint8_t)|next (uintptr_t)|checksum (uint8_t)
 *
 *    2. Out-of-band free list entries. These utilitise the remaining padding in the 8mb region that
 *       follows the blocks, metadata and region trailer. Out-of-band entries are used *iff* the
 *       freed address lies on a page boundary and the freed region spans more than a page. If we were
 *       to store the free list entry in-line in that memory, it would keep the entire page dirty, 
 *       so an out-of-band entry is used.
 *
 *       An out-of-band free list entry is laid out as:
 *           |prev (uintptr_t)|next (uintptr_t)|ptr (uint16_t)|
 *
 * The szone maintains an array of 32 freelists, each of which is used to hold free objects
 * of the corresponding quantum size.
 */

#define SMALL_IS_FREE (1 << 15)
#define FOLLOWING_SMALL_PTR(ptr, msize) (((unsigned char *)(ptr)) + ((msize) << SHIFT_SMALL_QUANTUM))

/*
 * SMALL_IS_OOB is used mark to the MSB of OOB free list entries to show that they are in use, and 
 * distinguish them from their initial, empty, state.
 */
#define SMALL_IS_OOB (1 << 15)

#define SMALL_ENTROPY_BITS 13
#define SMALL_ENTROPY_MASK ((1 << SMALL_ENTROPY_BITS) - 1)

/*
 * Avoid having so much entropy that the end of a valid small allocation
 * might overrun the end of the small region.
 */
#if SMALL_ENTROPY_MASK + NUM_SMALL_SLOTS > NUM_SMALL_BLOCKS
#error Too many entropy bits for small region requested
#endif

#define SMALL_METADATA_SIZE (sizeof(region_trailer_t) + NUM_SMALL_BLOCKS * sizeof(msize_t))
#define SMALL_REGION_SIZE ((NUM_SMALL_BLOCKS * SMALL_QUANTUM + SMALL_METADATA_SIZE + PAGE_MAX_SIZE - 1) & ~(PAGE_MAX_SIZE - 1))

#define SMALL_METADATA_START (NUM_SMALL_BLOCKS * SMALL_QUANTUM)

/*
 * Beginning and end pointers for a region's heap.
 */
#define SMALL_REGION_ADDRESS(region) ((unsigned char *)region)
#define SMALL_REGION_END(region) (SMALL_REGION_ADDRESS(region) + (NUM_SMALL_BLOCKS * SMALL_QUANTUM))

/*
 * Locate the heap base for a pointer known to be within a small region.
 */
#define SMALL_REGION_FOR_PTR(_p) ((void *)((uintptr_t)(_p) & ~((1 << SMALL_BLOCKS_ALIGN) - 1)))
#define SMALL_OFFSET_FOR_PTR(_p) ((uintptr_t)(_p) & ((1 << SMALL_BLOCKS_ALIGN) - 1))

/*
 * Convert between byte and msize units.
 */
#define SMALL_BYTES_FOR_MSIZE(_m) ((_m) << SHIFT_SMALL_QUANTUM)
#define SMALL_MSIZE_FOR_BYTES(_b) ((_b) >> SHIFT_SMALL_QUANTUM)

#define SMALL_PREVIOUS_MSIZE(ptr) (*SMALL_METADATA_FOR_PTR(ptr - 1) & ~SMALL_IS_FREE)

/*
 * Layout of a small region
 */
typedef uint32_t small_block_t[SMALL_QUANTUM / sizeof(uint32_t)];
#define SMALL_HEAP_SIZE (NUM_SMALL_BLOCKS * sizeof(small_block_t))
#define SMALL_OOB_COUNT ((SMALL_REGION_SIZE - SMALL_HEAP_SIZE - SMALL_METADATA_SIZE) / sizeof(oob_free_entry_s))
#define SMALL_OOB_SIZE (SMALL_OOB_COUNT * sizeof(oob_free_entry_s))
#define SMALL_REGION_PAD (SMALL_REGION_SIZE - SMALL_HEAP_SIZE - SMALL_METADATA_SIZE - SMALL_OOB_SIZE)

typedef struct small_region {
	small_block_t blocks[NUM_SMALL_BLOCKS];
	region_trailer_t trailer;
	msize_t small_meta_words[NUM_SMALL_BLOCKS];
	oob_free_entry_s small_oob_free_entries[SMALL_OOB_COUNT];
	uint8_t pad[SMALL_REGION_PAD];
} * small_region_t;

// The layout described above should result in a small_region_t being 8MB.
MALLOC_STATIC_ASSERT(sizeof(struct small_region) == 8388608, "incorrect small_region_size");

/*
 * Per-region meta data for small allocator
 */
#define REGION_TRAILER_FOR_SMALL_REGION(r) (&(((small_region_t)(r))->trailer))
#define MAGAZINE_INDEX_FOR_SMALL_REGION(r) (REGION_TRAILER_FOR_SMALL_REGION(r)->mag_index)
#define BYTES_USED_FOR_SMALL_REGION(r) (REGION_TRAILER_FOR_SMALL_REGION(r)->bytes_used)

/*
 * Locate the metadata base for a pointer known to be within a small region.
 */
#define SMALL_META_HEADER_FOR_PTR(_p) (((small_region_t)SMALL_REGION_FOR_PTR(_p))->small_meta_words)

/*
 * Compute the metadata index for a pointer known to be within a small region.
 */
#define SMALL_META_INDEX_FOR_PTR(_p) (((uintptr_t)(_p) >> SHIFT_SMALL_QUANTUM) & (NUM_SMALL_CEIL_BLOCKS - 1))

/*
 * Find the metadata word for a pointer known to be within a small region.
 */
#define SMALL_METADATA_FOR_PTR(_p) (SMALL_META_HEADER_FOR_PTR(_p) + SMALL_META_INDEX_FOR_PTR(_p))

/*
 * Determine whether a pointer known to be within a small region points to memory which is free.
 */
#define SMALL_PTR_IS_FREE(_p) (*SMALL_METADATA_FOR_PTR(_p) & SMALL_IS_FREE)

/*
 * Extract the msize value for a pointer known to be within a small region.
 */
#define SMALL_PTR_SIZE(_p) (*SMALL_METADATA_FOR_PTR(_p) & ~SMALL_IS_FREE)

#if !CONFIG_SMALL_CACHE
#warning CONFIG_SMALL_CACHE turned off
#endif

#define SMALL_REGION_PAYLOAD_BYTES (NUM_SMALL_BLOCKS * SMALL_QUANTUM)

/*************************  DEFINITIONS for large  ****************************/


typedef struct large_entry_s {
	vm_address_t address;
	vm_size_t size;
	boolean_t did_madvise_reusable;
} large_entry_t;

#if !CONFIG_LARGE_CACHE && DEBUG_MALLOC
#warning CONFIG_LARGE_CACHE turned off
#endif


/*******************************************************************************
 * Definitions for region hash
 ******************************************************************************/

typedef void *region_t;
typedef region_t *rgnhdl_t; /* A pointer into hashed_regions array. */

#define INITIAL_NUM_REGIONS_SHIFT 6							 // log2(INITIAL_NUM_REGIONS)
#define INITIAL_NUM_REGIONS (1 << INITIAL_NUM_REGIONS_SHIFT) // Must be a power of 2!
#define HASHRING_OPEN_ENTRY ((region_t)0)					 // Initial value and sentinel marking end of collision chain
#define HASHRING_REGION_DEALLOCATED ((region_t)-1)			 // Region at this slot reclaimed by OS
#define HASH_BLOCKS_ALIGN TINY_BLOCKS_ALIGN					 // MIN( TINY_BLOCKS_ALIGN, SMALL_BLOCKS_ALIGN, ... )

typedef struct region_hash_generation {
	size_t num_regions_allocated;
	size_t num_regions_allocated_shift; // log2(num_regions_allocated)
	region_t *hashed_regions;			// hashed by location
	struct region_hash_generation *nextgen;
} region_hash_generation_t;

/*******************************************************************************
 * Per-processor magazine for tiny and small allocators
 ******************************************************************************/

typedef struct magazine_s { // vm_allocate()'d, so the array of magazines is page-aligned to begin with.
	// Take magazine_lock first,  Depot lock when needed for recirc, then szone->{tiny,small}_regions_lock when needed for alloc
	_malloc_lock_s magazine_lock MALLOC_CACHE_ALIGN;
	// Protection for the crtical section that does allocate_pages outside the magazine_lock
	volatile boolean_t alloc_underway;

	// One element deep "death row", optimizes malloc/free/malloc for identical size.
	void *mag_last_free;		// low SHIFT_{TINY,SMALL}_QUANTUM bits indicate the msize
	region_t mag_last_free_rgn; // holds the region for mag_last_free

	free_list_t mag_free_list[256]; // assert( 256 >= MAX( NUM_TINY_SLOTS, NUM_SMALL_SLOTS_LARGEMEM ))
	unsigned mag_bitmap[8];			 // assert( sizeof(mag_bitmap) << 3 >= sizeof(mag_free_list)/sizeof(free_list_t) )

	// the first and last free region in the last block are treated as big blocks in use that are not accounted for
	size_t mag_bytes_free_at_end;
	size_t mag_bytes_free_at_start;
	region_t mag_last_region; // Valid iff mag_bytes_free_at_end || mag_bytes_free_at_start > 0

	// bean counting ...
	unsigned mag_num_objects;
	size_t mag_num_bytes_in_objects;
	size_t num_bytes_in_magazine;

	// recirculation list -- invariant: all regions owned by this magazine that meet the emptiness criteria
	// are located nearer to the head of the list than any region that doesn't satisfy that criteria.
	// Doubly linked list for efficient extraction.
	unsigned recirculation_entries;
	region_trailer_t *firstNode;
	region_trailer_t *lastNode;

	uintptr_t pad[50 - MALLOC_CACHE_LINE / sizeof(uintptr_t)];
} magazine_t;

#if MALLOC_TARGET_64BIT
MALLOC_STATIC_ASSERT(sizeof(magazine_t) == 2560, "Incorrect padding in magazine_t");
#else
MALLOC_STATIC_ASSERT(sizeof(magazine_t) == 1280, "Incorrect padding in magazine_t");
#endif

#define TINY_MAX_MAGAZINES 32 /* MUST BE A POWER OF 2! */
#define TINY_MAGAZINE_PAGED_SIZE                                                   \
	(((sizeof(magazine_t) * (TINY_MAX_MAGAZINES + 1)) + vm_page_quanta_size - 1) & \
	~(vm_page_quanta_size - 1)) /* + 1 for the Depot */

#define SMALL_MAX_MAGAZINES 32 /* MUST BE A POWER OF 2! */
#define SMALL_MAGAZINE_PAGED_SIZE                                                   \
	(((sizeof(magazine_t) * (SMALL_MAX_MAGAZINES + 1)) + vm_page_quanta_size - 1) & \
	~(vm_page_quanta_size - 1)) /* + 1 for the Depot */

#define DEPOT_MAGAZINE_INDEX -1

/****************************** zone itself ***********************************/

/*
 * Note that objects whose adddress are held in pointers here must be pursued
 * individually in the {tiny,small}_in_use_enumeration() routines. See for
 * example the treatment of region_hash_generation and tiny_magazines below.
 */

typedef struct szone_s {	  // vm_allocate()'d, so page-aligned to begin with.
	malloc_zone_t basic_zone; // first page will be given read-only protection
	uint8_t pad[PAGE_MAX_SIZE - sizeof(malloc_zone_t)];

	unsigned long cpu_id_key; // unused
	// remainder of structure is R/W (contains no function pointers)
	unsigned debug_flags;
	void *log_address;

	/* Regions for tiny objects */
	_malloc_lock_s tiny_regions_lock MALLOC_CACHE_ALIGN;
	size_t num_tiny_regions;
	size_t num_tiny_regions_dealloc;
	region_hash_generation_t *tiny_region_generation;
	region_hash_generation_t trg[2];

	int num_tiny_magazines;
	unsigned num_tiny_magazines_mask;
	int num_tiny_magazines_mask_shift;
	magazine_t *tiny_magazines; // array of per-processor magazines

	uintptr_t last_tiny_advise;

	/* Regions for small objects */
	_malloc_lock_s small_regions_lock MALLOC_CACHE_ALIGN;
	size_t num_small_regions;
	size_t num_small_regions_dealloc;
	region_hash_generation_t *small_region_generation;
	region_hash_generation_t srg[2];

	unsigned num_small_slots; // determined by physmem size

	int num_small_magazines;
	unsigned num_small_magazines_mask;
	int num_small_magazines_mask_shift;
	magazine_t *small_magazines; // array of per-processor magazines

	uintptr_t last_small_advise;

	/* large objects: all the rest */
	_malloc_lock_s large_szone_lock MALLOC_CACHE_ALIGN; // One customer at a time for large
	unsigned num_large_objects_in_use;
	unsigned num_large_entries;
	large_entry_t *large_entries; // hashed by location; null entries don't count
	size_t num_bytes_in_large_objects;

#if CONFIG_LARGE_CACHE
	int large_entry_cache_oldest;
	int large_entry_cache_newest;
	large_entry_t large_entry_cache[LARGE_ENTRY_CACHE_SIZE]; // "death row" for large malloc/free
	boolean_t large_legacy_reset_mprotect;
	size_t large_entry_cache_reserve_bytes;
	size_t large_entry_cache_reserve_limit;
	size_t large_entry_cache_bytes; // total size of death row, bytes
#endif

	/* flag and limits pertaining to altered malloc behavior for systems with
	 * large amounts of physical memory */
	unsigned is_largemem;
	unsigned large_threshold;
	unsigned vm_copy_threshold;

	/* security cookie */
	uintptr_t cookie;

	/* Initial region list */
	region_t initial_tiny_regions[INITIAL_NUM_REGIONS];
	region_t initial_small_regions[INITIAL_NUM_REGIONS];

	/* The purgeable zone constructed by create_purgeable_zone() would like to hand off tiny and small
	 * allocations to the default scalable zone. Record the latter as the "helper" zone here. */
	struct szone_s *helper_zone;

	boolean_t flotsam_enabled;
} szone_t;

#define SZONE_PAGED_SIZE round_page_quanta((sizeof(szone_t)))

#endif // __MAGAZINE_ZONE_H
