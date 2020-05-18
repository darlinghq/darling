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

#include "internal.h"

// The address and size of the block in mag_last_free are combined. These
// macros abstract construction of the combined value and extraction of the
// size and pointer.
#define TINY_MAG_LAST_FREE_FROM_PTR_AND_MSIZE(ptr, msize) (void *)(((uintptr_t)(ptr))|((msize_t)msize))
#define TINY_PTR_FROM_MAG_LAST_FREE(x) (void *)(((uintptr_t)(x)) & ~(TINY_QUANTUM - 1))
#define TINY_MSIZE_FROM_MAG_LAST_FREE(x) (msize_t)(((uintptr_t)(x)) & (TINY_QUANTUM - 1))

// Adjusts the pointer part of mag_last_free by a given amount in bytes. Must be
// a multiple of the quantum size (not checked).
#define TINY_MAG_LAST_FREE_PTR_ADJUST_PTR(x, size) (x) = ((void *)(x) + (size))

// Decrements the size part of mag_last_free by a given msize value. Must not
// reduce the msize part below zero (not checked).
#define TINY_MAG_LAST_FREE_PTR_DEC_MSIZE(x, msize_delta) (x) = ((void *)(x) - (msize_delta))

static MALLOC_INLINE MALLOC_ALWAYS_INLINE
mag_index_t
tiny_mag_get_thread_index(void)
{
#if CONFIG_TINY_USES_HYPER_SHIFT
	if (os_likely(_os_cpu_number_override == -1)) {
		return _os_cpu_number() >> hyper_shift;
	} else {
		return _os_cpu_number_override >> hyper_shift;
	}
#else // CONFIG_SMALL_USES_HYPER_SHIFT
	if (os_likely(_os_cpu_number_override == -1)) {
		return _os_cpu_number();
	} else {
		return _os_cpu_number_override;
	}
#endif // CONFIG_SMALL_USES_HYPER_SHIFT
}

static inline grain_t
tiny_slot_from_msize(msize_t msize)
{
	return (!msize || (msize > NUM_TINY_SLOTS) ? NUM_TINY_SLOTS : msize - 1);
}

/*
 * Get the size of the previous free block, which is stored in the last two
 * bytes of the block.  If the previous block is not free, then the result is
 * undefined.
 */
static msize_t
get_tiny_previous_free_msize(const void *ptr)
{
	// check whether the previous block is in the tiny region and a block header
	// if so, then the size of the previous block is one, and there is no stored
	// size.
	if (ptr != TINY_REGION_FOR_PTR(ptr)) {
		void *prev_block = (void *)((uintptr_t)ptr - TINY_QUANTUM);
		uint32_t *prev_header = TINY_BLOCK_HEADER_FOR_PTR(prev_block);
		msize_t prev_index = TINY_INDEX_FOR_PTR(prev_block);
		if (BITARRAY_BIT(prev_header, prev_index)) {
			return 1;
		}
		return TINY_PREVIOUS_MSIZE(ptr);
	}
	// don't read possibly unmapped memory before the beginning of the region
	return 0;
}

static MALLOC_INLINE void
set_tiny_meta_header_in_use(const void *ptr, msize_t msize)
{
	uint32_t *block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
	msize_t index = TINY_INDEX_FOR_PTR(ptr);
	msize_t clr_msize = msize - 1;
	msize_t midx = (index >> 5) << 1;
	uint32_t val = (1 << (index & 31));

#if DEBUG_MALLOC
	if (msize > NUM_TINY_SLOTS) {
		malloc_report(ASL_LEVEL_ERR, "set_tiny_meta_header_in_use() invariant broken %p %d\n", ptr, msize);
	}
	if ((unsigned)index + (unsigned)msize > 0x10000) {
		malloc_report(ASL_LEVEL_ERR, "set_tiny_meta_header_in_use() invariant broken (2) %p %d\n", ptr, msize);
	}
	if (msize > TINY_BITMAP_RANGE_LIMIT) {
		malloc_report(ASL_LEVEL_ERROR, "set_tiny_meta_header_in_use() invariant broken (3) %p %d\n", ptr, msize);
	}
#endif

	block_header[midx] |= val;	 // BITARRAY_SET(block_header, index);
	block_header[midx + 1] |= val; // BITARRAY_SET(in_use, index);

	// bitarray_mclr(block_header, index, end_bit);
	// bitarray_mclr(in_use, index, end_bit);

	index++;
	midx = (index >> 5) << 1;

	unsigned start = index & 31;
	unsigned end = start + clr_msize;

#if defined(__LP64__)
	if (end > 63) {
		unsigned mask0 = (0xFFFFFFFFU >> (31 - start)) >> 1;
		unsigned mask1 = (0xFFFFFFFFU << (end - 64));
		block_header[midx + 0] &= mask0; // clear header
		block_header[midx + 1] &= mask0; // clear in_use
		block_header[midx + 2] = 0;		 // clear header
		block_header[midx + 3] = 0;		 // clear in_use
		block_header[midx + 4] &= mask1; // clear header
		block_header[midx + 5] &= mask1; // clear in_use
	} else
#endif
		if (end > 31) {
			unsigned mask0 = (0xFFFFFFFFU >> (31 - start)) >> 1;
			unsigned mask1 = (0xFFFFFFFFU << (end - 32));
			block_header[midx + 0] &= mask0;
			block_header[midx + 1] &= mask0;
			block_header[midx + 2] &= mask1;
			block_header[midx + 3] &= mask1;
		} else {
			unsigned mask = (0xFFFFFFFFU >> (31 - start)) >> 1;
			mask |= (0xFFFFFFFFU << end);
			block_header[midx + 0] &= mask;
			block_header[midx + 1] &= mask;
		}

	// we set the block_header bit for the following block to reaffirm next block is a block
	index += clr_msize;
	midx = (index >> 5) << 1;
	val = (1 << (index & 31));
	block_header[midx] |= val; // BITARRAY_SET(block_header, (index+clr_msize));
#if DEBUG_MALLOC
	{
		boolean_t ff;
		msize_t mf;

		mf = get_tiny_meta_header(ptr, &ff);
		if (msize != mf) {
			malloc_report(ASL_LEVEL_INFO, "setting header for tiny in_use %p : %d\n", ptr, msize);
			malloc_report(ASL_LEVEL_INFO, "reading header for tiny %p : %d %d\n", ptr, mf, ff);
		}
	}
#endif
}

static MALLOC_INLINE void set_tiny_meta_header_in_use_1(const void *ptr) // As above with msize == 1
{
	uint32_t *block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
	msize_t index = TINY_INDEX_FOR_PTR(ptr);
	msize_t midx = (index >> 5) << 1;
	uint32_t val = (1 << (index & 31));

	block_header[midx] |= val;	 // BITARRAY_SET(block_header, index);
	block_header[midx + 1] |= val; // BITARRAY_SET(in_use, index);

	index++;
	midx = (index >> 5) << 1;
	val = (1 << (index & 31));

	block_header[midx] |= val; // BITARRAY_SET(block_header, (index+clr_msize))
}

static MALLOC_INLINE void
set_tiny_meta_header_middle(const void *ptr)
{
	// indicates this block is in the middle of an in use block
	uint32_t *block_header;
	uint32_t *in_use;
	msize_t index;

	block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
	in_use = TINY_INUSE_FOR_HEADER(block_header);
	index = TINY_INDEX_FOR_PTR(ptr);

	BITARRAY_CLR(block_header, index);
	BITARRAY_CLR(in_use, index);
}

static MALLOC_INLINE void
set_tiny_meta_header_free(const void *ptr, msize_t msize)
{
	// !msize is acceptable and means 65536
	uint32_t *block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
	msize_t index = TINY_INDEX_FOR_PTR(ptr);
	msize_t midx = (index >> 5) << 1;
	uint32_t val = (1 << (index & 31));

#if DEBUG_MALLOC
	if ((unsigned)index + (unsigned)msize > 0x10000) {
		malloc_report(ASL_LEVEL_ERR, "setting header for tiny free %p msize too large: %d\n", ptr, msize);
	}
#endif

	block_header[midx] |= val;		// BITARRAY_SET(block_header, index);
	block_header[midx + 1] &= ~val; // BITARRAY_CLR(in_use, index);

	// mark the end of this block if msize is > 1.  For msize == 0, the whole
	// region is free, so there is no following block. For msize == 1, there is
	// no space to write the size on 64 bit systems.  The size for 1 quantum
	// blocks is computed from the metadata bitmaps.
	if (msize > 1) {
		void *follower = FOLLOWING_TINY_PTR(ptr, msize);
		TINY_PREVIOUS_MSIZE(follower) = msize;
		TINY_FREE_SIZE(ptr) = msize;
	}
	if (msize == 0) {
		TINY_FREE_SIZE(ptr) = msize;
	}
#if DEBUG_MALLOC
	boolean_t ff;
	msize_t mf = get_tiny_meta_header(ptr, &ff);
	if ((msize != mf) || !ff) {
		malloc_report(ASL_LEVEL_INFO, "setting header for tiny free %p : %u\n", ptr, msize);
		malloc_report(ASL_LEVEL_INFO, "reading header for tiny %p : %u %u\n", ptr, mf, ff);
	}
#endif
}

static MALLOC_INLINE boolean_t
tiny_meta_header_is_free(const void *ptr)
{
	uint32_t *block_header;
	uint32_t *in_use;
	msize_t index;

	block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
	in_use = TINY_INUSE_FOR_HEADER(block_header);
	index = TINY_INDEX_FOR_PTR(ptr);
	if (!BITARRAY_BIT(block_header, index)) {
		return 0;
	}
	return !BITARRAY_BIT(in_use, index);
}

static MALLOC_INLINE void *
tiny_previous_preceding_free(void *ptr, msize_t *prev_msize)
{
	// returns the previous block, assuming and verifying it's free
	uint32_t *block_header;
	uint32_t *in_use;
	msize_t index;
	msize_t previous_msize;
	msize_t previous_index;
	void *previous_ptr;

	block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
	in_use = TINY_INUSE_FOR_HEADER(block_header);
	index = TINY_INDEX_FOR_PTR(ptr);

	if (!index) {
		return NULL;
	}
	if ((previous_msize = get_tiny_previous_free_msize(ptr)) > index) {
		return NULL;
	}

	previous_index = index - previous_msize;
	previous_ptr = (void *)((uintptr_t)TINY_REGION_FOR_PTR(ptr) + TINY_BYTES_FOR_MSIZE(previous_index));
	if (!BITARRAY_BIT(block_header, previous_index)) {
		return NULL;
	}
	if (BITARRAY_BIT(in_use, previous_index)) {
		return NULL;
	}
	if (get_tiny_free_size(previous_ptr) != previous_msize) {
		return NULL;
	}

	// conservative check did match true check
	*prev_msize = previous_msize;
	return previous_ptr;
}

// Given a region and a slot index, scans regions that precede the region on
// the magazine region list for one that has something on its free list for
// the given slot and returns the address of the last such block, or NULL if
// there is none. To reduce overhead, we scan forward from the first region
// looking for our region and noting the last on the freelist for the given slot
// for each earlier region, stopping once we have looked at 5 regions if we did
// not reach our own. This has the effect of keeping the blocks for early
// regions near the front of the freelist while not spending large amounts of
// time looking for the "best" place to put this region's free blocks when there
// are many regions in the magazine.
static MALLOC_INLINE void *
tiny_earlier_region_last_free(magazine_t *tiny_mag_ptr,
		region_trailer_t *trailer, grain_t slot)
{
	int count = 0;
	uint16_t target_block = 0;
	region_trailer_t *target_trailer = NULL;
	region_trailer_t *next_trailer = tiny_mag_ptr->firstNode;

	while (next_trailer && next_trailer != trailer && count++ < 5) {
		tiny_region_t r = TINY_REGION_FOR_PTR(next_trailer);
		uint16_t block = r->free_blocks_by_slot[slot].last_block;
		if (block) {
			target_block = block;
			target_trailer = next_trailer;
		}
		next_trailer = next_trailer->next;
	}
	return target_block ? TINY_PTR_FOR_INDEX(target_block - 1,
			TINY_REGION_FOR_PTR(target_trailer)) : NULL;
}

static MALLOC_INLINE void
tiny_update_region_free_list_for_remove(grain_t slot, tiny_free_list_t *ptr,
		tiny_free_list_t *new_head)
{
	uint16_t ptr_index = TINY_INDEX_FOR_PTR(ptr);
	tiny_region_t ptr_region = TINY_REGION_FOR_PTR(ptr);
	region_free_blocks_t *blocks = &ptr_region->free_blocks_by_slot[slot];
	MALLOC_ASSERT(ptr_index == blocks->first_block - 1);

	if (new_head && (TINY_REGION_FOR_PTR(new_head) == TINY_REGION_FOR_PTR(ptr))) {
		uint16_t new_head_block = TINY_INDEX_FOR_PTR(new_head) + 1;
		if (blocks->first_block == blocks->last_block) {
			blocks->last_block = new_head_block;
		}
		blocks->first_block = new_head_block;
	} else {
		// No more entries in this region.
		blocks->first_block = blocks->last_block = 0;
	}
}

void
tiny_print_region_free_list(void *ptr, grain_t slot)
{
	tiny_region_t region = TINY_REGION_FOR_PTR(ptr);
	region_free_blocks_t *blocks = &region->free_blocks_by_slot[slot];
	malloc_printf("For region %p, first block: %d (%p), last block: %d (%p)\n",
			region,
			blocks->first_block,
			blocks->first_block ? TINY_PTR_FOR_INDEX(blocks->first_block - 1, region) : (void *)0,
			blocks->last_block,
			blocks->last_block ? TINY_PTR_FOR_INDEX(blocks->last_block - 1, region) : (void *)0);
}

/*
 * Adds an item to the proper free list, and also marks the meta-header of the
 * block properly.
 * Assumes szone has been locked
 */
static void
tiny_free_list_add_ptr(rack_t *rack, magazine_t *tiny_mag_ptr, void *ptr, msize_t msize)
{
	grain_t slot = (!msize || (msize > NUM_TINY_SLOTS)) ? NUM_TINY_SLOTS : msize - 1;
	tiny_free_list_t *free_ptr = ptr;
	tiny_free_list_t *free_head = tiny_mag_ptr->mag_free_list[slot].p;

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "in %s, ptr=%p, msize=%d\n", __FUNCTION__, ptr, msize);
	}
	if (((uintptr_t)ptr) & (TINY_QUANTUM - 1)) {
		malloc_zone_error(rack->debug_flags, true, "tiny_free_list_add_ptr: Unaligned ptr: %p\n", ptr);
	}
#endif
	set_tiny_meta_header_free(ptr, msize);
	if (free_head) {
#if DEBUG_MALLOC
		if (free_list_unchecksum_ptr(szone, &free_head->previous)) {
			malloc_zone_error(rack->debug_flags, true,
					"tiny_free_list_add_ptr: Internal invariant broken (free_head->previous): "
					"ptr=%p slot=%d free_head=%p previous=%p\n", ptr, slot, (void *)free_head, free_head->previous.p);
		}
		if (!tiny_meta_header_is_free(free_head)) {
			malloc_zone_error(rack->debug_flags, true,
					"tiny_free_list_add_ptr: Internal invariant broken (free_head is not a free pointer): "
					"ptr=%p slot=%d free_head=%p\n", ptr, slot, (void *)free_head);
		}
#endif
	} else {
		BITMAPV_SET(tiny_mag_ptr->mag_bitmap, slot);
	}

	tiny_region_t region = TINY_REGION_FOR_PTR(ptr);
	region_free_blocks_t *free_blocks = &region->free_blocks_by_slot[slot];
	uint16_t first_free_block_index = free_blocks->first_block;
	uint16_t this_block_index = TINY_INDEX_FOR_PTR(ptr);

	if (first_free_block_index) {
		// This region already has something on its free list.
		tiny_free_list_t *old_first_free = TINY_PTR_FOR_INDEX(first_free_block_index - 1, region);
		tiny_free_list_t *prev_ptr = free_list_unchecksum_ptr(rack, &old_first_free->previous);
		if (!prev_ptr) {
			// Old first item was the first item in the magazine free list -
			// update the magazine head pointer to point to this block.
			tiny_mag_ptr->mag_free_list[slot].p = free_ptr;
		} else {
			prev_ptr->next.u = free_list_checksum_ptr(rack, free_ptr); // XXX
		}

		// Set our previous pointer to the one from the old first block.
		// It's already checksummed, so just copy it directly. It is NULL if
		// the old first block was also the first block on the magazine free
		// list.
		free_ptr->previous.u = old_first_free->previous.u;

		// Our "next" pointer always points to the block that used to be first
		// and we are always its predecessor.
		free_ptr->next.u = free_list_checksum_ptr(rack, old_first_free);
		old_first_free->previous.u = free_list_checksum_ptr(rack, free_ptr);

		// Update the first free block index for this region.
		free_blocks->first_block = this_block_index + 1;
	} else {
		// Free list for this region is empty. Add ourselves to the magazine
		// free list between the last block of the preceding region that has
		// a free block and the first block of the next region with a free
		// block (either of which may not exist).
		tiny_free_list_t *prev_free = NULL;
		tiny_free_list_t *next_free;

		// If the magazine free list is empty, we know we are going to add at
		// the front. Otherwise, find the correct place. If we are freeing to
		// the recirc depot, we can always free to the front because we don't
		// care about ordering in the depot (since no allocations occur there).
		mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(region);
		if (mag_index != DEPOT_MAGAZINE_INDEX
				&& tiny_mag_ptr->mag_free_list[slot].p) {
			region_trailer_t *trailer = REGION_TRAILER_FOR_TINY_REGION(region);
			prev_free = tiny_earlier_region_last_free(tiny_mag_ptr, trailer, slot);
		}
		if (!prev_free) {
			// We are becoming the head of the magazine free list.
			next_free = tiny_mag_ptr->mag_free_list[slot].p;
			tiny_mag_ptr->mag_free_list[slot].p = free_ptr;
		} else {
			next_free = free_list_unchecksum_ptr(rack, &prev_free->next);
			prev_free->next.u = free_list_checksum_ptr(rack, free_ptr);
		}
		free_ptr->previous.u = free_list_checksum_ptr(rack, prev_free);

		if (next_free) {
			next_free->previous.u = free_list_checksum_ptr(rack, free_ptr);
		}
		free_ptr->next.u = free_list_checksum_ptr(rack, next_free);

		// Set the first and last free block index for this region.
		free_blocks->first_block = free_blocks->last_block =
				this_block_index + 1;
	}
}

/*
 * Removes the item pointed to by ptr in the proper free list.
 * Assumes szone has been locked
 */
static void
tiny_free_list_remove_ptr(rack_t *rack, magazine_t *tiny_mag_ptr, void *ptr, msize_t msize)
{
	grain_t slot = tiny_slot_from_msize(msize);
	tiny_free_list_t *free_ptr = ptr, *next, *previous;

	next = free_list_unchecksum_ptr(rack, &free_ptr->next);
	previous = free_list_unchecksum_ptr(rack, &free_ptr->previous);

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "In %s, ptr=%p, msize=%d\n", __FUNCTION__, ptr, msize);
	}
#endif
	if (!previous) {
		// The block to remove is the head of the free list
#if DEBUG_MALLOC
		if (tiny_mag_ptr->mag_free_list[slot] != ptr) {
			malloc_zone_error(rack->debug_flags, true,
					"tiny_free_list_remove_ptr: Internal invariant broken (tiny_mag_ptr->mag_free_list[slot]): "
					"ptr=%p slot=%d msize=%d tiny_mag_ptr->mag_free_list[slot]=%p\n", ptr, slot, msize,
					(void *)tiny_mag_ptr->mag_free_list[slot]);
			return;
		}
#endif
		tiny_mag_ptr->mag_free_list[slot].p = next;
		if (!next) {
			BITMAPV_CLR(tiny_mag_ptr->mag_bitmap, slot);
		}
	} else {
		// Check that the next pointer of "previous" points to free_ptr.
		tiny_free_list_t *prev_next = free_list_unchecksum_ptr(rack, &previous->next);
		if (prev_next != free_ptr) {
			malloc_zone_error(rack->debug_flags, true,
					"tiny_free_list_remove_ptr: Internal invariant broken (next ptr of prev): "
					"ptr=%p, prev_next=%p\n", ptr, prev_next);
			__builtin_unreachable(); // Always crashes in malloc_zone_error().
		}

		// We know free_ptr is already checksummed, so we don't need to do it
		// again.
		previous->next = free_ptr->next;
	}
	if (next) {
		// Check that the previous pointer of "next" points to free_ptr.
		tiny_free_list_t *next_prev = free_list_unchecksum_ptr(rack, &next->previous);
		if (next_prev != free_ptr) {
			malloc_zone_error(rack->debug_flags, true,
					"tiny_free_list_remove_ptr: Internal invariant broken (prev ptr of next): "
					"ptr=%p, next_prev=%p\n", ptr, next_prev);
			__builtin_unreachable(); // Always crashes in malloc_zone_error().
		}

		// We know free_ptr is already checksummed, so we don't need to do it
		// again.
		next->previous = free_ptr->previous;
	}

	tiny_region_t region = TINY_REGION_FOR_PTR(ptr);
	region_free_blocks_t *free_blocks = &region->free_blocks_by_slot[slot];
	uint16_t this_block_index = TINY_INDEX_FOR_PTR(ptr);

	boolean_t is_first = free_blocks->first_block == this_block_index + 1;
	boolean_t is_last = free_blocks->last_block == this_block_index + 1;

	if (is_first && is_last) {
		// Removing the one and only item on the list. Set both block indices to 0.
		free_blocks->first_block = free_blocks->last_block = 0;
	} else if (is_first) {
		MALLOC_ASSERT(next);
		free_blocks->first_block = TINY_INDEX_FOR_PTR(next) + 1;
	} else if (is_last) {
		MALLOC_ASSERT(previous);
		free_blocks->last_block	= TINY_INDEX_FOR_PTR(previous) + 1;
	}
}

void
tiny_finalize_region(rack_t *rack, magazine_t *tiny_mag_ptr)
{
	void *last_block, *previous_block;
	uint32_t *last_header;
	msize_t last_msize, previous_msize, last_index;

	// It is possible that the block prior to the last block in the region has
	// been free'd, but was not coalesced with the free bytes at the end of the
	// block, since we treat the bytes at the end of the region as "in use" in
	// the meta headers. Attempt to coalesce the last block with the previous
	// block, so we don't violate the "no consecutive free blocks" invariant.
	//
	// FIXME: Need to investigate how much work would be required to increase
	//        'mag_bytes_free_at_end' when freeing the preceding block, rather
	//        than performing this workaround.
	//

	if (tiny_mag_ptr->mag_bytes_free_at_end) {
		last_block = (void *)((uintptr_t)TINY_REGION_END(tiny_mag_ptr->mag_last_region) - tiny_mag_ptr->mag_bytes_free_at_end);
		last_msize = TINY_MSIZE_FOR_BYTES(tiny_mag_ptr->mag_bytes_free_at_end);
		last_header = TINY_BLOCK_HEADER_FOR_PTR(last_block);
		last_index = TINY_INDEX_FOR_PTR(last_block);

		// Before anything we transform any remaining mag_bytes_free_at_end into a
		// regular free block.  We take special care here to update the bitfield
		// information, since we are bypassing the normal free codepath.  If there
		// is more than one quanta worth of memory in mag_bytes_free_at_end, then
		// there will be two block headers:
		// 1) header for the free space at end, msize = 1
		// 2) header inserted by set_tiny_meta_header_in_use after block
		// We must clear the second one so that when the free block's size is
		// queried, we do not think the block is only 1 quantum in size because
		// of the second set header bit.
		if (last_index != (NUM_TINY_BLOCKS - 1)) {
			BITARRAY_CLR(last_header, (last_index + 1));
		}

		previous_block = tiny_previous_preceding_free(last_block, &previous_msize);
		if (previous_block) {
			set_tiny_meta_header_middle(last_block);
			tiny_free_list_remove_ptr(rack, tiny_mag_ptr, previous_block, previous_msize);
			last_block = previous_block;
			last_msize += previous_msize;
		}

		// splice last_block into the free list
		tiny_free_list_add_ptr(rack, tiny_mag_ptr, last_block, last_msize);
		tiny_mag_ptr->mag_bytes_free_at_end = 0;
	}

#if CONFIG_ASLR_INTERNAL
	// Coalesce the big free block at start with any following free blocks
	if (tiny_mag_ptr->mag_bytes_free_at_start) {
		last_block = TINY_REGION_ADDRESS(tiny_mag_ptr->mag_last_region);
		last_msize = TINY_MSIZE_FOR_BYTES(tiny_mag_ptr->mag_bytes_free_at_start);

		void *next_block = (void *)((uintptr_t)last_block + tiny_mag_ptr->mag_bytes_free_at_start);

		// clear the in use bit we were using to mark the end of the big start block
		set_tiny_meta_header_middle((void *)((uintptr_t)next_block - TINY_QUANTUM));

		// Coalesce the big start block with any following free blocks
		if (tiny_meta_header_is_free(next_block)) {
			msize_t next_msize = get_tiny_free_size(next_block);
			set_tiny_meta_header_middle(next_block);
			tiny_free_list_remove_ptr(rack, tiny_mag_ptr, next_block, next_msize);
			last_msize += next_msize;
		}

		// splice last_block into the free list
		tiny_free_list_add_ptr(rack, tiny_mag_ptr, last_block, last_msize);
		tiny_mag_ptr->mag_bytes_free_at_start = 0;
	}
#endif

	tiny_mag_ptr->mag_last_region = NULL;
}

int
tiny_free_detach_region(rack_t *rack, magazine_t *tiny_mag_ptr, region_t r)
{
	uintptr_t start = (uintptr_t)TINY_REGION_ADDRESS(r);
	uintptr_t current = start;
	uintptr_t limit = (uintptr_t)TINY_REGION_END(r);
	boolean_t is_free;
	msize_t msize;
	region_trailer_t *trailer = REGION_TRAILER_FOR_TINY_REGION(r);

	while (current < limit) {
		msize = get_tiny_meta_header((void *)current, &is_free);
		if (is_free && !msize && (current == start)) {
			// first block is all free
			break;
		}
		if (!msize) {
#if DEBUG_MALLOC
			malloc_report(ASL_LEVEL_ERR, "*** tiny_free_detach_region error with %p: msize=%d is_free=%d\n", (void *)current, msize, is_free);
#endif
			break;
		}
		if (is_free) {
			tiny_free_list_remove_ptr(rack, tiny_mag_ptr, (void *)current, msize);
		}
		current += TINY_BYTES_FOR_MSIZE(msize);
	}
	return trailer->objects_in_use;
}

size_t
tiny_free_reattach_region(rack_t *rack, magazine_t *tiny_mag_ptr, region_t r)
{
	uintptr_t start = (uintptr_t)TINY_REGION_ADDRESS(r);
	uintptr_t current = start;
	uintptr_t limit = (uintptr_t)TINY_REGION_END(r);
	boolean_t is_free;
	msize_t msize;
	size_t bytes_used = REGION_TRAILER_FOR_TINY_REGION(r)->bytes_used;

	while (current < limit) {
		msize = get_tiny_meta_header((void *)current, &is_free);
		if (is_free && !msize && (current == start)) {
			// first block is all free
			break;
		}
		if (!msize) {
#if DEBUG_MALLOC
			malloc_report(ASL_LEVEL_ERR, "*** tiny_free_reattach_region error with %p: msize=%d is_free=%d\n", (void *)current, msize, is_free);
#endif
			break;
		}
		if (is_free) {
			tiny_free_list_add_ptr(rack, tiny_mag_ptr, (void *)current, msize);
		}
		current += TINY_BYTES_FOR_MSIZE(msize);
	}
	return bytes_used;
}

typedef struct {
	uint8_t pnum, size;
} tiny_pg_pair_t;

void
tiny_free_scan_madvise_free(rack_t *rack, magazine_t *depot_ptr, region_t r)
{
	uintptr_t start = (uintptr_t)TINY_REGION_ADDRESS(r);
	uintptr_t current = start;
	uintptr_t limit = (uintptr_t)TINY_REGION_END(r);
	boolean_t is_free;
	msize_t msize;
	tiny_pg_pair_t advisory[((TINY_REGION_PAYLOAD_BYTES + vm_kernel_page_size - 1) >> vm_kernel_page_shift) >>
							1]; // 256bytes stack allocated
	int advisories = 0;

	// Scan the metadata identifying blocks which span one or more pages. Mark the pages MADV_FREE taking care to preserve free list
	// management data.
	while (current < limit) {
		msize = get_tiny_meta_header((void *)current, &is_free);
		if (is_free && !msize && (current == start)) {
			// first block is all free
#if DEBUG_MALLOC
			malloc_report(ASL_LEVEL_INFO, "*** tiny_free_scan_madvise_free first block is all free! %p: msize=%d is_free=%d\n", (void *)current,
						  msize, is_free);
#endif
			uintptr_t pgLo = round_page_kernel(start + sizeof(tiny_free_list_t) + sizeof(msize_t));
			uintptr_t pgHi = trunc_page_kernel(start + TINY_REGION_SIZE - sizeof(msize_t));

			if (pgLo < pgHi) {
				advisory[advisories].pnum = (pgLo - start) >> vm_kernel_page_shift;
				advisory[advisories].size = (pgHi - pgLo) >> vm_kernel_page_shift;
				advisories++;
			}
			break;
		}
		if (!msize) {
#if DEBUG_MALLOC
			malloc_report(ASL_LEVEL_ERR, "*** tiny_free_scan_madvise_free error with %p: msize=%d is_free=%d\n", (void *)current, msize, is_free);
#endif
			break;
		}
		if (is_free) {
			uintptr_t pgLo = round_page_kernel(current + sizeof(tiny_free_list_t) + sizeof(msize_t));
			uintptr_t pgHi = trunc_page_kernel(current + TINY_BYTES_FOR_MSIZE(msize) - sizeof(msize_t));

			if (pgLo < pgHi) {
				advisory[advisories].pnum = (pgLo - start) >> vm_kernel_page_shift;
				advisory[advisories].size = (pgHi - pgLo) >> vm_kernel_page_shift;
				advisories++;
			}
		}
		current += TINY_BYTES_FOR_MSIZE(msize);
	}

	if (advisories > 0) {
		int i;

		// So long as the following hold for this region:
		// (1) No malloc()'s are ever performed from the depot (hence free pages remain free,)
		// (2) The region is not handed over to a per-CPU magazine (where malloc()'s could be performed),
		// (3) The entire region is not mumap()'d (so the madvise's are applied to the intended addresses),
		// then the madvise opportunities collected just above can be applied outside all locks.
		// (1) is ensured by design, (2) and (3) are ensured by bumping the globally visible counter node->pinned_to_depot.

		OSAtomicIncrement32Barrier(&(REGION_TRAILER_FOR_TINY_REGION(r)->pinned_to_depot));
		SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);
		for (i = 0; i < advisories; ++i) {
			uintptr_t addr = (advisory[i].pnum << vm_kernel_page_shift) + start;
			size_t size = advisory[i].size << vm_kernel_page_shift;

			mvm_madvise_free(rack, r, addr, addr + size, NULL, rack->debug_flags & MALLOC_DO_SCRIBBLE);
		}
		SZONE_MAGAZINE_PTR_LOCK(depot_ptr);
		OSAtomicDecrement32Barrier(&(REGION_TRAILER_FOR_TINY_REGION(r)->pinned_to_depot));
	}
}

#if CONFIG_RECIRC_DEPOT
static region_t
tiny_find_msize_region(rack_t *rack, magazine_t *tiny_mag_ptr, mag_index_t mag_index, msize_t msize)
{
	tiny_free_list_t *ptr;
	grain_t slot = tiny_slot_from_msize(msize);
	free_list_t *free_list = tiny_mag_ptr->mag_free_list;
	free_list_t *the_slot = free_list + slot;
	free_list_t *limit;
#if defined(__LP64__)
	uint64_t bitmap;
#else
	uint32_t bitmap;
#endif
	// Assumes we've locked the magazine
	CHECK_MAGAZINE_PTR_LOCKED(szone, tiny_mag_ptr, __PRETTY_FUNCTION__);

	// Look for an exact match by checking the freelist for this msize.
	ptr = the_slot->p;
	if (ptr) {
		return TINY_REGION_FOR_PTR(ptr);
	}

	// Mask off the bits representing slots holding free blocks smaller than the
	// size we need.  If there are no larger free blocks, try allocating from
	// the free space at the end of the tiny region.
#if defined(__LP64__)
	bitmap = ((uint64_t *)(tiny_mag_ptr->mag_bitmap))[0] & ~((1ULL << slot) - 1);
#else
	bitmap = tiny_mag_ptr->mag_bitmap[0] & ~((1 << slot) - 1);
#endif
	if (!bitmap) {
		return NULL;
	}

	slot = BITMAPV_CTZ(bitmap);
	limit = free_list + NUM_TINY_SLOTS;
	free_list += slot;

	if (free_list < limit) {
		ptr = free_list->p;
		if (ptr) {
			return TINY_REGION_FOR_PTR(ptr);
		} else {
			/* Shouldn't happen. Fall through to look at last slot. */
#if DEBUG_MALLOC
			malloc_report(ASL_LEVEL_ERR, "in tiny_find_msize_region(), mag_bitmap out of sync, slot=%d\n", slot);
#endif
		}
	}

	// We are now looking at the last slot, which contains blocks equal to, or
	// due to coalescing of free blocks, larger than NUM_TINY_SLOTS * tiny quantum size.
	ptr = limit->p;
	if (ptr) {
		return TINY_REGION_FOR_PTR(ptr);
	}

	return NULL;
}
#endif // CONFIG_RECIRC_DEPOT

#if CONFIG_MADVISE_PRESSURE_RELIEF
void
tiny_madvise_pressure_relief(rack_t *rack)
{
	mag_index_t mag_index;
	magazine_t *tiny_depot_ptr = (&rack->magazines[DEPOT_MAGAZINE_INDEX]);

	for (mag_index = 0; mag_index < rack->num_magazines; mag_index++) {
		size_t index;
		for (index = 0; index < rack->region_generation->num_regions_allocated; ++index) {
			SZONE_LOCK(TINY_SZONE_FROM_RACK(rack));

			region_t tiny = rack->region_generation->hashed_regions[index];
			if (!tiny || tiny == HASHRING_REGION_DEALLOCATED) {
				SZONE_UNLOCK(TINY_SZONE_FROM_RACK(rack));
				continue;
			}

			magazine_t *mag_ptr = mag_lock_zine_for_region_trailer(rack->magazines,
					REGION_TRAILER_FOR_TINY_REGION(tiny),
					MAGAZINE_INDEX_FOR_TINY_REGION(tiny));
			SZONE_UNLOCK(TINY_SZONE_FROM_RACK(rack));

			/* Ordering is important here, the magazine of a region may potentially change
			 * during mag_lock_zine_for_region_trailer, so src_mag_index must be taken
			 * after we've obtained the lock.
			 */
			mag_index_t src_mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(tiny);

			/* We can (and must) ignore magazines that are already in the recirc depot. */
			if (src_mag_index == DEPOT_MAGAZINE_INDEX) {
				SZONE_MAGAZINE_PTR_UNLOCK(mag_ptr);
				continue;
			}

			if (tiny == mag_ptr->mag_last_region && (mag_ptr->mag_bytes_free_at_end || mag_ptr->mag_bytes_free_at_start)) {
				tiny_finalize_region(rack, mag_ptr);
			}

			/* Because this region is currently in use, we can't safely madvise it while
			 * it's attached to the magazine. For this operation we have to remove it from
			 * the current mag, attach it to the depot and then madvise.
			 */

			recirc_list_extract(rack, mag_ptr, REGION_TRAILER_FOR_TINY_REGION(tiny));
			int objects_in_use = tiny_free_detach_region(rack, mag_ptr, tiny);

			SZONE_MAGAZINE_PTR_LOCK(tiny_depot_ptr);
			MAGAZINE_INDEX_FOR_TINY_REGION(tiny) = DEPOT_MAGAZINE_INDEX;
			REGION_TRAILER_FOR_TINY_REGION(tiny)->pinned_to_depot = 0;

			size_t bytes_inplay = tiny_free_reattach_region(rack, tiny_depot_ptr, tiny);

			/* Fix up the metadata of the target magazine while the region is in the depot. */
			mag_ptr->mag_num_bytes_in_objects -= bytes_inplay;
			mag_ptr->num_bytes_in_magazine -= TINY_REGION_PAYLOAD_BYTES;
			mag_ptr->mag_num_objects -= objects_in_use;

			/* Now we can drop the magazine lock of the source mag. */
			SZONE_MAGAZINE_PTR_UNLOCK(mag_ptr);

			tiny_depot_ptr->mag_num_bytes_in_objects += bytes_inplay;
			tiny_depot_ptr->num_bytes_in_magazine += TINY_REGION_PAYLOAD_BYTES;
			tiny_depot_ptr->mag_num_objects -= objects_in_use;

			recirc_list_splice_last(rack, tiny_depot_ptr, REGION_TRAILER_FOR_TINY_REGION(tiny));

			/* Actually do the scan, done holding the depot lock, the call will drop the lock
			 * around the actual madvise syscalls.
			 */
			tiny_free_scan_madvise_free(rack, tiny_depot_ptr, tiny);

			/* Now the region is in the recirc depot, the next allocations to require more
			 * blocks will come along and take one of these regions back out of the depot.
			 * As OS X madvise's reuse on an per-region basis, we leave as many of these
			 * regions in the depot as possible after memory pressure.
			 */
			SZONE_MAGAZINE_PTR_UNLOCK(tiny_depot_ptr);
		}
	}
}
#endif // CONFIG_MADVISE_PRESSURE_RELIEF

static MALLOC_INLINE void
tiny_madvise_free_range_no_lock(rack_t *rack,
								magazine_t *tiny_mag_ptr,
								region_t region,
								void *headptr,
								size_t headsize,
								void *ptr,
								msize_t msize)
{
	region_trailer_t *node = REGION_TRAILER_FOR_TINY_REGION(region);

	// Lock on tiny_magazines[mag_index] is already held here
	// Calculate the first page in the coalesced block that would be safe to mark MADV_FREE
	size_t free_header_size = sizeof(tiny_free_list_t) + sizeof(msize_t);
	uintptr_t safe_ptr = (uintptr_t)ptr + free_header_size;
	uintptr_t round_safe = round_page_kernel(safe_ptr);

	// Calculate the last page in the coalesced block that would be safe to mark MADV_FREE
	size_t free_tail_size = sizeof(msize_t);
	uintptr_t safe_extent = (uintptr_t)ptr + TINY_BYTES_FOR_MSIZE(msize) - free_tail_size;
	uintptr_t trunc_extent = trunc_page_kernel(safe_extent);

	// The newly freed block may complete a span of bytes that cover a page. Mark it with MADV_FREE.
	if (round_safe < trunc_extent) { // Coalesced area covers a page (perhaps many)
		// Extend the freed block by the free region header and tail sizes to include pages
		// we may have coalesced that no longer host free region tails and headers.
		// This may extend over in-use ranges, but the MIN/MAX clamping below will fix that up.
		uintptr_t lo = trunc_page_kernel((uintptr_t)headptr - free_tail_size);
		uintptr_t hi = round_page_kernel((uintptr_t)headptr + headsize + free_header_size);

		uintptr_t free_lo = MAX(round_safe, lo);
		uintptr_t free_hi = MIN(trunc_extent, hi);

		if (free_lo < free_hi) {
			tiny_free_list_remove_ptr(rack, tiny_mag_ptr, ptr, msize);
			set_tiny_meta_header_in_use(ptr, msize);

			OSAtomicIncrement32Barrier(&(node->pinned_to_depot));
			SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
			mvm_madvise_free(rack, region, free_lo, free_hi, &rack->last_madvise, rack->debug_flags & MALLOC_DO_SCRIBBLE);
			SZONE_MAGAZINE_PTR_LOCK(tiny_mag_ptr);
			OSAtomicDecrement32Barrier(&(node->pinned_to_depot));

			set_tiny_meta_header_free(ptr, msize);
			tiny_free_list_add_ptr(rack, tiny_mag_ptr, ptr, msize);
		}
	}
}

#if CONFIG_RECIRC_DEPOT
static boolean_t
tiny_get_region_from_depot(rack_t *rack, magazine_t *tiny_mag_ptr, mag_index_t mag_index, msize_t msize)
{
	magazine_t *depot_ptr = &(rack->magazines[DEPOT_MAGAZINE_INDEX]);

	/* FIXME: Would Uniprocessor benefit from recirc and MADV_FREE? */
	if (rack->num_magazines == 1) { // Uniprocessor, single magazine, so no recirculation necessary
		return 0;
	}

#if DEBUG_MALLOC
	if (DEPOT_MAGAZINE_INDEX == mag_index) {
		malloc_zone_error(rack->debug_flags, true, "tiny_get_region_from_depot called for magazine index -1\n");
		return 0;
	}
#endif

	SZONE_MAGAZINE_PTR_LOCK(depot_ptr);

	// Appropriate a Depot'd region that can satisfy requested msize.
	region_trailer_t *node;
	region_t sparse_region;

	while (1) {
		sparse_region = tiny_find_msize_region(rack, depot_ptr, DEPOT_MAGAZINE_INDEX, msize);
		if (NULL == sparse_region) { // Depot empty?
			SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);
			return 0;
		}

		node = REGION_TRAILER_FOR_TINY_REGION(sparse_region);
		if (0 >= node->pinned_to_depot) {
			break;
		}

		SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);
		yield();
		SZONE_MAGAZINE_PTR_LOCK(depot_ptr);
	}

	// disconnect node from Depot
	recirc_list_extract(rack, depot_ptr, node);

	// Iterate the region pulling its free entries off the (locked) Depot's free list
	int objects_in_use = tiny_free_detach_region(rack, depot_ptr, sparse_region);

	// Transfer ownership of the region
	MAGAZINE_INDEX_FOR_TINY_REGION(sparse_region) = mag_index;
	node->pinned_to_depot = 0;

	// Iterate the region putting its free entries on its new (locked) magazine's free list
	size_t bytes_inplay = tiny_free_reattach_region(rack, tiny_mag_ptr, sparse_region);

	depot_ptr->mag_num_bytes_in_objects -= bytes_inplay;
	depot_ptr->num_bytes_in_magazine -= TINY_REGION_PAYLOAD_BYTES;
	depot_ptr->mag_num_objects -= objects_in_use;

	tiny_mag_ptr->mag_num_bytes_in_objects += bytes_inplay;
	tiny_mag_ptr->num_bytes_in_magazine += TINY_REGION_PAYLOAD_BYTES;
	tiny_mag_ptr->mag_num_objects += objects_in_use;

	// connect to magazine as last node
	recirc_list_splice_last(rack, tiny_mag_ptr, node);

	SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);

	// DTrace USDT Probe
	MAGMALLOC_DEPOTREGION(TINY_SZONE_FROM_RACK(rack), (int)mag_index, (void *)sparse_region,
			TINY_REGION_SIZE, (int)BYTES_USED_FOR_TINY_REGION(sparse_region));

	return 1;
}

static region_t
tiny_free_try_depot_unmap_no_lock(rack_t *rack, magazine_t *depot_ptr, region_trailer_t *node)
{
	if (0 < node->bytes_used || 0 < node->pinned_to_depot ||
		depot_ptr->recirculation_entries < recirc_retained_regions)
	{
		return NULL;
	}

	// disconnect node from Depot
	recirc_list_extract(rack, depot_ptr, node);

	// Iterate the region pulling its free entries off the (locked) Depot's free list
	region_t sparse_region = TINY_REGION_FOR_PTR(node);
	int objects_in_use = tiny_free_detach_region(rack, depot_ptr, sparse_region);

	if (0 == objects_in_use) {
		// Invalidate the hash table entry for this region with HASHRING_REGION_DEALLOCATED.
		// Using HASHRING_REGION_DEALLOCATED preserves the collision chain, using HASHRING_OPEN_ENTRY (0) would not.
		rgnhdl_t pSlot = hash_lookup_region_no_lock(rack->region_generation->hashed_regions,
				rack->region_generation->num_regions_allocated,
				rack->region_generation->num_regions_allocated_shift,
				sparse_region);

		if (NULL == pSlot) {
			malloc_zone_error(rack->debug_flags, true, "tiny_free_try_depot_unmap_no_lock hash lookup failed: %p\n", sparse_region);
			return NULL;
		}
		*pSlot = HASHRING_REGION_DEALLOCATED;
		depot_ptr->num_bytes_in_magazine -= TINY_REGION_PAYLOAD_BYTES;

		// Atomically increment num_regions_dealloc
#ifdef __LP64___
		OSAtomicIncrement64(&rack->num_regions_dealloc);
#else
		OSAtomicIncrement32((int32_t *)&rack->num_regions_dealloc);
#endif

		// Caller will transfer ownership of the region back to the OS with no locks held
		MAGMALLOC_DEALLOCREGION(TINY_SZONE_FROM_RACK(rack), (void *)sparse_region, TINY_REGION_SIZE); // DTrace USDT Probe
		return sparse_region;
	} else {
		malloc_zone_error(rack->debug_flags, true, "tiny_free_try_depot_unmap_no_lock objects_in_use not zero: %d\n", objects_in_use);
		return NULL;
	}
}

static boolean_t
tiny_free_do_recirc_to_depot(rack_t *rack, magazine_t *tiny_mag_ptr, mag_index_t mag_index)
{
	// The entire magazine crossed the "emptiness threshold". Transfer a region
	// from this magazine to the Depot. Choose a region that itself has crossed the emptiness threshold (i.e
	// is at least fraction "f" empty.) Such a region will be marked "suitable" on the recirculation list.
	// Start from the last node in order to keep the regions created earlier at the front of the list for
	// as long as possible. This helps reduce fragmentation.
	region_trailer_t *node = tiny_mag_ptr->lastNode;

	while (node && (!node->recirc_suitable || node->pinned_to_depot)) {
		// If we skip a node due to pinned_to_depot being non-zero, it must be
		// because another thread is madvising the same region in
		// tiny_madvise_free_range_no_lock(), called from tiny_free_no_lock().
		// When that's done, the same thread will enter tiny_free_try_recirc_to_depot()
		// for the same region, which will come back here. So this just defers
		// recirculation of the region.
		node = node->prev;
	}

	if (!node) {
#if DEBUG_MALLOC
		malloc_report(ASL_LEVEL_INFO, "*** tiny_free_do_recirc_to_depot end of list\n");
#endif
		return TRUE; // Caller must SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
	}

	region_t sparse_region = TINY_REGION_FOR_PTR(node);

	// Deal with unclaimed memory -- mag_bytes_free_at_end or mag_bytes_free_at_start
	if (sparse_region == tiny_mag_ptr->mag_last_region &&
		(tiny_mag_ptr->mag_bytes_free_at_end || tiny_mag_ptr->mag_bytes_free_at_start)) {
		tiny_finalize_region(rack, tiny_mag_ptr);
	}

	// disconnect "suitable" node from magazine
	recirc_list_extract(rack, tiny_mag_ptr, node);

	// Iterate the region pulling its free entries off its (locked) magazine's free list
	int objects_in_use = tiny_free_detach_region(rack, tiny_mag_ptr, sparse_region);
	magazine_t *depot_ptr = &(rack->magazines[DEPOT_MAGAZINE_INDEX]);

	// hand over the region to the (locked) Depot
	SZONE_MAGAZINE_PTR_LOCK(depot_ptr);
	// this will cause tiny_free_list_add_ptr called by tiny_free_reattach_region to use
	// the depot as its target magazine, rather than magazine formerly associated with sparse_region
	MAGAZINE_INDEX_FOR_TINY_REGION(sparse_region) = DEPOT_MAGAZINE_INDEX;
	node->pinned_to_depot = 0;

	// Iterate the region putting its free entries on Depot's free list
	size_t bytes_inplay = tiny_free_reattach_region(rack, depot_ptr, sparse_region);

	tiny_mag_ptr->mag_num_bytes_in_objects -= bytes_inplay;
	tiny_mag_ptr->num_bytes_in_magazine -= TINY_REGION_PAYLOAD_BYTES;
	tiny_mag_ptr->mag_num_objects -= objects_in_use;

	SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr); // Unlock the originating magazine

	depot_ptr->mag_num_bytes_in_objects += bytes_inplay;
	depot_ptr->num_bytes_in_magazine += TINY_REGION_PAYLOAD_BYTES;
	depot_ptr->mag_num_objects += objects_in_use;

	// connect to Depot as last node
	recirc_list_splice_last(rack, depot_ptr, node);

	MAGMALLOC_RECIRCREGION(TINY_SZONE_FROM_RACK(rack), (int)mag_index, (void *)sparse_region, TINY_REGION_SIZE,
						   (int)BYTES_USED_FOR_TINY_REGION(sparse_region)); // DTrace USDT Probe

#if !CONFIG_AGGRESSIVE_MADVISE
	// Mark free'd dirty pages with MADV_FREE to reduce memory pressure
	tiny_free_scan_madvise_free(rack, depot_ptr, sparse_region);
#endif

	// If the region is entirely empty vm_deallocate() it outside the depot lock
	region_t r_dealloc = tiny_free_try_depot_unmap_no_lock(rack, depot_ptr, node);
	SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);
	if (r_dealloc) {
		mvm_deallocate_pages(r_dealloc, TINY_REGION_SIZE, 0);
	}
	return FALSE; // Caller need not unlock the originating magazine
}

static MALLOC_INLINE boolean_t
tiny_free_try_recirc_to_depot(rack_t *rack,
							  magazine_t *tiny_mag_ptr,
							  mag_index_t mag_index,
							  region_t region,
							  void *headptr,
							  size_t headsize,
							  void *ptr,
							  msize_t msize)
{
	region_trailer_t *node = REGION_TRAILER_FOR_TINY_REGION(region);
	size_t bytes_used = node->bytes_used;

	/* FIXME: Would Uniprocessor benefit from recirc and MADV_FREE? */
	if (rack->num_magazines == 1) { // Uniprocessor, single magazine, so no recirculation necessary
		/* NOTHING */
		return TRUE; // Caller must do SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr)
	} else if (DEPOT_MAGAZINE_INDEX != mag_index) {
		// Emptiness discriminant
		if (tiny_region_below_recirc_threshold(region)) {
			/* Region has crossed threshold from density to sparsity. Mark it "suitable" on the
			 * recirculation candidates list. */
			node->recirc_suitable = TRUE;
		} else {
			/* After this free, we've found the region is still dense, so it must have been even more so before
			 * the free. That implies the region is already correctly marked. Do nothing. */
		}

		// Has the entire magazine crossed the "emptiness threshold"? If so, transfer a region
		// from this magazine to the Depot. Choose a region that itself has crossed the emptiness threshold (i.e
		// is at least fraction "f" empty.) Such a region will be marked "suitable" on the recirculation list.
		if (tiny_magazine_below_recirc_threshold(tiny_mag_ptr)) {
			return tiny_free_do_recirc_to_depot(rack, tiny_mag_ptr, mag_index);
		}
	} else {
#if !CONFIG_AGGRESSIVE_MADVISE
		// We are free'ing into the depot, so madvise as we do so unless we were madvising every incoming
		// allocation anyway.
		tiny_madvise_free_range_no_lock(rack, tiny_mag_ptr, region, headptr, headsize, ptr, msize);
#endif

		if (0 < bytes_used || 0 < node->pinned_to_depot) {
			/* Depot'd region is still live. Leave it in place on the Depot's recirculation list
			 * so as to avoid thrashing between the Depot's free list and a magazines's free list
			 * with detach_region/reattach_region */
		} else {
			/* Depot'd region is just now empty. Consider return to OS. */
			region_t r_dealloc = tiny_free_try_depot_unmap_no_lock(rack, tiny_mag_ptr, node);
			SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
			if (r_dealloc) {
				mvm_deallocate_pages(r_dealloc, TINY_REGION_SIZE, 0);
			}
			return FALSE; // Caller need not unlock
		}
	}
	return TRUE; // Caller must do SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr)
}
#endif // CONFIG_RECIRC_DEPOT

boolean_t
tiny_free_no_lock(rack_t *rack, magazine_t *tiny_mag_ptr, mag_index_t mag_index, region_t region, void *ptr, msize_t msize, boolean_t partial_free)
{
	void *original_ptr = ptr;
	size_t original_size = TINY_BYTES_FOR_MSIZE(msize);
	void *next_block = ((unsigned char *)ptr + original_size);
	msize_t previous_msize, next_msize;
	void *previous;
	tiny_free_list_t *big_free_block;
	tiny_free_list_t *after_next_block;
	tiny_free_list_t *before_next_block;

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "in tiny_free_no_lock(), ptr=%p, msize=%d\n", ptr, msize);
	}
	if (!msize) {
		malloc_zone_error(rack->debug_flags, true,
				"trying to free tiny block that is too small in tiny_free_no_lock(), ptr=%p, msize=%d\n",
				ptr, msize);
	}
#endif

	// Check that the region cookie is intact.
	region_trailer_t *trailer = REGION_TRAILER_FOR_TINY_REGION(region);
	region_check_cookie(region, trailer);

	// We try to coalesce this block with the preceeding one
	previous = tiny_previous_preceding_free(ptr, &previous_msize);
	if (previous) {
#if DEBUG_MALLOC
		if (LOG(szone, ptr) || LOG(szone, previous)) {
			malloc_report(ASL_LEVEL_INFO, "in tiny_free_no_lock(), coalesced backwards for %p previous=%p\n", ptr, previous);
		}
#endif

		// clear the meta_header since this is no longer the start of a block
		set_tiny_meta_header_middle(ptr);
		tiny_free_list_remove_ptr(rack, tiny_mag_ptr, previous, previous_msize);
		ptr = previous;
		msize += previous_msize;
	}
	// We try to coalesce with the next block
	if ((next_block < TINY_REGION_END(region)) && tiny_meta_header_is_free(next_block)) {
		next_msize = get_tiny_free_size(next_block);
#if DEBUG_MALLOC
		if (LOG(szone, ptr) || LOG(szone, next_block)) {
			malloc_report(ASL_LEVEL_INFO, "in tiny_free_no_lock(), for ptr=%p, msize=%d coalesced forward=%p next_msize=%d\n", ptr, msize,
						  next_block, next_msize);
		}
#endif
		// If we are coalescing with the next block, and the next block is in
		// the last slot of the free list, then we optimize this case here to
		// avoid removing next_block from the slot NUM_TINY_SLOTS and then adding ptr back
		// to slot NUM_TINY_SLOTS.
		if (next_msize > NUM_TINY_SLOTS) {
			msize += next_msize;

			big_free_block = (tiny_free_list_t *)next_block;
			after_next_block = free_list_unchecksum_ptr(rack, &big_free_block->next);
			before_next_block = free_list_unchecksum_ptr(rack, &big_free_block->previous);

			if (!before_next_block) {
				tiny_mag_ptr->mag_free_list[NUM_TINY_SLOTS].p = ptr;
			} else {
				before_next_block->next.u = free_list_checksum_ptr(rack, ptr);
			}

			if (after_next_block) {
				after_next_block->previous.u = free_list_checksum_ptr(rack, ptr);
			}

			// we don't need to checksum these since they are already checksummed
			((tiny_free_list_t *)ptr)->previous = big_free_block->previous;
			((tiny_free_list_t *)ptr)->next = big_free_block->next;

			// clear the meta_header to enable coalescing backwards
			set_tiny_meta_header_middle(big_free_block);
			set_tiny_meta_header_free(ptr, msize);

			uint16_t next_block_index = TINY_INDEX_FOR_PTR(big_free_block) + 1;
			uint16_t ptr_index = TINY_INDEX_FOR_PTR(ptr) + 1;
			const grain_t slot = NUM_TINY_SLOTS;
			region_free_blocks_t *free_blocks = &((tiny_region_t)region)->free_blocks_by_slot[slot];
			if (free_blocks->first_block == next_block_index) {
				free_blocks->first_block = ptr_index;
			}
			if (free_blocks->last_block == next_block_index) {
				free_blocks->last_block = ptr_index;
			}
			goto tiny_free_ending;
		}
		tiny_free_list_remove_ptr(rack, tiny_mag_ptr, next_block, next_msize);
		set_tiny_meta_header_middle(next_block); // clear the meta_header to enable coalescing backwards
		msize += next_msize;
	}

	// The tiny cache already scribbles free blocks as they go through the
	// cache whenever msize < TINY_QUANTUM , so we do not need to do it here.
	if ((rack->debug_flags & MALLOC_DO_SCRIBBLE) && msize && (msize >= TINY_QUANTUM)) {
		memset(ptr, SCRABBLE_BYTE, TINY_BYTES_FOR_MSIZE(msize));
	}

	tiny_free_list_add_ptr(rack, tiny_mag_ptr, ptr, msize);

tiny_free_ending:
	// we use original_size and not msize to avoid double counting the coalesced blocks
	tiny_mag_ptr->mag_num_bytes_in_objects -= original_size;

	// Update this region's bytes in use count
	size_t bytes_used = trailer->bytes_used - original_size;
	trailer->bytes_used = (unsigned int)bytes_used;

	// Partial free accounts for the case where we allocate a block for
	// posix_memalign and then free some range of bytes at the start and/or
	// the end. In that case, we aren't changing the number of allocated objects.
	// Similarly for realloc() in the case where we shrink in place.
	if (!partial_free) {
		trailer->objects_in_use--;
		tiny_mag_ptr->mag_num_objects--;
	}
#if CONFIG_AGGRESSIVE_MADVISE
	// Platforms that want to madvise every freed allocation do so here, even if we continue
	// on to use the recirc depot after.
	tiny_madvise_free_range_no_lock(rack, tiny_mag_ptr, region, original_ptr, original_size, ptr, msize);
#endif

	// Caller must do SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr) if this function
	// returns TRUE.
	boolean_t needs_unlock = TRUE;

#if CONFIG_RECIRC_DEPOT
	needs_unlock = tiny_free_try_recirc_to_depot(rack, tiny_mag_ptr, mag_index, region, original_ptr, original_size, ptr, msize);
#endif // CONFIG_RECIRC_DEPOT
	return needs_unlock;
}

// Allocates from the last region or a freshly allocated region
static void *
tiny_malloc_from_region_no_lock(rack_t *rack,
								magazine_t *tiny_mag_ptr,
								mag_index_t mag_index,
								msize_t msize,
								void *aligned_address)
{
	void *ptr;

	// Deal with unclaimed memory -- mag_bytes_free_at_end or mag_bytes_free_at_start
	if (tiny_mag_ptr->mag_bytes_free_at_end || tiny_mag_ptr->mag_bytes_free_at_start) {
		tiny_finalize_region(rack, tiny_mag_ptr);
	}

	// We set the unused bits of the header in the last pair to be all ones, and those of the inuse to zeroes.
#if NUM_TINY_BLOCKS & 31
	const uint32_t header = 0xFFFFFFFFU << (NUM_TINY_BLOCKS & 31);
#else
	const uint32_t header = 0;
#endif
	((tiny_region_t)aligned_address)->pairs[CEIL_NUM_TINY_BLOCKS_WORDS - 1].header = header;
	((tiny_region_t)aligned_address)->pairs[CEIL_NUM_TINY_BLOCKS_WORDS - 1].inuse = 0;

	// Tag the region at "aligned_address" as belonging to us,
	// and so put it under the protection of the magazine lock we are holding.
	// Do this before advertising "aligned_address" on the hash ring(!)
	MAGAZINE_INDEX_FOR_TINY_REGION(aligned_address) = mag_index;

	// Insert the new region into the hash ring
	rack_region_insert(rack, (region_t)aligned_address);

	tiny_mag_ptr->mag_last_region = aligned_address;
	BYTES_USED_FOR_TINY_REGION(aligned_address) = TINY_BYTES_FOR_MSIZE(msize);
	OBJECTS_IN_USE_FOR_TINY_REGION(aligned_address) = 1;

#if CONFIG_ASLR_INTERNAL
	int offset_msize = malloc_entropy[0] & TINY_ENTROPY_MASK;
#if DEBUG_MALLOC
	if (getenv("MallocASLRForce")) {
		offset_msize = strtol(getenv("MallocASLRForce"), NULL, 0) & TINY_ENTROPY_MASK;
	}
	if (getenv("MallocASLRPrint")) {
		malloc_report(ASL_LEVEL_INFO, "Region: %p offset: %d\n", aligned_address, offset_msize);
	}
#endif
#else
	int offset_msize = 0;
#endif
	ptr = (void *)((uintptr_t)aligned_address + TINY_BYTES_FOR_MSIZE(offset_msize));
	set_tiny_meta_header_in_use(ptr, msize);
	tiny_mag_ptr->mag_num_objects++;
	tiny_mag_ptr->mag_num_bytes_in_objects += TINY_BYTES_FOR_MSIZE(msize);
	tiny_mag_ptr->num_bytes_in_magazine += TINY_REGION_PAYLOAD_BYTES;

	// We put a header on the last block so that it appears in use (for coalescing, etc...)
	set_tiny_meta_header_in_use_1((void *)((uintptr_t)ptr + TINY_BYTES_FOR_MSIZE(msize)));
	tiny_mag_ptr->mag_bytes_free_at_end = TINY_BYTES_FOR_MSIZE(NUM_TINY_BLOCKS - msize - offset_msize);

#if CONFIG_ASLR_INTERNAL
	// Put a header on the previous block for same reason
	tiny_mag_ptr->mag_bytes_free_at_start = TINY_BYTES_FOR_MSIZE(offset_msize);
	if (offset_msize) {
		set_tiny_meta_header_in_use_1((void *)((uintptr_t)ptr - TINY_QUANTUM));
	}
#else
	tiny_mag_ptr->mag_bytes_free_at_start = 0;
#endif

	// connect to magazine as last node
	recirc_list_splice_last(rack, tiny_mag_ptr, REGION_TRAILER_FOR_TINY_REGION(aligned_address));

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "in tiny_malloc_from_region_no_lock(), ptr=%p, msize=%d\n", ptr, msize);
	}
#endif
	return ptr;
}

void *
tiny_memalign(szone_t *szone, size_t alignment, size_t size, size_t span)
{
	msize_t mspan = TINY_MSIZE_FOR_BYTES(span + TINY_QUANTUM - 1);
	void *p = tiny_malloc_should_clear(&szone->tiny_rack, mspan, 0);

	if (NULL == p) {
		return NULL;
	}

	size_t offset = ((uintptr_t)p) & (alignment - 1);	// p % alignment
	size_t pad = (0 == offset) ? 0 : alignment - offset; // p + pad achieves desired alignment

	msize_t msize = TINY_MSIZE_FOR_BYTES(size + TINY_QUANTUM - 1);
	msize_t mpad = TINY_MSIZE_FOR_BYTES(pad + TINY_QUANTUM - 1);
	msize_t mwaste = mspan - msize - mpad; // excess blocks

	if (mpad > 0) {
		void *q = (void *)(((uintptr_t)p) + pad);

		// Mark q as a block header and in-use, thus creating two blocks.
		magazine_t *tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone->tiny_rack.magazines,
				REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(p)),
				MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(p)));
		set_tiny_meta_header_in_use(q, msize);
		tiny_mag_ptr->mag_num_objects++;

		// set_tiny_meta_header_in_use() "reaffirms" the block_header on the *following* block, so
		// now set its in_use bit as well. But only if its within the original allocation made above.
		if (mwaste > 0) {
			BITARRAY_SET(TINY_INUSE_FOR_HEADER(TINY_BLOCK_HEADER_FOR_PTR(q)), TINY_INDEX_FOR_PTR(q) + msize);
		}
		SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);

		// Give up mpad blocks beginning at p to the tiny free list
		free_tiny(&szone->tiny_rack, p, TINY_REGION_FOR_PTR(p), TINY_BYTES_FOR_MSIZE(mpad), true);

		p = q; // advance p to the desired alignment
	}

	if (mwaste > 0) {
		void *q = (void *)(((uintptr_t)p) + TINY_BYTES_FOR_MSIZE(msize));
		// Mark q as block header and in-use, thus creating two blocks.
		magazine_t *tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone->tiny_rack.magazines,
				REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(p)),
				MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(p)));
		set_tiny_meta_header_in_use(q, mwaste);
		tiny_mag_ptr->mag_num_objects++;
		SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);

		// Give up mwaste blocks beginning at q to the tiny free list
		free_tiny(&szone->tiny_rack, q, TINY_REGION_FOR_PTR(q), TINY_BYTES_FOR_MSIZE(mwaste), true);
	}

	return p; // p has the desired size and alignment, and can later be free()'d
}

boolean_t
tiny_claimed_address(rack_t *rack, void *ptr)
{
	region_t r = tiny_region_for_ptr_no_lock(rack, ptr);
	return r && ptr < TINY_REGION_END(r);
}

void *
tiny_try_shrink_in_place(rack_t *rack, void *ptr, size_t old_size, size_t new_good_size)
{
	msize_t new_msize = TINY_MSIZE_FOR_BYTES(new_good_size);
	msize_t mshrinkage = TINY_MSIZE_FOR_BYTES(old_size) - new_msize;

	if (mshrinkage) {
		void *q = (void *)((uintptr_t)ptr + TINY_BYTES_FOR_MSIZE(new_msize));
		magazine_t *tiny_mag_ptr = mag_lock_zine_for_region_trailer(rack->magazines,
				REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr)),
				MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr)));

		// Mark q as block header and in-use, thus creating two blocks.
		set_tiny_meta_header_in_use(q, mshrinkage);
		tiny_mag_ptr->mag_num_objects++;

		SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
		free_tiny(rack, q, TINY_REGION_FOR_PTR(q), 0, true);
	}
	return ptr;
}

boolean_t
tiny_try_realloc_in_place(rack_t *rack, void *ptr, size_t old_size, size_t new_size)
{
	// returns 1 on success
	msize_t index;
	msize_t old_msize;
	unsigned next_index;
	void *next_block;
	boolean_t is_free;
	msize_t next_msize, coalesced_msize, leftover_msize, new_msize;
	void *leftover;
	region_trailer_t *trailer = REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr));

	index = TINY_INDEX_FOR_PTR(ptr);
	old_msize = TINY_MSIZE_FOR_BYTES(old_size);
	new_msize = TINY_MSIZE_FOR_BYTES(new_size + TINY_QUANTUM - 1);
	next_index = index + old_msize;

	if (next_index >= NUM_TINY_BLOCKS) {
		return 0;
	}
	next_block = (char *)ptr + old_size;

	magazine_t *tiny_mag_ptr = mag_lock_zine_for_region_trailer(rack->magazines,
			trailer,
			MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr)));

	if (DEPOT_MAGAZINE_INDEX == MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr))) {
		SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
		return 0;
	}

	coalesced_msize = new_msize - old_msize;
#if CONFIG_TINY_CACHE
	void *last_free_ptr = tiny_mag_ptr->mag_last_free;
	msize_t last_free_msize = tiny_mag_ptr->mag_last_free_msize;
	if (last_free_ptr == next_block && old_msize + last_free_msize >= new_msize) {
		/*
		 * There is a block in mag_last_free and it's immediately after
		 * this block and it's large enough. We can use some or all of it.
		 */
		leftover_msize = last_free_msize - coalesced_msize;
		if (leftover_msize) {
			tiny_mag_ptr->mag_last_free_msize -= coalesced_msize;
			tiny_mag_ptr->mag_last_free += new_size - old_size;
			// The block in mag_last_free is still marked as header and in-use, so copy that
			// state to the block that remains. The state for the block that we're going to
			// use is adjusted by the set_tiny_meta_header_middle() call below.
			set_tiny_meta_header_in_use(next_block + TINY_BYTES_FOR_MSIZE(coalesced_msize), leftover_msize);
		} else {
			// Using the whole block.
			tiny_mag_ptr->mag_last_free = NULL;
			tiny_mag_ptr->mag_last_free_msize = 0;
			tiny_mag_ptr->mag_last_free_rgn = NULL;
			trailer->objects_in_use--;
		}
		set_tiny_meta_header_middle(next_block);
		coalesced_msize = 0; // No net change in memory use
	} else {
#endif // CONFIG_TINY_CACHE
		/*
		 * Try to expand into unused space immediately after this block.
		 */
		msize_t unused_msize = TINY_MSIZE_FOR_BYTES(tiny_mag_ptr->mag_bytes_free_at_end);
		void *unused_start = TINY_REGION_END(TINY_REGION_FOR_PTR(ptr)) - tiny_mag_ptr->mag_bytes_free_at_end;
		if (tiny_mag_ptr->mag_last_region == TINY_REGION_FOR_PTR(ptr)
				&& coalesced_msize < unused_msize && unused_start == ptr + old_size) {
			// The block at the start of mag_bytes_free_at_end is marked as
			// header/in-use and the next one has header/free. We need to
			// reset both the header and in-use bit in the first block and we
			// need to reset the header bit in the second block if it's part of
			// the new allocation.
			set_tiny_meta_header_middle(unused_start);
			if (coalesced_msize > 1) {
				set_tiny_meta_header_middle(unused_start + TINY_QUANTUM);
			}
			tiny_mag_ptr->mag_bytes_free_at_end -= TINY_BYTES_FOR_MSIZE(coalesced_msize);
			if (tiny_mag_ptr->mag_bytes_free_at_end) {
				// Mark the first block of the remaining free area as a header and in-use.
				set_tiny_meta_header_in_use_1(ptr + TINY_BYTES_FOR_MSIZE(new_msize));
			}
		} else {
			/*
			 * Look for a free block immediately afterwards.  If it's large
			 * enough, we can consume (part of) it.
			 */
			is_free = tiny_meta_header_is_free(next_block);
			if (!is_free) {
				SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
				return 0; // next_block is in use;
			}
			next_msize = get_tiny_free_size(next_block);
			if (old_msize + next_msize < new_msize) {
				SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
				return 0; // even with next block, not enough
			}
			/*
		 	 * The following block is big enough; pull it from its freelist and chop off enough to satisfy
		 	 * our needs.
		 	 */
			tiny_free_list_remove_ptr(rack, tiny_mag_ptr, next_block, next_msize);
			set_tiny_meta_header_middle(next_block); // clear the meta_header to enable coalescing backwards
			leftover_msize = next_msize - coalesced_msize;
			if (leftover_msize) {
				/* there's some left, so put the remainder back */
				leftover = (void *)((uintptr_t)next_block + TINY_BYTES_FOR_MSIZE(coalesced_msize));
				tiny_free_list_add_ptr(rack, tiny_mag_ptr, leftover, leftover_msize);
			}
			set_tiny_meta_header_in_use(ptr, old_msize + coalesced_msize);
		}
#if CONFIG_TINY_CACHE
	}
#endif // CONFIG_TINY_CACHE
#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "in tiny_try_realloc_in_place(), ptr=%p, msize=%d\n", ptr, old_msize + coalesced_msize);
	}
#endif
	tiny_mag_ptr->mag_num_bytes_in_objects += TINY_BYTES_FOR_MSIZE(coalesced_msize);

	// Update this region's bytes in use count
	region_trailer_t *node = REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr));
	size_t bytes_used = node->bytes_used + TINY_BYTES_FOR_MSIZE(coalesced_msize);
	node->bytes_used = (unsigned int)bytes_used;

	// Emptiness discriminant
	if (bytes_used < DENSITY_THRESHOLD(TINY_REGION_PAYLOAD_BYTES)) {
		/* After this reallocation the region is still sparse, so it must have been even more so before
		 * the reallocation. That implies the region is already correctly marked. Do nothing. */
	} else {
		/* Region has crossed threshold from sparsity to density. Mark it not "suitable" on the
		 * recirculation candidates list. */
		node->recirc_suitable = FALSE;
	}

	SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
	CHECK(szone, __PRETTY_FUNCTION__);
	return 1;
}

static char *tiny_check_fail_msg = "*** check: incorrect tiny region ";

#define TINY_CHECK_FAIL(fmt, ...) \
	malloc_zone_check_fail(tiny_check_fail_msg, \
			"%ld, counter=%d\n" fmt, region_index, counter, __VA_ARGS__);

boolean_t
tiny_check_region(rack_t *rack, region_t region, size_t region_index,
		unsigned counter)
{
	uintptr_t start, ptr, region_end;
	boolean_t prev_free = 0;
	boolean_t is_free;
	msize_t msize;
	tiny_free_list_t *free_head;
	void *follower, *previous, *next;
	mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(region);
	magazine_t *tiny_mag_ptr = &(rack->magazines[mag_index]);

	// Assumes locked
	CHECK_MAGAZINE_PTR_LOCKED(szone, tiny_mag_ptr, __PRETTY_FUNCTION__);

	// Do not check the region if pinned_to_depot is not zero because it
	// may not be in a consistent state (specifically, if may have a
	// block marked as in-use that's longer than any legal allocation,
	// which upsets get_tiny_meta_header() because it can't determine the
	// block's length).
	if (!REGION_TRAILER_FOR_TINY_REGION(region)->pinned_to_depot) {
		return 1;
	}

	/* establish region limits */
	start = (uintptr_t)TINY_REGION_ADDRESS(region);
	ptr = start;
	if (region == tiny_mag_ptr->mag_last_region) {
		ptr += tiny_mag_ptr->mag_bytes_free_at_start;

		/*
		 * Check the leading block's integrity here also.
		 */
		if (tiny_mag_ptr->mag_bytes_free_at_start) {
			msize = get_tiny_meta_header((void *)(ptr - TINY_QUANTUM), &is_free);
			if (is_free || (msize != 1)) {
				TINY_CHECK_FAIL("*** invariant broken for leader block %p - %d %d\n",
						(void *)(ptr - TINY_QUANTUM), msize, is_free);
				return 0;
			}
		}
	}
	region_end = (uintptr_t)TINY_REGION_END(region);

	/*
	 * The last region may have a trailing chunk which has not been converted into inuse/freelist
	 * blocks yet.
	 */
	if (region == tiny_mag_ptr->mag_last_region) {
		region_end -= tiny_mag_ptr->mag_bytes_free_at_end;
	}

	/*
	 * Scan blocks within the region.
	 */
	while (ptr < region_end) {
		/*
		 * If the first block is free, and its size is 65536 (msize = 0) then the entire region is
		 * free.
		 */
		msize = get_tiny_meta_header((void *)ptr, &is_free);
		if (is_free && !msize && (ptr == start)) {
			return 1;
		}

		/*
		 * If the block's size is 65536 (msize = 0) then since we're not the first entry the size is
		 * corrupt.
		 */
		if (!msize) {
			TINY_CHECK_FAIL("*** invariant broken for tiny block %p this msize=%d - size is too small\n", (void *)ptr, msize);
			return 0;
		}

		if (!is_free) {
			/*
			 * In use blocks cannot be more than NUM_TINY_SLOTS quanta large.
			 */
			prev_free = 0;
			if (msize > NUM_TINY_SLOTS) {
				TINY_CHECK_FAIL("*** invariant broken for %p this tiny msize=%d - size is too large\n", (void *)ptr, msize);
				return 0;
			}
			/* move to next block */
			ptr += TINY_BYTES_FOR_MSIZE(msize);
		} else {
#if !CONFIG_RELAXED_INVARIANT_CHECKS
			/*
			 * Free blocks must have been coalesced, we cannot have a free block following another
			 * free block.
			 */
			if (prev_free) {
				TINY_CHECK_FAIL("*** invariant broken for free block %p this tiny msize=%d: two free blocks in a row\n", (void *)ptr, msize);
				return 0;
			}
#endif // CONFIG_RELAXED_INVARIANT_CHECKS
			prev_free = 1;
			/*
			 * Check the integrity of this block's entry in its freelist.
			 */
			free_head = (tiny_free_list_t *)ptr;
			previous = free_list_unchecksum_ptr(rack, &free_head->previous);
			next = free_list_unchecksum_ptr(rack, &free_head->next);
			if (previous && !tiny_meta_header_is_free(previous)) {
				TINY_CHECK_FAIL("*** invariant broken for %p (previous %p is not a free pointer)\n", (void *)ptr, previous);
				return 0;
			}
			if (next && !tiny_meta_header_is_free(next)) {
				TINY_CHECK_FAIL("*** invariant broken for %p (next in free list %p is not a free pointer)\n", (void *)ptr, next);
				return 0;
			}
			/*
			 * Check the free block's trailing size value.
			 */
			follower = FOLLOWING_TINY_PTR(ptr, msize);
			if (((uintptr_t)follower != region_end) && (get_tiny_previous_free_msize(follower) != msize)) {
				TINY_CHECK_FAIL("*** invariant broken for tiny free %p followed by %p in region [%p-%p] "
						"(end marker incorrect) should be %d; in fact %d\n",
						(void *)ptr, follower, TINY_REGION_ADDRESS(region), (void *)region_end,
						msize, get_tiny_previous_free_msize(follower));
				return 0;
			}
			/* move to next block */
			ptr = (uintptr_t)follower;
		}
	}
	/*
	 * Ensure that we scanned the entire region
	 */
	if (ptr != region_end) {
		TINY_CHECK_FAIL("*** invariant broken for region end %p - %p\n", (void *)ptr, (void *)region_end);
		return 0;
	}
	/*
	 * Check the trailing block's integrity.
	 */
	if (region == tiny_mag_ptr->mag_last_region) {
		if (tiny_mag_ptr->mag_bytes_free_at_end) {
			msize = get_tiny_meta_header((void *)ptr, &is_free);
			if (is_free || (msize != 1)) {
				TINY_CHECK_FAIL("*** invariant broken for blocker block %p - %d %d\n", (void *)ptr, msize, is_free);
				return 0;
			}
		}
	}
	return 1;
}

kern_return_t
tiny_in_use_enumerator(task_t task,
					   void *context,
					   unsigned type_mask,
					   szone_t *szone,
					   memory_reader_t reader,
					   vm_range_recorder_t recorder)
{
	size_t num_regions;
	size_t index;
	region_t *regions;
	vm_range_t buffer[MAX_RECORDER_BUFFER];
	unsigned count = 0;
	kern_return_t err;
	region_t region;
	vm_range_t range;
	vm_range_t admin_range;
	vm_range_t ptr_range;
	unsigned char *mapped_region;
	uint32_t *block_header;
	uint32_t *in_use;
	unsigned block_index;
	unsigned block_limit;
	boolean_t is_free;
	msize_t msize;
	void *mapped_ptr;
	unsigned bit;
	magazine_t *tiny_mag_base = NULL;

	region_hash_generation_t *trg_ptr;
	err = reader(task, (vm_address_t)szone->tiny_rack.region_generation, sizeof(region_hash_generation_t), (void **)&trg_ptr);
	if (err) {
		return err;
	}

	num_regions = trg_ptr->num_regions_allocated;
	err = reader(task, (vm_address_t)trg_ptr->hashed_regions, sizeof(region_t) * num_regions, (void **)&regions);
	if (err) {
		return err;
	}

	if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
		// Map in all active magazines. Do this outside the iteration over regions.
		err = reader(task, (vm_address_t)(szone->tiny_rack.magazines), szone->tiny_rack.num_magazines * sizeof(magazine_t),
					 (void **)&tiny_mag_base);
		if (err) {
			return err;
		}
	}

	for (index = 0; index < num_regions; ++index) {
		region = regions[index];
		if (HASHRING_OPEN_ENTRY != region && HASHRING_REGION_DEALLOCATED != region) {
			range.address = (vm_address_t)TINY_REGION_ADDRESS(region);
			range.size = (vm_size_t)TINY_REGION_SIZE;
			if (type_mask & MALLOC_ADMIN_REGION_RANGE_TYPE) {
				admin_range.address = range.address + TINY_METADATA_START;
				admin_range.size = TINY_METADATA_SIZE;
				recorder(task, context, MALLOC_ADMIN_REGION_RANGE_TYPE, &admin_range, 1);
			}
			if (type_mask & (MALLOC_PTR_REGION_RANGE_TYPE | MALLOC_ADMIN_REGION_RANGE_TYPE)) {
				ptr_range.address = range.address;
				ptr_range.size = NUM_TINY_BLOCKS * TINY_QUANTUM;
				recorder(task, context, MALLOC_PTR_REGION_RANGE_TYPE, &ptr_range, 1);
			}
			if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
				err = reader(task, range.address, range.size, (void **)&mapped_region);
				if (err) {
					return err;
				}

				mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(mapped_region);
				magazine_t *tiny_mag_ptr = tiny_mag_base + mag_index;

				int cached_free_blocks = 0;
#if CONFIG_TINY_CACHE
				// Each magazine could have a pointer to a cached free block from
				// this region. Count the regions that have such a pointer.
				for (mag_index = 0; mag_index < szone->tiny_rack.num_magazines; mag_index++) {
					if ((void *)range.address == (tiny_mag_base + mag_index)->mag_last_free_rgn) {
						cached_free_blocks++;
					}
				}
#endif // CONFIG_TINY_CACHE

				block_header = (uint32_t *)(mapped_region + TINY_METADATA_START + sizeof(region_trailer_t));
				in_use = TINY_INUSE_FOR_HEADER(block_header);
				block_index = 0;
				block_limit = NUM_TINY_BLOCKS;
				if (region == tiny_mag_ptr->mag_last_region) {
					block_index += TINY_MSIZE_FOR_BYTES(tiny_mag_ptr->mag_bytes_free_at_start);
					block_limit -= TINY_MSIZE_FOR_BYTES(tiny_mag_ptr->mag_bytes_free_at_end);
				}

				for (; block_index < block_limit; block_index += msize) {
					vm_size_t block_offset = TINY_BYTES_FOR_MSIZE(block_index);
					is_free = !BITARRAY_BIT(in_use, block_index);
					if (is_free) {
						mapped_ptr = mapped_region + block_offset;

						// mapped_region, the address at which 'range' in 'task' has been
						// mapped into our process, is not necessarily aligned to
						// TINY_BLOCKS_ALIGN.
						//
						// Since the code in get_tiny_free_size() assumes the pointer came
						// from a properly aligned tiny region, and mapped_region is not
						// necessarily aligned, then do the size calculation directly.
						// If the next bit is set in the header bitmap, then the size is one
						// quantum.  Otherwise, read the size field.
						if (!BITARRAY_BIT(block_header, (block_index + 1))) {
							msize = TINY_FREE_SIZE(mapped_ptr);
						} else {
							msize = 1;
						}
					} else {
#if CONFIG_TINY_CACHE
						// If there are still magazines that have cached free
						// blocks in this region, check whether this is one of
						// them and don't return the block pointer if it is.
						vm_address_t ptr = range.address + block_offset;
						boolean_t block_cached = false;
						if (cached_free_blocks) {
							for (mag_index = 0; mag_index < szone->tiny_rack.num_magazines; mag_index++) {
								if ((void *)ptr == (tiny_mag_base + mag_index)->mag_last_free) {
									block_cached = true;
									cached_free_blocks--;
									msize = (tiny_mag_base + mag_index)->mag_last_free_msize;
									break;
								}
							}
						}
						if (block_cached) {
							if (!msize) {
								return KERN_FAILURE; // Somethings amiss. Avoid looping at this block_index.
							}
							continue;
						}
#endif // CONFIG_TINY_CACHE
						msize = 1;
						bit = block_index + 1;
						while (!BITARRAY_BIT(block_header, bit)) {
							bit++;
							msize++;
						}
						buffer[count].address = range.address + block_offset;
						buffer[count].size = TINY_BYTES_FOR_MSIZE(msize);
						count++;
						if (count >= MAX_RECORDER_BUFFER) {
							recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE, buffer, count);
							count = 0;
						}
					}

					if (!msize) {
						return KERN_FAILURE; // Somethings amiss. Avoid looping at this block_index.
					}
				}
				if (count) {
					recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE, buffer, count);
					count = 0;
				}
			}
		}
	}
	return 0;
}

void *
tiny_malloc_from_free_list(rack_t *rack, magazine_t *tiny_mag_ptr, mag_index_t mag_index, msize_t msize)
{
	tiny_free_list_t *ptr;
	msize_t this_msize;
	grain_t slot = tiny_slot_from_msize(msize);
	free_list_t *free_list = tiny_mag_ptr->mag_free_list;
	free_list_t *the_slot = free_list + slot;
	tiny_free_list_t *next;
	free_list_t *limit;
#if defined(__LP64__)
	uint64_t bitmap;
#else
	uint32_t bitmap;
#endif
	msize_t leftover_msize;
	tiny_free_list_t *leftover_ptr;

	// Assumes we've locked the region
	CHECK_MAGAZINE_PTR_LOCKED(szone, tiny_mag_ptr, __PRETTY_FUNCTION__);

	// Look for an exact match by checking the freelist for this msize.
	//
	ptr = the_slot->p;
	if (ptr) {
		next = free_list_unchecksum_ptr(rack, &ptr->next);
		if (next) {
			next->previous = ptr->previous;
		} else {
			BITMAPV_CLR(tiny_mag_ptr->mag_bitmap, slot);
		}
		the_slot->p = next;
		this_msize = msize;
#if DEBUG_MALLOC
		if (LOG(szone, ptr)) {
			malloc_report(ASL_LEVEL_INFO, "in tiny_malloc_from_free_list(), exact match ptr=%p, this_msize=%d\n", ptr, this_msize);
		}
#endif
		tiny_update_region_free_list_for_remove(slot, ptr, next);

		goto return_tiny_alloc;
	}

	// Mask off the bits representing slots holding free blocks smaller than the
	// size we need.  If there are no larger free blocks, try allocating from
	// the free space at the end of the tiny region.
#if defined(__LP64__)
	bitmap = ((uint64_t *)(tiny_mag_ptr->mag_bitmap))[0] & ~((1ULL << slot) - 1);
#else
	bitmap = tiny_mag_ptr->mag_bitmap[0] & ~((1 << slot) - 1);
#endif
	if (!bitmap) {
		goto try_tiny_malloc_from_end;
	}

	slot = BITMAPV_CTZ(bitmap);
	limit = free_list + NUM_TINY_SLOTS;
	free_list += slot;

	if (free_list < limit) {
		ptr = free_list->p;
		if (ptr) {
			next = free_list_unchecksum_ptr(rack, &ptr->next);
			free_list->p = next;
			if (next) {
				next->previous = ptr->previous;
			} else {
				BITMAPV_CLR(tiny_mag_ptr->mag_bitmap, slot);
			}
			this_msize = get_tiny_free_size(ptr);
			tiny_update_region_free_list_for_remove(slot, ptr, next);
			goto add_leftover_and_proceed;
		}
#if DEBUG_MALLOC
		malloc_report(ASL_LEVEL_ERR, "in tiny_malloc_from_free_list(), mag_bitmap out of sync, slot=%d\n", slot);
#endif
	}

	// We are now looking at the last slot, which contains blocks equal to, or
	// due to coalescing of free blocks, larger than NUM_TINY_SLOTS * tiny quantum size.
	// If the last freelist is not empty, and the head contains a block that is
	// larger than our request, then the remainder is put back on the free list.
	ptr = limit->p;
	if (ptr) {
		this_msize = get_tiny_free_size(ptr);
		next = free_list_unchecksum_ptr(rack, &ptr->next);
		if (this_msize - msize > NUM_TINY_SLOTS) {
			// the leftover will go back to the free list, so we optimize by
			// modifying the free list rather than a pop and push of the head
			leftover_msize = this_msize - msize;
			leftover_ptr = (tiny_free_list_t *)((unsigned char *)ptr + TINY_BYTES_FOR_MSIZE(msize));
			limit->p = leftover_ptr;
			if (next) {
				next->previous.u = free_list_checksum_ptr(rack, leftover_ptr);
			}
			leftover_ptr->previous = ptr->previous;
			leftover_ptr->next = ptr->next;
			set_tiny_meta_header_free(leftover_ptr, leftover_msize);
#if DEBUG_MALLOC
			if (LOG(szone, ptr)) {
				malloc_report(ASL_LEVEL_INFO,
							  "in tiny_malloc_from_free_list(), last slot ptr=%p, msize=%d this_msize=%d\n", ptr, msize, this_msize);
			}
#endif
			this_msize = msize;
			tiny_update_region_free_list_for_remove(NUM_TINY_SLOTS, ptr, leftover_ptr);

			goto return_tiny_alloc;
		}
		if (next) {
			next->previous = ptr->previous;
		}
		limit->p = next;
		tiny_update_region_free_list_for_remove(slot, ptr, next);
		goto add_leftover_and_proceed;
		/* NOTREACHED */
	}

try_tiny_malloc_from_end:
	// Let's see if we can use tiny_mag_ptr->mag_bytes_free_at_end
	if (tiny_mag_ptr->mag_bytes_free_at_end >= TINY_BYTES_FOR_MSIZE(msize)) {
		ptr = (tiny_free_list_t *)((uintptr_t)TINY_REGION_END(tiny_mag_ptr->mag_last_region) - tiny_mag_ptr->mag_bytes_free_at_end);
		tiny_mag_ptr->mag_bytes_free_at_end -= TINY_BYTES_FOR_MSIZE(msize);
		if (tiny_mag_ptr->mag_bytes_free_at_end) {
			// let's add an in use block after ptr to serve as boundary
			set_tiny_meta_header_in_use_1((unsigned char *)ptr + TINY_BYTES_FOR_MSIZE(msize));
		}
		this_msize = msize;
#if DEBUG_MALLOC
		if (LOG(szone, ptr)) {
			malloc_report(ASL_LEVEL_INFO, "in tiny_malloc_from_free_list(), from end ptr=%p, msize=%d\n", ptr, msize);
		}
#endif
		goto return_tiny_alloc;
	}
#if CONFIG_ASLR_INTERNAL
	// Try from start if nothing left at end
	if (tiny_mag_ptr->mag_bytes_free_at_start >= TINY_BYTES_FOR_MSIZE(msize)) {
		ptr = (tiny_free_list_t *)(TINY_REGION_ADDRESS(tiny_mag_ptr->mag_last_region) + tiny_mag_ptr->mag_bytes_free_at_start -
							  TINY_BYTES_FOR_MSIZE(msize));
		tiny_mag_ptr->mag_bytes_free_at_start -= TINY_BYTES_FOR_MSIZE(msize);
		if (tiny_mag_ptr->mag_bytes_free_at_start) {
			// let's add an in use block before ptr to serve as boundary
			set_tiny_meta_header_in_use_1((unsigned char *)ptr - TINY_QUANTUM);
		}
		this_msize = msize;
#if DEBUG_MALLOC
		if (LOG(szone, ptr)) {
			malloc_report(ASL_LEVEL_INFO, "in tiny_malloc_from_free_list(), from start ptr=%p, msize=%d\n", ptr, msize);
		}
#endif
		goto return_tiny_alloc;
	}
#endif
	return NULL;

add_leftover_and_proceed:
	if (!this_msize || (this_msize > msize)) {
		leftover_msize = this_msize - msize;
		leftover_ptr = (tiny_free_list_t *)((unsigned char *)ptr + TINY_BYTES_FOR_MSIZE(msize));
#if DEBUG_MALLOC
		if (LOG(szone, ptr)) {
			malloc_report(ASL_LEVEL_INFO, "in tiny_malloc_from_free_list(), adding leftover ptr=%p, this_msize=%d\n", ptr, this_msize);
		}
#endif
		tiny_free_list_add_ptr(rack, tiny_mag_ptr, leftover_ptr, leftover_msize);
		this_msize = msize;
	}

return_tiny_alloc:
	tiny_mag_ptr->mag_num_objects++;
	tiny_mag_ptr->mag_num_bytes_in_objects += TINY_BYTES_FOR_MSIZE(this_msize);

	// Check that the region cookie is intact and update the region's bytes in use count
	region_t *region = TINY_REGION_FOR_PTR(ptr);
	region_trailer_t *trailer = REGION_TRAILER_FOR_TINY_REGION(region);
	region_check_cookie(region, trailer);
	size_t bytes_used = trailer->bytes_used + TINY_BYTES_FOR_MSIZE(this_msize);
	trailer->bytes_used = (unsigned int)bytes_used;
	trailer->objects_in_use++;

	// Emptiness discriminant
	if (bytes_used < DENSITY_THRESHOLD(TINY_REGION_PAYLOAD_BYTES)) {
		/* After this allocation the region is still sparse, so it must have been even more so before
		 * the allocation. That implies the region is already correctly marked. Do nothing. */
	} else {
		/* Region has crossed threshold from sparsity to density. Mark it not "suitable" on the
		 * recirculation candidates list. */
		trailer->recirc_suitable = FALSE;
	}
#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "in tiny_malloc_from_free_list(), ptr=%p, this_msize=%d, msize=%d\n", ptr, this_msize, msize);
	}
#endif
	if (this_msize > 1) {
		set_tiny_meta_header_in_use(ptr, this_msize);
	} else {
		set_tiny_meta_header_in_use_1(ptr);
	}
	return ptr;
}

void *
tiny_malloc_should_clear(rack_t *rack, msize_t msize, boolean_t cleared_requested)
{
	void *ptr;
	mag_index_t mag_index = tiny_mag_get_thread_index() % rack->num_magazines;
	magazine_t *tiny_mag_ptr = &(rack->magazines[mag_index]);

	MALLOC_TRACE(TRACE_tiny_malloc, (uintptr_t)rack, TINY_BYTES_FOR_MSIZE(msize), (uintptr_t)tiny_mag_ptr, cleared_requested);

#if DEBUG_MALLOC
	if (DEPOT_MAGAZINE_INDEX == mag_index) {
		malloc_zone_error(rack->debug_flags, true, "malloc called for magazine index -1\n");
		return (NULL);
	}

	if (!msize) {
		malloc_zone_error(rack->debug_flags, true, "invariant broken (!msize) in allocation (region)\n");
		return (NULL);
	}
#endif

	SZONE_MAGAZINE_PTR_LOCK(tiny_mag_ptr);

#if CONFIG_TINY_CACHE
	ptr = tiny_mag_ptr->mag_last_free;

	if (tiny_mag_ptr->mag_last_free_msize == msize) {
		// we have a winner
		tiny_mag_ptr->mag_last_free = NULL;
		tiny_mag_ptr->mag_last_free_msize = 0;
		tiny_mag_ptr->mag_last_free_rgn = NULL;
		SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
		CHECK(szone, __PRETTY_FUNCTION__);
		if (cleared_requested) {
			memset(ptr, 0, TINY_BYTES_FOR_MSIZE(msize));
		}
#if DEBUG_MALLOC
		if (LOG(szone, ptr)) {
			malloc_report(ASL_LEVEL_INFO, "in tiny_malloc_should_clear(), tiny cache ptr=%p, msize=%d\n", ptr, msize);
		}
#endif
		return ptr;
	}
#endif /* CONFIG_TINY_CACHE */

	while (1) {
		ptr = tiny_malloc_from_free_list(rack, tiny_mag_ptr, mag_index, msize);
		if (ptr) {
			SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
			CHECK(szone, __PRETTY_FUNCTION__);
			if (cleared_requested) {
				memset(ptr, 0, TINY_BYTES_FOR_MSIZE(msize));
			}
			return ptr;
		}

#if CONFIG_RECIRC_DEPOT
		if (tiny_get_region_from_depot(rack, tiny_mag_ptr, mag_index, msize)) {
			ptr = tiny_malloc_from_free_list(rack, tiny_mag_ptr, mag_index, msize);
			if (ptr) {
				SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
				CHECK(szone, __PRETTY_FUNCTION__);
				if (cleared_requested) {
					memset(ptr, 0, TINY_BYTES_FOR_MSIZE(msize));
				}
				return ptr;
			}
		}
#endif // CONFIG_RECIRC_DEPOT

		// The magazine is exhausted. A new region (heap) must be allocated to satisfy this call to malloc().
		// The allocation, an mmap() system call, will be performed outside the magazine spin locks by the first
		// thread that suffers the exhaustion. That thread sets "alloc_underway" and enters a critical section.
		// Threads arriving here later are excluded from the critical section, yield the CPU, and then retry the
		// allocation. After some time the magazine is resupplied, the original thread leaves with its allocation,
		// and retry-ing threads succeed in the code just above.
		if (!tiny_mag_ptr->alloc_underway) {
			void *fresh_region;

			// time to create a new region (do this outside the magazine lock)
			tiny_mag_ptr->alloc_underway = TRUE;
			OSMemoryBarrier();
			SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
			fresh_region = mvm_allocate_pages_securely(TINY_REGION_SIZE, TINY_BLOCKS_ALIGN, VM_MEMORY_MALLOC_TINY, rack->debug_flags);
			SZONE_MAGAZINE_PTR_LOCK(tiny_mag_ptr);

			// DTrace USDT Probe
			MAGMALLOC_ALLOCREGION(TINY_SZONE_FROM_RACK(rack), (int)mag_index, fresh_region, TINY_REGION_SIZE);

			if (!fresh_region) { // out of memory!
				tiny_mag_ptr->alloc_underway = FALSE;
				OSMemoryBarrier();
				SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
				return NULL;
			}

			region_set_cookie(REGION_TRAILER_FOR_TINY_REGION(fresh_region));
			ptr = tiny_malloc_from_region_no_lock(rack, tiny_mag_ptr, mag_index, msize, fresh_region);

			// we don't clear because this freshly allocated space is pristine
			tiny_mag_ptr->alloc_underway = FALSE;
			OSMemoryBarrier();
			SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
			CHECK(szone, __PRETTY_FUNCTION__);
			return ptr;
		} else {
			SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
			yield();
			SZONE_MAGAZINE_PTR_LOCK(tiny_mag_ptr);
		}
	}
	/* NOTREACHED */
}

size_t
tiny_size(rack_t *rack, const void *ptr)
{
	if (tiny_region_for_ptr_no_lock(rack, ptr)) {
		if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS) {
			return 0;
		}

		boolean_t is_free;
		msize_t msize = get_tiny_meta_header(ptr, &is_free);
		if (is_free) {
			return 0;
		}

#if CONFIG_TINY_CACHE
		{
			mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr));
			if (DEPOT_MAGAZINE_INDEX != mag_index) {
				magazine_t *tiny_mag_ptr = &rack->magazines[mag_index];

				if (msize < TINY_QUANTUM && ptr == tiny_mag_ptr->mag_last_free) {
					return 0;
				}
			} else {
				for (mag_index = 0; mag_index < rack->num_magazines; mag_index++) {
					magazine_t *tiny_mag_ptr = &(rack->magazines[mag_index]);

					if (msize < TINY_QUANTUM && ptr == tiny_mag_ptr->mag_last_free) {
						return 0;
					}
				}
			}
		}
#endif
		return TINY_BYTES_FOR_MSIZE(msize);
	}

	return 0;
}

static MALLOC_NOINLINE void
free_tiny_botch(rack_t *rack, tiny_free_list_t *ptr)
{
	mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr));
	magazine_t *tiny_mag_ptr = &(rack->magazines[mag_index]);
	SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
	malloc_zone_error(rack->debug_flags, true, "Double free of object %p\n", ptr);
}

void
free_tiny(rack_t *rack, void *ptr, region_t tiny_region, size_t known_size,
		boolean_t partial_free)
{
	msize_t msize;
	boolean_t is_free;
	mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(tiny_region);
	magazine_t *tiny_mag_ptr = &(rack->magazines[mag_index]);

	MALLOC_TRACE(TRACE_tiny_free, (uintptr_t)rack, (uintptr_t)ptr, (uintptr_t)tiny_mag_ptr, known_size);

	// ptr is known to be in tiny_region
	if (known_size) {
		msize = TINY_MSIZE_FOR_BYTES(known_size + TINY_QUANTUM - 1);
	} else {
		msize = get_tiny_meta_header(ptr, &is_free);
		if (is_free) {
			free_tiny_botch(rack, ptr);
			return;
		}
	}
#if DEBUG_MALLOC
	if (!msize) {
		malloc_report(ASL_LEVEL_ERR, "*** free_tiny() block in use is too large: %p\n", ptr);
		return;
	}
#endif

	SZONE_MAGAZINE_PTR_LOCK(tiny_mag_ptr);

#if CONFIG_TINY_CACHE
	// Depot does not participate in CONFIG_TINY_CACHE since it can't be directly malloc()'d
	if (DEPOT_MAGAZINE_INDEX != mag_index && !partial_free) {
		if (msize < TINY_QUANTUM) {					  // to see if the bits fit in the last 4 bits
			void *ptr2 = tiny_mag_ptr->mag_last_free; // Might be NULL
			msize_t msize2 = tiny_mag_ptr->mag_last_free_msize;
			region_t rgn2 = tiny_mag_ptr->mag_last_free_rgn;

			/* check that we don't already have this pointer in the cache */
			if (ptr == ptr2) {
				free_tiny_botch(rack, ptr);
				return;
			}

			if ((rack->debug_flags & MALLOC_DO_SCRIBBLE) && msize) {
				memset(ptr, SCRABBLE_BYTE, TINY_BYTES_FOR_MSIZE(msize));
			}

			tiny_mag_ptr->mag_last_free = ptr;
			tiny_mag_ptr->mag_last_free_msize = msize;
			tiny_mag_ptr->mag_last_free_rgn = tiny_region;

			if (!ptr2) {
				SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
				CHECK(szone, __PRETTY_FUNCTION__);
				return;
			}

			msize = msize2;
			ptr = ptr2;
			tiny_region = rgn2;
		}
	}
#endif /* CONFIG_TINY_CACHE */

	// Now in the time it took to acquire the lock, the region may have migrated
	// from one magazine to another. I.e. trailer->mag_index is volatile.
	// In which case the magazine lock we obtained (namely magazines[mag_index].mag_lock)
	// is stale. If so, keep on tryin' ...
	region_trailer_t *trailer = REGION_TRAILER_FOR_TINY_REGION(tiny_region);
	mag_index_t refreshed_index;

	while (mag_index != (refreshed_index = trailer->mag_index)) { // Note assignment
		SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
		mag_index = refreshed_index;
		tiny_mag_ptr = &(rack->magazines[mag_index]);
		SZONE_MAGAZINE_PTR_LOCK(tiny_mag_ptr);
	}

	if (tiny_free_no_lock(rack, tiny_mag_ptr, mag_index, tiny_region, ptr,
			msize, partial_free)) {
		SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
	}

	CHECK(szone, __PRETTY_FUNCTION__);
}

unsigned
tiny_batch_malloc(szone_t *szone, size_t size, void **results, unsigned count)
{
	msize_t msize = TINY_MSIZE_FOR_BYTES(size + TINY_QUANTUM - 1);
	unsigned found = 0;
	mag_index_t mag_index = tiny_mag_get_thread_index() % szone->tiny_rack.num_magazines;
	magazine_t *tiny_mag_ptr = &(szone->tiny_rack.magazines[mag_index]);

	// make sure to return objects at least one quantum in size
	if (!msize) {
		msize = 1;
	}

	CHECK(szone, __PRETTY_FUNCTION__);

	// We must lock the zone now, since tiny_malloc_from_free_list assumes that
	// the caller has done so.
	SZONE_MAGAZINE_PTR_LOCK(tiny_mag_ptr);

	// with the zone locked, allocate objects from the free list until all
	// sufficiently large objects have been exhausted, or we have met our quota
	// of objects to allocate.
	while (found < count) {
		void *ptr = tiny_malloc_from_free_list(&szone->tiny_rack, tiny_mag_ptr, mag_index, msize);
		if (!ptr) {
			break;
		}

		*results++ = ptr;
		found++;
	}
	SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
	return found;
}

void
tiny_batch_free(szone_t *szone, void **to_be_freed, unsigned count)
{
	unsigned cc = 0;
	void *ptr;
	region_t tiny_region = NULL;
	boolean_t is_free;
	msize_t msize;
	magazine_t *tiny_mag_ptr = NULL;
	mag_index_t mag_index = -1;

	// frees all the pointers in to_be_freed
	// note that to_be_freed may be overwritten during the process
	if (!count) {
		return;
	}

	CHECK(szone, __PRETTY_FUNCTION__);
	while (cc < count) {
		ptr = to_be_freed[cc];
		if (ptr) {
			if (NULL == tiny_region || tiny_region != TINY_REGION_FOR_PTR(ptr)) { // region same as last iteration?
				if (tiny_mag_ptr) {												  // non-NULL iff magazine lock taken
					SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
					tiny_mag_ptr = NULL;
				}

				tiny_region = tiny_region_for_ptr_no_lock(&szone->tiny_rack, ptr);

				if (tiny_region) {
					tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone->tiny_rack.magazines,
							REGION_TRAILER_FOR_TINY_REGION(tiny_region),
							MAGAZINE_INDEX_FOR_TINY_REGION(tiny_region));
					mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(tiny_region);
				}
			}
			if (tiny_region) {
				// this is a tiny pointer
				if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS) {
					break; // pointer to metadata; let the standard free deal with it
				}
				msize = get_tiny_meta_header(ptr, &is_free);
				if (is_free) {
					break; // a double free; let the standard free deal with it
				}
				if (!tiny_free_no_lock(&szone->tiny_rack, tiny_mag_ptr, mag_index, tiny_region, ptr, msize, false)) {
					// Arrange to re-acquire magazine lock
					tiny_mag_ptr = NULL;
					tiny_region = NULL;
				}
				to_be_freed[cc] = NULL;
			} else {
				// No region in this zone claims ptr; let the standard free deal with it
				break;
			}
		}
		cc++;
	}

	if (tiny_mag_ptr) {
		SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
		tiny_mag_ptr = NULL;
	}
}


void
print_tiny_free_list(task_t task, memory_reader_t reader,
		print_task_printer_t printer, rack_t *rack)
{
	tiny_free_list_t *ptr;
	_SIMPLE_STRING b = _simple_salloc();
	mag_index_t mag_index;

	if (b) {
		rack_t *mapped_rack;
		magazine_t *mapped_magazines;
		if (reader(task, (vm_address_t)rack, sizeof(struct rack_s),
				(void **)&mapped_rack)) {
			printer("Failed to map tiny rack\n");
			return;
		}
		_simple_sappend(b, "Tiny free sizes:\n");
		if (reader(task, (vm_address_t)mapped_rack->magazines,
				mapped_rack->num_magazines * sizeof(magazine_t),
				(void **)&mapped_magazines)) {
			printer("Failed to map tiny rack magazines\n");
			return;
		}

		for (mag_index = -1; mag_index < mapped_rack->num_magazines; mag_index++) {
			grain_t slot = 0;
			if (mag_index == -1) {
				_simple_sprintf(b, "\tRecirc depot: ");
			} else {
				_simple_sprintf(b, "\tMagazine %d: ", mag_index);
			}
			while (slot <= NUM_TINY_SLOTS) {
				ptr = mapped_magazines[mag_index].mag_free_list[slot].p;
				if (ptr) {
					_simple_sprintf(b, "%s%y[%d]; ",
							(slot == NUM_TINY_SLOTS) ? ">=" : "",
							(slot + 1) * TINY_QUANTUM,
							free_list_count(task, reader, printer, mapped_rack,
									(free_list_t){ .p = ptr }));
				}
				slot++;
			}
			_simple_sappend(b, "\n");
		}
		printer("%s\n", _simple_string(b));
		_simple_sfree(b);
	}
}

void
print_tiny_region(task_t task, memory_reader_t reader,
		print_task_printer_t printer, int level, region_t region,
		size_t bytes_at_start, size_t bytes_at_end)
{
	unsigned counts[1024];
	unsigned in_use = 0;
	uintptr_t start = (uintptr_t)TINY_REGION_ADDRESS(region);
	uintptr_t current = start + bytes_at_start;
	uintptr_t limit = (uintptr_t)TINY_REGION_END(region) - bytes_at_end;
	uintptr_t mapped_start;
	boolean_t is_free;
	msize_t msize;
	unsigned ci;
	_SIMPLE_STRING b;
	uintptr_t pgTot = 0;

	if (reader(task, (vm_address_t)start, TINY_REGION_SIZE,
            (void **)&mapped_start)) {
        printer("Failed to map tiny region at %p\n", start);
        return;
    }
    off_t start_offset = mapped_start - start;
    region_t mapped_region = (region_t)mapped_start;

	if (region == HASHRING_REGION_DEALLOCATED) {
		if ((b = _simple_salloc()) != NULL) {
			_simple_sprintf(b, "Tiny region [unknown address] was returned to the OS\n");
			printer("%s\n", _simple_string(b));
			_simple_sfree(b);
		}
		return;
	}

	memset(counts, 0, sizeof(counts));
	while (current < limit) {
		msize = get_tiny_meta_header_offset((void *)current, start_offset, &is_free);
		if (is_free && !msize && (current == start)) {
			// first block is all free
			uintptr_t pgLo = round_page_quanta(start + sizeof(tiny_free_list_t) + sizeof(msize_t));
			uintptr_t pgHi = trunc_page_quanta(start + TINY_REGION_SIZE - sizeof(msize_t));

			if (pgLo < pgHi) {
				pgTot += (pgHi - pgLo);
			}
			break;
		}
		if (!msize) {
			printer("*** error with %p: msize=%d\n", (void *)current, (unsigned)msize);
			break;
		}
		if (!is_free) {
			// block in use
			if (msize > NUM_TINY_SLOTS) {
				printer("*** error at %p msize for in_use is %d\n", (void *)current, msize);
			}
			if (msize < 1024) {
				counts[msize]++;
			}
			in_use++;
		} else {
			uintptr_t pgLo = round_page_quanta(current + sizeof(tiny_free_list_t) + sizeof(msize_t));
			uintptr_t pgHi = trunc_page_quanta(current + TINY_BYTES_FOR_MSIZE(msize) - sizeof(msize_t));

			if (pgLo < pgHi) {
				pgTot += (pgHi - pgLo);
			}
		}
		current += TINY_BYTES_FOR_MSIZE(msize);
	}
	if ((b = _simple_salloc()) != NULL) {
		mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(mapped_region);
		_simple_sprintf(b, "Tiny region [%p-%p, %y] \t", (void *)start, TINY_REGION_END(region), (int)TINY_REGION_SIZE);
        if (mag_index == DEPOT_MAGAZINE_INDEX) {
            _simple_sprintf(b, "Recirc depot \t");
        } else {
		    _simple_sprintf(b, "Magazine=%d \t", mag_index);
        }
		_simple_sprintf(b, "Allocations in use=%d \t Bytes in use=%ly (%d%%) \t",
				in_use, BYTES_USED_FOR_TINY_REGION(mapped_region),
				(int)(100.0F * BYTES_USED_FOR_TINY_REGION(mapped_region))/TINY_REGION_SIZE);
		if (bytes_at_end || bytes_at_start) {
			_simple_sprintf(b, "Untouched=%ly ", bytes_at_end + bytes_at_start);
		}
		if (mag_index == DEPOT_MAGAZINE_INDEX) {
			_simple_sprintf(b, "Advised MADV_FREE=%ly", pgTot);
		} else {
			_simple_sprintf(b, "Fragments subject to reclamation=%ly", pgTot);
#if CONFIG_RECIRC_DEPOT
			_simple_sprintf(b, tiny_region_below_recirc_threshold(mapped_region) ?
					"\tEmpty enough to be moved to recirc depot" :
					"\tNot empty enough to be moved to recirc depot");
#endif // CONFIG_RECIRC_DEPOT
		}
		if (level >= MALLOC_VERBOSE_PRINT_LEVEL && in_use) {
			_simple_sappend(b, "\n\tSizes in use: ");
			for (ci = 0; ci < 1024; ci++) {
				if (counts[ci]) {
					_simple_sprintf(b, "%y[%d] ", TINY_BYTES_FOR_MSIZE(ci), counts[ci]);
				}
			}
		}
		printer("%s\n", _simple_string(b));
		_simple_sfree(b);
	}
}

static char *tiny_freelist_fail_msg = "check: tiny free list incorrect ";

#define TINY_FREELIST_FAIL(fmt, ...) \
	malloc_zone_check_fail(tiny_freelist_fail_msg, \
			" (slot=%u), counter=%d\n" fmt, slot, counter, __VA_ARGS__);

boolean_t
tiny_free_list_check(rack_t *rack, grain_t slot, unsigned counter)
{
	mag_index_t mag_index;

	for (mag_index = -1; mag_index < rack->num_magazines; mag_index++) {
		magazine_t *tiny_mag_ptr = &(rack->magazines[mag_index]);
		SZONE_MAGAZINE_PTR_LOCK(tiny_mag_ptr);

		unsigned count = 0;
		tiny_free_list_t *ptr = rack->magazines[mag_index].mag_free_list[slot].p;
		boolean_t is_free;
		tiny_free_list_t *previous = NULL;

		while (ptr) {
			is_free = tiny_meta_header_is_free(ptr);
			if (!is_free) {
				TINY_FREELIST_FAIL("*** in-use ptr in free list slot=%u count=%d ptr=%p\n", slot, count, ptr);
				SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
				return 0;
			}
			if (((uintptr_t)ptr) & (TINY_QUANTUM - 1)) {
				TINY_FREELIST_FAIL("*** unaligned ptr in free list slot=%u count=%d ptr=%p\n", slot, count, ptr);
				SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
				return 0;
			}
			if (!tiny_region_for_ptr_no_lock(rack, ptr)) {
				TINY_FREELIST_FAIL("*** ptr not in szone slot=%d  count=%u ptr=%p\n", slot, count, ptr);
				SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
				return 0;
			}
			if (free_list_unchecksum_ptr(rack, &ptr->previous) != previous) {
				TINY_FREELIST_FAIL("*** previous incorrectly set slot=%u count=%d ptr=%p\n", slot, count, ptr);
				SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
				return 0;
			}
			previous = ptr;
			ptr = free_list_unchecksum_ptr(rack, &ptr->next);
			count++;
		}
		
		SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr);
	}
	return 1;
}
