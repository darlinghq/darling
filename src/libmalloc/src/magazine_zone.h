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

typedef struct _medium_inplace_free_entry_s {
	inplace_linkage_s previous;
	inplace_linkage_s next;
} medium_inplace_free_entry_s, *medium_inplace_free_entry_t;

typedef union {
	small_inplace_free_entry_t small_inplace;
	medium_inplace_free_entry_t medium_inplace;
	inplace_free_entry_t inplace;
	oob_free_entry_t oob;
	void *p;
} free_list_t;

typedef struct {
	inplace_union previous;
	inplace_union next;
} tiny_free_list_t;

typedef unsigned int grain_t; // N.B. wide enough to index all free slots

#define CHECK_REGIONS (1 << 31)
#define DISABLE_ASLR (1 << 30)

#define MAX_RECORDER_BUFFER 256

/*********************	DEFINITIONS for tiny	************************/

/*
 * Memory in the Tiny range is allocated from regions (heaps) pointed to by the
 * szone's hashed_regions pointer.
 *
 * Each region is laid out as a heap, followed by a header block, all within
 * a 1MB (2^20) block.  This means there are 64504 16-byte blocks and the header
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
 * free objects of the corresponding quantum size. In thr tiny region, the free
 * objects for each region are arranged so that they are grouped together in their
 * per-slot freelists and the groups are ordered roughly in the order of regions
 * as they appear in the magazine's region list. This approach helps to reduce
 * fragmentation. Not guaranteeing strictly the same ordering as the regions
 * helps reduce the CPU time required to reduce fragmentation.
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
#define TINY_METADATA_SIZE (sizeof(region_trailer_t) + sizeof(tiny_header_inuse_pair_t) * CEIL_NUM_TINY_BLOCKS_WORDS + (sizeof(region_free_blocks_t) * NUM_TINY_SLOTS))
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

#define TINY_REGION_PAD (TINY_REGION_SIZE - (NUM_TINY_BLOCKS * sizeof(tiny_block_t)) - TINY_METADATA_SIZE)

typedef struct tiny_header_inuse_pair {
	uint32_t header;
	uint32_t inuse;
} tiny_header_inuse_pair_t;

typedef struct {
	// Block indices are +1 so that 0 represents no free block.
	uint16_t first_block;
	uint16_t last_block;
} region_free_blocks_t;

typedef struct region_trailer {
	uint32_t region_cookie;
	volatile int32_t pinned_to_depot;
	struct region_trailer *prev;
	struct region_trailer *next;
	boolean_t recirc_suitable;
	unsigned bytes_used;
	unsigned objects_in_use;  // Used only by tiny allocator.
	mag_index_t mag_index;
} region_trailer_t;

typedef struct tiny_region {
	tiny_block_t blocks[NUM_TINY_BLOCKS];

	region_trailer_t trailer;

	// The interleaved bit arrays comprising the header and inuse bitfields.
	// The unused bits of each component in the last pair will be initialized to sentinel values.
	tiny_header_inuse_pair_t pairs[CEIL_NUM_TINY_BLOCKS_WORDS];

	// Indices of the first and last free block in this region. Value is the
	// block index + 1 so that 0 indicates no free block in this region for the
	// corresponding slot.
	region_free_blocks_t free_blocks_by_slot[NUM_TINY_SLOTS];

	uint8_t pad[TINY_REGION_PAD];
} * tiny_region_t;

// The layout described above should result in a tiny_region_t being 1MB.
MALLOC_STATIC_ASSERT(TINY_REGION_SIZE == (1024 * 1024), "incorrect TINY_REGION_SIZE");
MALLOC_STATIC_ASSERT(sizeof(struct tiny_region) == TINY_REGION_SIZE, "incorrect tiny_region_size");

/*
 * Per-region meta data for tiny allocator
 */
#define REGION_TRAILER_FOR_TINY_REGION(r) (&(((tiny_region_t)(r))->trailer))
#define MAGAZINE_INDEX_FOR_TINY_REGION(r) (REGION_TRAILER_FOR_TINY_REGION(r)->mag_index)
#define BYTES_USED_FOR_TINY_REGION(r) (REGION_TRAILER_FOR_TINY_REGION(r)->bytes_used)
#define OBJECTS_IN_USE_FOR_TINY_REGION(r) (REGION_TRAILER_FOR_TINY_REGION(r)->objects_in_use)

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

/*
 * Get the pointer for a given index in a region.
 */
#define TINY_PTR_FOR_INDEX(index, region) (region_t)((void *)(((uintptr_t)(region)) + ((index) << SHIFT_TINY_QUANTUM)))

/*
 * Offset back to an szone_t given prior knowledge that this rack_t
 * is contained within an szone_t.
 *
 * Note: the only place this is used, the dtrace probes, only occurs
 *       when the rack has been set up inside a scalable zone. Should
 *       this ever be used somewhere that this does not hold true
 *       (say, the test cases) then the pointer returned will be junk.
 */
#define TINY_SZONE_FROM_RACK(_r) \
		(szone_t *)((uintptr_t)(_r) - offsetof(struct szone_s, tiny_rack))


#if !CONFIG_TINY_CACHE
#warning CONFIG_TINY_CACHE turned off
#endif

#define TINY_REGION_PAYLOAD_BYTES (NUM_TINY_BLOCKS * TINY_QUANTUM)

/*********************	DEFINITIONS for small	************************/

/*
 * Memory in the small range is allocated from regions (heaps) pointed to by the szone's hashed_regions
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
 * SMALL_IS_OOB is used to mark the MSB of OOB free list entries to show that they are in use, and
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
#define SMALL_BYTES_FOR_MSIZE(_m) ((uint32_t)(_m) << SHIFT_SMALL_QUANTUM)
#define SMALL_MSIZE_FOR_BYTES(_b) ((_b) >> SHIFT_SMALL_QUANTUM)

#define SMALL_PREVIOUS_MSIZE(ptr) (*SMALL_METADATA_FOR_PTR(ptr - 1) & ~SMALL_IS_FREE)

/*
 * Convert from msize unit to free list slot.
 */
#define SMALL_FREE_SLOT_COUNT(_r) \
		(NUM_SMALL_SLOTS + 1)
#define SMALL_FREE_SLOT_FOR_MSIZE(_r, _m) \
		(((_m) <= SMALL_FREE_SLOT_COUNT(_r)) ? ((_m) - 1) : (SMALL_FREE_SLOT_COUNT(_r) - 1))
/* compare with MAGAZINE_FREELIST_BITMAP_WORDS */
#define SMALL_FREELIST_BITMAP_WORDS(_r) ((SMALL_FREE_SLOT_COUNT(_r) + 31) >> 5)

/*
 * Offset back to an szone_t given prior knowledge that this rack_t
 * is contained within an szone_t.
 *
 * Note: the only place this is used, the dtrace probes, only occurs
 *       when the rack has been set up inside a scalable zone. Should
 *       this ever be used somewhere that this does not hold true
 *       (say, the test cases) then the pointer returned will be junk.
 */
#define SMALL_SZONE_FROM_RACK(_r) \
		(szone_t *)((uintptr_t)(_r) - offsetof(struct szone_s, small_rack))

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
MALLOC_STATIC_ASSERT(SMALL_REGION_SIZE == (8 * 1024 * 1024), "incorrect SMALL_REGION_SIZE");
MALLOC_STATIC_ASSERT(sizeof(struct small_region) == SMALL_REGION_SIZE, "incorrect small_region_size");

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

/*********************	DEFINITIONS for medium	************************/

/*
 * Memory in the medium range is allocated from regions (heaps) pointed to by the szone's hashed_regions
 * pointer.
 *
 * Each region is laid out as a heap, followed by the metadata array, all within an 512MB block.
 * The array is arranged as an array of shorts, one for each MEDIUM_QUANTUM in the heap. There are
 * 16382 32k-blocks and the array is 16382*2 bytes, which totals 8387966, leaving 32,772b unused.
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
 * The medium zone represents the free list in one of two ways:
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
 * The szone maintains an array of 256 freelists, each of which is used to hold free objects
 * of the corresponding quantum size.
 */

#define MEDIUM_IS_FREE (1 << 15)
#define MEDIUM_IS_ADVISED (1 << 15)
#define FOLLOWING_MEDIUM_PTR(ptr, msize) (((unsigned char *)(ptr)) + ((msize) << SHIFT_MEDIUM_QUANTUM))
#define MEDIUM_MAX_MSIZE ((uint16_t)(NUM_MEDIUM_BLOCKS >> SHIFT_MEDIUM_QUANTUM) \
		& ~(uint16_t)MEDIUM_IS_FREE)

// Ensure that the we don't overflow the number of blocks that msize can
// represent (without running into the free bit).
MALLOC_STATIC_ASSERT(NUM_MEDIUM_BLOCKS <= (uint16_t)(~MEDIUM_IS_FREE),
		"NUM_MEDIUM_BLOCKS should fit into a msize_t");

/*
 * MEDIUM_IS_OOB is used mark to the MSB of OOB free list entries to show that they are in use, and
 * distinguish them from their initial, empty, state.
 */
#define MEDIUM_IS_OOB (1 << 15)

#define MEDIUM_ENTROPY_BITS 11
#define MEDIUM_ENTROPY_MASK ((1 << MEDIUM_ENTROPY_BITS) - 1)

/*
 * Avoid having so much entropy that the end of a valid medium allocation
 * might overrun the end of the medium region.
 */
#if MEDIUM_ENTROPY_MASK + NUM_MEDIUM_SLOTS > NUM_MEDIUM_BLOCKS
#error Too many entropy bits for medium region requested
#endif

#define MEDIUM_METADATA_SIZE (sizeof(region_trailer_t) + \
		(NUM_MEDIUM_BLOCKS * sizeof(msize_t)) + \
		(NUM_MEDIUM_BLOCKS * sizeof(msize_t)))
// Note: The other instances of x_REGION_SIZE use PAGE_MAX_SIZE as the rounding
// and truncating constant but because medium's quanta size is larger than a
// page, it's used instead.
#define MEDIUM_REGION_SIZE ((NUM_MEDIUM_BLOCKS * MEDIUM_QUANTUM + \
		MEDIUM_METADATA_SIZE + MEDIUM_QUANTUM - 1) & ~(MEDIUM_QUANTUM - 1))

#define MEDIUM_METADATA_START (NUM_MEDIUM_BLOCKS * MEDIUM_QUANTUM)

/*
 * Beginning and end pointers for a region's heap.
 */
#define MEDIUM_REGION_ADDRESS(region) ((unsigned char *)region)
#define MEDIUM_REGION_END(region) (MEDIUM_REGION_ADDRESS(region) + (NUM_MEDIUM_BLOCKS * MEDIUM_QUANTUM))

/*
 * Locate the heap base for a pointer known to be within a medium region.
 */
#define MEDIUM_REGION_FOR_PTR(_p) ((void *)((uintptr_t)(_p) & ~((1ull << MEDIUM_BLOCKS_ALIGN) - 1)))
#define MEDIUM_OFFSET_FOR_PTR(_p) ((uintptr_t)(_p) & ((1ull << MEDIUM_BLOCKS_ALIGN) - 1))

/*
 * Convert between byte and msize units.
 */
#define MEDIUM_BYTES_FOR_MSIZE(_m) ((uint32_t)(_m) << SHIFT_MEDIUM_QUANTUM)
#define MEDIUM_MSIZE_FOR_BYTES(_b) ((_b) >> SHIFT_MEDIUM_QUANTUM)

#define MEDIUM_PREVIOUS_MSIZE(ptr) (*MEDIUM_METADATA_FOR_PTR(ptr - 1) & ~MEDIUM_IS_FREE)

/*
 * Convert from msize unit to free list slot.
 */
#define MEDIUM_FREE_SLOT_COUNT(_r) (NUM_MEDIUM_SLOTS + 1)
#define MEDIUM_FREE_SLOT_FOR_MSIZE(_r, _m) \
		(((_m) <= MEDIUM_FREE_SLOT_COUNT(_r)) ? ((_m) - 1) : (MEDIUM_FREE_SLOT_COUNT(_r) - 1))
/* compare with MAGAZINE_FREELIST_BITMAP_WORDS */
#define MEDIUM_FREELIST_BITMAP_WORDS(_r) ((MEDIUM_FREE_SLOT_COUNT(_r) + 31) >> 5)

/*
 * Offset back to an szone_t given prior knowledge that this rack_t
 * is contained within an szone_t.
 *
 * Note: the only place this is used, the dtrace probes, only occurs
 *       when the rack has been set up inside a scalable zone. Should
 *       this ever be used somewhere that this does not hold true
 *       (say, the test cases) then the pointer returned will be junk.
 */
#define MEDIUM_SZONE_FROM_RACK(_r) \
		(szone_t *)((uintptr_t)(_r) - offsetof(struct szone_s, medium_rack))

/*
 * Layout of a medium region
 */
typedef uint32_t medium_block_t[MEDIUM_QUANTUM / sizeof(uint32_t)];
#define MEDIUM_HEAP_SIZE (NUM_MEDIUM_BLOCKS * sizeof(medium_block_t))
#define MEDIUM_OOB_COUNT ((MEDIUM_REGION_SIZE - MEDIUM_HEAP_SIZE - \
		MEDIUM_METADATA_SIZE) / sizeof(oob_free_entry_s))
#define MEDIUM_OOB_SIZE (MEDIUM_OOB_COUNT * sizeof(oob_free_entry_s))
#define MEDIUM_REGION_PAD (MEDIUM_REGION_SIZE - MEDIUM_HEAP_SIZE - \
		MEDIUM_METADATA_SIZE - MEDIUM_OOB_SIZE)

typedef struct medium_region {
	medium_block_t blocks[NUM_MEDIUM_BLOCKS];
	region_trailer_t trailer;
	msize_t medium_meta_words[NUM_MEDIUM_BLOCKS];
	msize_t medium_madvise_words[NUM_MEDIUM_BLOCKS];
	oob_free_entry_s medium_oob_free_entries[MEDIUM_OOB_COUNT];
	uint8_t pad[MEDIUM_REGION_PAD];
} * medium_region_t;

// The layout described above should result in a medium_region_t being 512MB.
MALLOC_STATIC_ASSERT(sizeof(struct medium_region) == 128 * 1024 * 1024,
		"incorrect medium_region_size");

/*
 * Per-region meta data for medium allocator
 */
#define REGION_TRAILER_FOR_MEDIUM_REGION(r) (&(((medium_region_t)(r))->trailer))
#define MAGAZINE_INDEX_FOR_MEDIUM_REGION(r) (REGION_TRAILER_FOR_MEDIUM_REGION(r)->mag_index)
#define BYTES_USED_FOR_MEDIUM_REGION(r) (REGION_TRAILER_FOR_MEDIUM_REGION(r)->bytes_used)

/*
 * Locate the metadata base for a pointer known to be within a medium region.
 */
#define MEDIUM_META_HEADER_FOR_PTR(_p) (((medium_region_t)MEDIUM_REGION_FOR_PTR(_p))->medium_meta_words)
#define MEDIUM_MADVISE_HEADER_FOR_PTR(_p) (((medium_region_t)MEDIUM_REGION_FOR_PTR(_p))->medium_madvise_words)

/*
 * Compute the metadata index for a pointer known to be within a medium region.
 */
#define MEDIUM_META_INDEX_FOR_PTR(_p) (((uintptr_t)(_p) >> SHIFT_MEDIUM_QUANTUM) & (NUM_MEDIUM_CEIL_BLOCKS - 1))
#define MEDIUM_PTR_FOR_META_INDEX(_region, _i) ((uintptr_t)(_region) + MEDIUM_BYTES_FOR_MSIZE(_i))

/*
 * Find the metadata word for a pointer known to be within a medium region.
 */
#define MEDIUM_METADATA_FOR_PTR(_p) (MEDIUM_META_HEADER_FOR_PTR(_p) + MEDIUM_META_INDEX_FOR_PTR(_p))

/*
 * Determine whether a pointer known to be within a medium region points to memory which is free.
 */
#define MEDIUM_PTR_IS_FREE(_p) (*MEDIUM_METADATA_FOR_PTR(_p) & MEDIUM_IS_FREE)

/*
 * Extract the msize value for a pointer known to be within a medium region.
 */
#define MEDIUM_PTR_SIZE(_p) (*MEDIUM_METADATA_FOR_PTR(_p) & ~MEDIUM_IS_FREE)

#if !CONFIG_MEDIUM_CACHE
#warning CONFIG_MEDIUM_CACHE turned off
#endif

#define MEDIUM_REGION_PAYLOAD_BYTES (NUM_MEDIUM_BLOCKS * MEDIUM_QUANTUM)

/*************************  DEFINITIONS for large  ****************************/


typedef struct large_entry_s {
	vm_address_t address;
	vm_size_t size;
	boolean_t did_madvise_reusable;
} large_entry_t;

#if !CONFIG_LARGE_CACHE && DEBUG_MALLOC
#warning CONFIG_LARGE_CACHE turned off
#endif

#if CONFIG_MEDIUM_ALLOCATOR
#define LARGE_THRESHOLD(szone) ((szone)->is_medium_engaged ? \
		(MEDIUM_LIMIT_THRESHOLD) : (SMALL_LIMIT_THRESHOLD))
#else // CONFIG_MEDIUM_ALLOCATOR
#define LARGE_THRESHOLD(szone) (SMALL_LIMIT_THRESHOLD)
#endif // CONFIG_MEDIUM_ALLOCATOR


/*******************************************************************************
 * Per-processor magazine for tiny and small allocators
 ******************************************************************************/

typedef struct magazine_s { // vm_allocate()'d, so the array of magazines is page-aligned to begin with.
	// Take magazine_lock first,  Depot lock when needed for recirc, then szone->{tiny,small}_regions_lock when needed for alloc
	_malloc_lock_s magazine_lock MALLOC_CACHE_ALIGN;
	// Protection for the crtical section that does allocate_pages outside the magazine_lock
	volatile boolean_t alloc_underway;

	// One element deep "death row", optimizes malloc/free/malloc for identical size.
	void *mag_last_free;
	msize_t mag_last_free_msize;	// msize for mag_last_free
#if MALLOC_TARGET_64BIT
	uint32_t _pad;
#endif
	region_t mag_last_free_rgn; // holds the region for mag_last_free

	free_list_t mag_free_list[MAGAZINE_FREELIST_SLOTS];
	uint32_t mag_bitmap[MAGAZINE_FREELIST_BITMAP_WORDS];

	// the first and last free region in the last block are treated as big blocks in use that are not accounted for
	size_t mag_bytes_free_at_end;
	size_t mag_bytes_free_at_start;
	region_t mag_last_region; // Valid iff mag_bytes_free_at_end || mag_bytes_free_at_start > 0

	// bean counting ...
	size_t mag_num_bytes_in_objects;
	size_t num_bytes_in_magazine;
	unsigned mag_num_objects;

	// recirculation list -- invariant: all regions owned by this magazine that meet the emptiness criteria
	// are located nearer to the head of the list than any region that doesn't satisfy that criteria.
	// Doubly linked list for efficient extraction.
	unsigned recirculation_entries;
	region_trailer_t *firstNode;
	region_trailer_t *lastNode;

#if MALLOC_TARGET_64BIT
	uintptr_t pad[320 - 14 - MAGAZINE_FREELIST_SLOTS -
			(MAGAZINE_FREELIST_BITMAP_WORDS + 1) / 2];
#else
	uintptr_t pad[320 - 16 - MAGAZINE_FREELIST_SLOTS -
			MAGAZINE_FREELIST_BITMAP_WORDS];
#endif

} magazine_t;

#if MALLOC_TARGET_64BIT
MALLOC_STATIC_ASSERT(sizeof(magazine_t) == 2560, "Incorrect padding in magazine_t");
#else
MALLOC_STATIC_ASSERT(sizeof(magazine_t) == 1280, "Incorrect padding in magazine_t");
#endif

#define TINY_MAX_MAGAZINES 64 /* MUST BE A POWER OF 2! */
#define TINY_MAGAZINE_PAGED_SIZE                                                   \
	(((sizeof(magazine_t) * (TINY_MAX_MAGAZINES + 1)) + vm_page_quanta_size - 1) & \
	~(vm_page_quanta_size - 1)) /* + 1 for the Depot */

#define SMALL_MAX_MAGAZINES 64 /* MUST BE A POWER OF 2! */
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

	/* Allocation racks per allocator type. */
	struct rack_s tiny_rack;
	struct rack_s small_rack;
	struct rack_s medium_rack;

	/* large objects: all the rest */
	_malloc_lock_s large_szone_lock MALLOC_CACHE_ALIGN; // One customer at a time for large
	unsigned num_large_objects_in_use;
	unsigned num_large_entries;
	large_entry_t *large_entries; // hashed by location; null entries don't count
	size_t num_bytes_in_large_objects;

#if CONFIG_LARGE_CACHE
	int large_entry_cache_oldest;
	int large_entry_cache_newest;
	large_entry_t large_entry_cache[LARGE_ENTRY_CACHE_SIZE_HIGH]; // "death row" for large malloc/free
	int large_cache_depth;
	size_t large_cache_entry_limit;
	boolean_t large_legacy_reset_mprotect;
	size_t large_entry_cache_reserve_bytes;
	size_t large_entry_cache_reserve_limit;
	size_t large_entry_cache_bytes; // total size of death row, bytes
#endif

	/* flag and limits pertaining to altered malloc behavior for systems with
	 * large amounts of physical memory */
	bool is_medium_engaged;

	/* security cookie */
	uintptr_t cookie;

	/* The purgeable zone constructed by create_purgeable_zone() would like to hand off tiny and small
	 * allocations to the default scalable zone. Record the latter as the "helper" zone here. */
	struct szone_s *helper_zone;

	boolean_t flotsam_enabled;
} szone_t;

#define SZONE_PAGED_SIZE round_page_quanta((sizeof(szone_t)))

#endif // __MAGAZINE_ZONE_H
