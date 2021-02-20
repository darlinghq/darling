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

#ifndef __MAGAZINE_INLINE_H
#define __MAGAZINE_INLINE_H

extern unsigned int _os_cpu_number_override;

/*
 * MALLOC_ABSOLUTE_MAX_SIZE - There are many instances of addition to a
 * user-specified size_t, which can cause overflow (and subsequent crashes)
 * for values near SIZE_T_MAX.  Rather than add extra "if" checks everywhere
 * this occurs, it is easier to just set an absolute maximum request size,
 * and immediately return an error if the requested size exceeds this maximum.
 * Of course, values less than this absolute max can fail later if the value
 * is still too large for the available memory.  The largest value added
 * seems to be PAGE_SIZE (in the macro round_page()), so to be safe, we set
 * the maximum to be 2 * PAGE_SIZE less than SIZE_T_MAX.
 */
#define MALLOC_ABSOLUTE_MAX_SIZE (SIZE_T_MAX - (2 * PAGE_SIZE))

// Gets the allocation size for a calloc(). Multiples size by num_items and adds
// extra_size, storing the result in *total_size. Returns 0 on success, -1 (with
// errno set to ENOMEM) on overflow.
static int MALLOC_INLINE MALLOC_ALWAYS_INLINE
calloc_get_size(size_t num_items, size_t size, size_t extra_size, size_t *total_size)
{
	size_t alloc_size = size;
	if (num_items != 1 && (os_mul_overflow(num_items, size, &alloc_size)
			|| alloc_size > MALLOC_ABSOLUTE_MAX_SIZE)) {
		errno = ENOMEM;
		return -1;
	}
	if (extra_size && (os_add_overflow(alloc_size, extra_size, &alloc_size)
			|| alloc_size > MALLOC_ABSOLUTE_MAX_SIZE)) {
		errno = ENOMEM;
		return -1;
	}
	*total_size = alloc_size;
	return 0;
}

/*********************	FREE LIST UTILITIES  ************************/

// A free list entry is comprised of a pair of pointers, previous and next.
// These are used to implement a doubly-linked list, which permits efficient
// extraction.
//
// Because the free list entries are previously freed objects, a misbehaved
// program may write to a pointer after it has called free() on that pointer,
// either by dereference or buffer overflow from an adjacent pointer. This write
// would then corrupt the free list's previous and next pointers, leading to a
// crash.  In order to detect this case, we take advantage of the fact that
// malloc'd pointers are known to be at least 16 byte aligned, and thus have
// at least 4 trailing zero bits.
//
// When an entry is added to the free list, a checksum of the previous and next
// pointers is calculated and written to the high four bits of the respective
// pointers.  Upon detection of an invalid checksum, an error is logged and NULL
// is returned.  Since all code which un-checksums pointers checks for a NULL
// return, a potentially crashing or malicious dereference is avoided at the
// cost of leaking the corrupted block, and any subsequent blocks on the free
// list of that size.

#pragma mark forward decls

static MALLOC_INLINE uintptr_t free_list_gen_checksum(uintptr_t ptr) MALLOC_ALWAYS_INLINE;
static MALLOC_INLINE uintptr_t free_list_checksum_ptr(rack_t *rack, void *p) MALLOC_ALWAYS_INLINE;
static MALLOC_INLINE void *free_list_unchecksum_ptr(rack_t *rack, inplace_union *ptr) MALLOC_ALWAYS_INLINE;
static MALLOC_INLINE unsigned free_list_count(task_t task,
		memory_reader_t reader, print_task_printer_t printer,
		rack_t *mapped_rack, free_list_t ptr);

static MALLOC_INLINE void recirc_list_extract(rack_t *rack, magazine_t *mag_ptr, region_trailer_t *node) MALLOC_ALWAYS_INLINE;
static MALLOC_INLINE void recirc_list_splice_last(rack_t *rack, magazine_t *mag_ptr, region_trailer_t *node) MALLOC_ALWAYS_INLINE;
static MALLOC_INLINE void recirc_list_splice_first(rack_t *rack, magazine_t *mag_ptr, region_trailer_t *node) MALLOC_ALWAYS_INLINE;

static MALLOC_INLINE void
yield(void)
{
	thread_switch(MACH_PORT_NULL, SWITCH_OPTION_DEPRESS, 1);
}

static MALLOC_INLINE kern_return_t
_malloc_default_reader(task_t task, vm_address_t address, vm_size_t size, void **ptr)
{
	*ptr = (void *)address;
	return 0;
}

#pragma mark helpers

static MALLOC_INLINE MALLOC_ALWAYS_INLINE
uint64_t
platform_hw_memsize(void)
{
#if CONFIG_HAS_COMMPAGE_MEMSIZE
	return *(uint64_t *)(uintptr_t)_COMM_PAGE_MEMORY_SIZE;
#else
	uint64_t hw_memsize = 0;
	size_t uint64_t_size = sizeof(hw_memsize);
	// hw_memsize was always 0 if sysctlbyname failed, so preserve that behaviour
	(void)sysctlbyname("hw.memsize", &hw_memsize, &uint64_t_size, 0, 0);
	return hw_memsize;
#endif
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE
uint32_t
platform_cpu_count(void)
{
#if CONFIG_HAS_COMMPAGE_NCPUS
	return *(uint8_t *)(uintptr_t)_COMM_PAGE_NCPUS;
#else
	return sysconf(_SC_NPROCESSORS_CONF);
#endif
}

#pragma mark szone locking

static MALLOC_INLINE MALLOC_ALWAYS_INLINE void
SZONE_LOCK(szone_t *szone)
{
	_malloc_lock_lock(&szone->large_szone_lock);
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE void
SZONE_UNLOCK(szone_t *szone)
{
	_malloc_lock_unlock(&szone->large_szone_lock);
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE bool
SZONE_TRY_LOCK(szone_t *szone)
{
	return _malloc_lock_trylock(&szone->large_szone_lock);
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE void
SZONE_REINIT_LOCK(szone_t *szone)
{
	_malloc_lock_init(&szone->large_szone_lock);
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE void
SZONE_MAGAZINE_PTR_LOCK(magazine_t *mag_ptr)
{
	_malloc_lock_lock(&mag_ptr->magazine_lock);
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE void
SZONE_MAGAZINE_PTR_UNLOCK(magazine_t *mag_ptr)
{
	_malloc_lock_unlock(&mag_ptr->magazine_lock);
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE bool
SZONE_MAGAZINE_PTR_TRY_LOCK(magazine_t *mag_ptr)
{
	return _malloc_lock_trylock(&mag_ptr->magazine_lock);
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE void
SZONE_MAGAZINE_PTR_REINIT_LOCK(magazine_t *mag_ptr)
{
	_malloc_lock_init(&mag_ptr->magazine_lock);
}

#pragma mark free list

static MALLOC_NOINLINE void
free_list_checksum_botch(rack_t *rack, void *ptr, void *value)
{
	malloc_zone_error(rack->debug_flags, true,
			"Incorrect checksum for freed object %p: "
			"probably modified after being freed.\n"
			"Corrupt value: %p\n", ptr, value);
}

static MALLOC_INLINE uintptr_t
free_list_gen_checksum(uintptr_t ptr)
{
	uint8_t chk;

	chk = (unsigned char)(ptr >> 0);
	chk += (unsigned char)(ptr >> 8);
	chk += (unsigned char)(ptr >> 16);
	chk += (unsigned char)(ptr >> 24);
#if __LP64__
	chk += (unsigned char)(ptr >> 32);
	chk += (unsigned char)(ptr >> 40);
	chk += (unsigned char)(ptr >> 48);
	chk += (unsigned char)(ptr >> 56);
#endif

	return chk;
}

static unsigned
free_list_count(task_t task, memory_reader_t reader,
		print_task_printer_t printer, rack_t *mapped_rack, free_list_t ptr)
{
	unsigned int count = 0;

	// ptr.p is always pointer in the *target* process address space.
	inplace_free_entry_t mapped_inplace_free_entry;
	while (ptr.p) {
		count++;
		if (reader(task, (vm_address_t)ptr.inplace, sizeof(*ptr.inplace),
				(void **)&mapped_inplace_free_entry)) {
			printer("** invalid pointer in free list: %p\n", ptr.inplace);
			break;
		}
		ptr.p = free_list_unchecksum_ptr(mapped_rack, &mapped_inplace_free_entry->next);
	}
	return count;
}

#define NYBBLE 4
#if __LP64__
#define ANTI_NYBBLE (64 - NYBBLE)
#else
#define ANTI_NYBBLE (32 - NYBBLE)
#endif

static MALLOC_INLINE uintptr_t
free_list_checksum_ptr(rack_t *rack, void *ptr)
{
	uintptr_t p = (uintptr_t)ptr;
	return (p >> NYBBLE) | ((free_list_gen_checksum(p ^ rack->cookie) & (uintptr_t)0xF) << ANTI_NYBBLE); // compiles to rotate instruction
}

static MALLOC_INLINE void *
free_list_unchecksum_ptr(rack_t *rack, inplace_union *ptr)
{
	inplace_union p;
	uintptr_t t = ptr->u;

	t = (t << NYBBLE) | (t >> ANTI_NYBBLE); // compiles to rotate instruction
	p.u = t & ~(uintptr_t)0xF;

	if ((t ^ free_list_gen_checksum(p.u ^ rack->cookie)) & (uintptr_t)0xF) {
		free_list_checksum_botch(rack, ptr, (void *)ptr->u);
		__builtin_trap();
	}
	return p.p;
}

#undef ANTI_NYBBLE
#undef NYBBLE

#pragma mark recirc helpers

static MALLOC_INLINE void
recirc_list_extract(rack_t *rack, magazine_t *mag_ptr, region_trailer_t *node)
{
	// excise node from list
	if (NULL == node->prev) {
		mag_ptr->firstNode = node->next;
	} else {
		node->prev->next = node->next;
	}

	if (NULL == node->next) {
		mag_ptr->lastNode = node->prev;
	} else {
		node->next->prev = node->prev;
	}

	node->next = node->prev = NULL;
	mag_ptr->recirculation_entries--;
}

static MALLOC_INLINE void
recirc_list_splice_last(rack_t *rack, magazine_t *mag_ptr, region_trailer_t *node)
{
	if (NULL == mag_ptr->lastNode) {
		mag_ptr->firstNode = node;
		node->prev = NULL;
	} else {
		node->prev = mag_ptr->lastNode;
		mag_ptr->lastNode->next = node;
	}
	mag_ptr->lastNode = node;
	node->next = NULL;
	node->recirc_suitable = FALSE;
	mag_ptr->recirculation_entries++;
}

static MALLOC_INLINE void
recirc_list_splice_first(rack_t *rack, magazine_t *mag_ptr, region_trailer_t *node)
{
	if (NULL == mag_ptr->firstNode) {
		mag_ptr->lastNode = node;
		node->next = NULL;
	} else {
		node->next = mag_ptr->firstNode;
		mag_ptr->firstNode->prev = node;
	}
	mag_ptr->firstNode = node;
	node->prev = NULL;
	node->recirc_suitable = FALSE;
	mag_ptr->recirculation_entries++;
}

/*******************************************************************************
 * Region hash implementation
 *
 * This is essentially a duplicate of the existing Large allocator hash, minus
 * the ability to remove entries.  The two should be combined eventually.
 ******************************************************************************/
#pragma mark region hash

/*
 * hash_lookup_region_no_lock - Scan a hash ring looking for an entry for a
 * given region.
 *
 * FIXME: If consecutive queries of the same region are likely, a one-entry
 * cache would likely be a significant performance win here.
 */
static MALLOC_INLINE rgnhdl_t
hash_lookup_region_no_lock(region_t *regions, size_t num_entries, size_t shift, region_t r)
{
	size_t index, hash_index;
	rgnhdl_t entry;

	if (!num_entries) {
		return 0;
	}

	// Multiplicative hash where the multiplier is a prime near (ULONG_MAX / phi). [phi = 1.618033...]
	// Since the values of (((uintptr_t)r >> HASH_BLOCKS_ALIGN) are (roughly) an ascending sequence of integers,
	// this hash works really well. See Knuth TAOCP, Vol. 3.
#if __LP64__
	index = hash_index = (((uintptr_t)r >> HASH_BLOCKS_ALIGN) * 11400714819323198549ULL) >> (64 - shift);
#else
	index = hash_index = (((uintptr_t)r >> HASH_BLOCKS_ALIGN) * 2654435761UL) >> (32 - shift);
#endif
	do {
		entry = regions + index;
		if (*entry == 0) {
			return 0;
		}
		if (*entry == r) {
			return entry;
		}
		if (++index == num_entries) {
			index = 0;
		}
	} while (index != hash_index);
	return 0;
}

/*
 * hash_region_insert_no_lock - Insert a region into the hash ring.
 */
static void
hash_region_insert_no_lock(region_t *regions, size_t num_entries, size_t shift, region_t r)
{
	size_t index, hash_index;
	rgnhdl_t entry;

	// Multiplicative hash where the multiplier is a prime near (ULONG_MAX / phi). [phi = 1.618033...]
	// Since the values of (((uintptr_t)r >> HASH_BLOCKS_ALIGN) are (roughly) an ascending sequence of integers,
	// this hash works really well. See Knuth TAOCP, Vol. 3.
#if __LP64__
	index = hash_index = (((uintptr_t)r >> HASH_BLOCKS_ALIGN) * 11400714819323198549ULL) >> (64 - shift);
#else
	index = hash_index = (((uintptr_t)r >> HASH_BLOCKS_ALIGN) * 2654435761UL) >> (32 - shift);
#endif
	do {
		entry = regions + index;
		if (*entry == HASHRING_OPEN_ENTRY || *entry == HASHRING_REGION_DEALLOCATED) {
			*entry = r;
			return;
		}
		if (++index == num_entries) {
			index = 0;
		}
	} while (index != hash_index);
}

/*
 * hash_regions_alloc_no_lock - Allocate space for a number of entries.  This
 * must be a VM allocation as to avoid recursing between allocating a new small
 * region, and asking the small region to allocate space for the new list of
 * regions.
 */
static region_t *
hash_regions_alloc_no_lock(size_t num_entries)
{
	size_t size = num_entries * sizeof(region_t);
	return mvm_allocate_pages(round_page_quanta(size), 0, 0, VM_MEMORY_MALLOC);
}

/*
 * hash_regions_grow_no_lock - Grow the hash ring, and rehash the entries.
 * Return the new region and new size to update the szone.  Do not deallocate
 * the old entries since someone may still be allocating them.
 */
static MALLOC_INLINE region_t *
hash_regions_grow_no_lock(region_t *regions, size_t old_size, size_t *mutable_shift, size_t *new_size)
{
	// double in size and allocate memory for the regions
	*new_size = old_size + old_size;
	*mutable_shift = *mutable_shift + 1;
	region_t *new_regions = hash_regions_alloc_no_lock(*new_size);

	// rehash the entries into the new list
	size_t index;
	for (index = 0; index < old_size; ++index) {
		region_t r = regions[index];
		if (r != HASHRING_OPEN_ENTRY && r != HASHRING_REGION_DEALLOCATED) {
			hash_region_insert_no_lock(new_regions, *new_size, *mutable_shift, r);
		}
	}
	return new_regions;
}

#pragma mark mag index

/*
 * These commpage routines provide fast access to the logical cpu number
 * of the calling processor assuming no pre-emption occurs.
 */

extern unsigned int hyper_shift;
extern unsigned int phys_ncpus;
extern unsigned int logical_ncpus;

static MALLOC_INLINE MALLOC_ALWAYS_INLINE
unsigned int
mag_max_magazines(void)
{
	return max_magazines;
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE
unsigned int
mag_max_medium_magazines(void)
{
	return max_medium_magazines;
}

#pragma mark mag lock

static MALLOC_INLINE magazine_t *
mag_lock_zine_for_region_trailer(magazine_t *magazines, region_trailer_t *trailer, mag_index_t mag_index)
{
	mag_index_t refreshed_index;
	magazine_t *mag_ptr = &(magazines[mag_index]);

	// Take the lock  on entry.
	SZONE_MAGAZINE_PTR_LOCK(mag_ptr);

	// Now in the time it took to acquire the lock, the region may have migrated
	// from one magazine to another. In which case the magazine lock we obtained
	// (namely magazines[mag_index].mag_lock) is stale. If so, keep on tryin' ...
	while (mag_index != (refreshed_index = trailer->mag_index)) { // Note assignment

		SZONE_MAGAZINE_PTR_UNLOCK(mag_ptr);

		mag_index = refreshed_index;
		mag_ptr = &(magazines[mag_index]);
		SZONE_MAGAZINE_PTR_LOCK(mag_ptr);
	}

	return mag_ptr;
}

#pragma mark Region Cookie

extern uint64_t malloc_entropy[2];

static uint32_t
region_cookie(void)
{
	return (uint32_t)(malloc_entropy[0] >> 8) & 0xffff;
}

static MALLOC_INLINE void
region_check_cookie(region_t region, region_trailer_t *trailer)
{
	if (trailer->region_cookie != region_cookie())
	{
		malloc_zone_error(MALLOC_ABORT_ON_ERROR, true,
				"Region cookie corrupted for region %p (value is %x)\n",
				region, trailer->region_cookie);
		__builtin_unreachable();
	}
}

static MALLOC_INLINE void
region_set_cookie(region_trailer_t *trailer)
{
	trailer->region_cookie = region_cookie();
}

#pragma mark tiny allocator

/*
 * tiny_region_for_ptr_no_lock - Returns the tiny region containing the pointer,
 * or NULL if not found.
 */
static MALLOC_INLINE region_t
tiny_region_for_ptr_no_lock(rack_t *rack, const void *ptr)
{
	rgnhdl_t r = hash_lookup_region_no_lock(rack->region_generation->hashed_regions,
			rack->region_generation->num_regions_allocated,
			rack->region_generation->num_regions_allocated_shift,
			TINY_REGION_FOR_PTR(ptr));

	return r ? *r : r;
}

/*
 * Obtain the size of a free tiny block (in msize_t units).
 */
static MALLOC_INLINE msize_t
get_tiny_free_size_offset(const void *ptr, off_t mapped_offset)
{
	void *next_block = (void *)((uintptr_t)ptr + TINY_QUANTUM);
	void *region_end = TINY_REGION_END(TINY_REGION_FOR_PTR(ptr));

	// check whether the next block is outside the tiny region or a block header
	// if so, then the size of this block is one, and there is no stored size.
	if (next_block < region_end) {
		uint32_t *next_header = (uint32_t *)
				((char *)TINY_BLOCK_HEADER_FOR_PTR(next_block) + mapped_offset);
		msize_t next_index = TINY_INDEX_FOR_PTR(next_block);

		if (!BITARRAY_BIT(next_header, next_index)) {
			return TINY_FREE_SIZE((uintptr_t)ptr + mapped_offset);
		}
	}
	return 1;
}

static MALLOC_INLINE msize_t
get_tiny_free_size(const void *ptr)
{
	return get_tiny_free_size_offset(ptr, 0);
}

static MALLOC_INLINE msize_t
get_tiny_meta_header_offset(const void *ptr, off_t mapped_offset,
		boolean_t *is_free)
{
	// returns msize and is_free
	// may return 0 for the msize component (meaning 65536)
	uint32_t *block_header;
	msize_t index;

	block_header = (uint32_t *)((char *)TINY_BLOCK_HEADER_FOR_PTR(ptr) + mapped_offset);
	index = TINY_INDEX_FOR_PTR(ptr);

	msize_t midx = (index >> 5) << 1;
	uint32_t mask = 1 << (index & 31);
	*is_free = 0;
	if (0 == (block_header[midx] & mask)) { // if (!BITARRAY_BIT(block_header, index))
		return 0;
	}
	if (0 == (block_header[midx + 1] & mask)) { // if (!BITARRAY_BIT(in_use, index))
		*is_free = 1;
		return get_tiny_free_size_offset(ptr, mapped_offset);
	}

	// index >> 5 identifies the uint32_t to manipulate in the conceptually contiguous bits array
	// (index >> 5) << 1 identifies the uint32_t allowing for the actual interleaving
#if defined(__LP64__)
	// The return value, msize, is computed as the distance to the next 1 bit in block_header.
	// That's guaranteed to be somewhere in the next 64 bits. And those bits could span three
	// uint32_t block_header elements. Collect the bits into a single uint64_t and measure up with ffsl.
	uint32_t *addr = ((uint32_t *)block_header) + ((index >> 5) << 1);
	uint32_t bitidx = index & 31;
	uint64_t word_lo = addr[0];
	uint64_t word_mid = addr[2];
	uint64_t word_hi = addr[4];
	uint64_t word_lomid = (word_lo >> bitidx) | (word_mid << (32 - bitidx));
	uint64_t word = bitidx ? word_lomid | (word_hi << (64 - bitidx)) : word_lomid;
	uint32_t result = __builtin_ffsl(word >> 1);
#else
	// The return value, msize, is computed as the distance to the next 1 bit in block_header.
	// That's guaranteed to be somewhere in the next 32 bits. And those bits could span two
	// uint32_t block_header elements. Collect the bits into a single uint32_t and measure up with ffs.
	uint32_t *addr = ((uint32_t *)block_header) + ((index >> 5) << 1);
	uint32_t bitidx = index & 31;
	uint32_t word = bitidx ? (addr[0] >> bitidx) | (addr[2] << (32 - bitidx)) : addr[0];
	uint32_t result = __builtin_ffs(word >> 1);
#endif
	return result;
}

static MALLOC_INLINE msize_t
get_tiny_meta_header(const void *ptr, boolean_t *is_free)
{
	return get_tiny_meta_header_offset(ptr, 0, is_free);
}

#if CONFIG_RECIRC_DEPOT
/**
 * Returns true if a tiny region is below the emptiness threshold that allows it
 * to be moved to the recirc depot.
 */
static MALLOC_INLINE boolean_t
tiny_region_below_recirc_threshold(region_t region)
{
	region_trailer_t *trailer = REGION_TRAILER_FOR_TINY_REGION(region);
	return trailer->bytes_used < DENSITY_THRESHOLD(TINY_REGION_PAYLOAD_BYTES);
}

/**
 * Returns true if a tiny magazine has crossed the emptiness threshold that
 * allows regions to be moved to the recirc depot.
 */
static MALLOC_INLINE boolean_t
tiny_magazine_below_recirc_threshold(magazine_t *mag_ptr)
{
	size_t a = mag_ptr->num_bytes_in_magazine;	// Total bytes allocated to this magazine
	size_t u = mag_ptr->mag_num_bytes_in_objects; // In use (malloc'd) from this magaqzine

	return a - u > ((3 * TINY_REGION_PAYLOAD_BYTES) / 2)
			&& u < DENSITY_THRESHOLD(a);
}
#endif // CONFIG_RECIRC_DEPOT

#pragma mark small allocator

/*
 * small_region_for_ptr_no_lock - Returns the small region containing the pointer,
 * or NULL if not found.
 */
static MALLOC_INLINE region_t
small_region_for_ptr_no_lock(rack_t *rack, const void *ptr)
{
	rgnhdl_t r = hash_lookup_region_no_lock(rack->region_generation->hashed_regions,
			rack->region_generation->num_regions_allocated, rack->region_generation->num_regions_allocated_shift,
			SMALL_REGION_FOR_PTR(ptr));
	return r ? *r : r;
}

#if CONFIG_RECIRC_DEPOT
/**
 * Returns true if a small region is below the emptiness threshold that allows
 * it to be moved to the recirc depot.
 */
static MALLOC_INLINE boolean_t
small_region_below_recirc_threshold(region_t region)
{
	region_trailer_t *trailer = REGION_TRAILER_FOR_SMALL_REGION(region);
	return trailer->bytes_used < DENSITY_THRESHOLD(SMALL_REGION_PAYLOAD_BYTES);
}

/**
 * Returns true if a small magazine has crossed the emptiness threshold that
 * allows regions to be moved to the recirc depot.
 */
static MALLOC_INLINE boolean_t
small_magazine_below_recirc_threshold(magazine_t *mag_ptr)
{
	size_t a = mag_ptr->num_bytes_in_magazine;	// Total bytes allocated to this magazine
	size_t u = mag_ptr->mag_num_bytes_in_objects; // In use (malloc'd) from this magaqzine

	return a - u > ((3 * SMALL_REGION_PAYLOAD_BYTES) / 2)
			&& u < DENSITY_THRESHOLD(a);
}
#endif // CONFIG_RECIRC_DEPOT

#pragma mark medium allocator
/**
 * Returns true if a small region is below the emptiness threshold that allows
 * it to be moved to the recirc depot.
 */
static MALLOC_INLINE boolean_t
medium_region_below_recirc_threshold(region_t region)
{
	region_trailer_t *trailer = REGION_TRAILER_FOR_MEDIUM_REGION(region);
	return trailer->bytes_used < DENSITY_THRESHOLD(MEDIUM_REGION_PAYLOAD_BYTES);
}

/*
 * medium_region_for_ptr_no_lock - Returns the medium region containing the pointer,
 * or NULL if not found.
 */
static MALLOC_INLINE region_t
medium_region_for_ptr_no_lock(rack_t *rack, const void *ptr)
{
	rgnhdl_t r = hash_lookup_region_no_lock(rack->region_generation->hashed_regions,
			rack->region_generation->num_regions_allocated, rack->region_generation->num_regions_allocated_shift,
			MEDIUM_REGION_FOR_PTR(ptr));
	return r ? *r : r;
}

#endif // __MAGAZINE_INLINE_H
