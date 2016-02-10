/*
 * Copyright (c) 1999, 2006, 2008 Apple Inc. All rights reserved.
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

/* Author: Bertrand Serlet, August 1999 */

/*
    Multithread enhancements for "tiny" allocations introduced February 2008.
    These are in the spirit of "Hoard". See:
	Berger, E.D.; McKinley, K.S.; Blumofe, R.D.; Wilson, P.R. (2000).
	"Hoard: a scalable memory allocator for multithreaded applications".
	ACM SIGPLAN Notices 35 (11): 117-128. Berger2000. 
	<http://portal.acm.org/citation.cfm?id=356989.357000>
	Retrieved on 2008-02-22.
*/

/* gcc -g -O3 magazine_malloc.c malloc.c -o libmagmalloc.dylib -I. \
 -I/System/Library/Frameworks/System.framework/PrivateHeaders/ -funit-at-a-time \
 -dynamiclib -Wall -arch x86_64 -arch i386 -arch ppc */
	
#include <TargetConditionals.h>

#include "scalable_malloc.h"
#include "malloc_printf.h"
#include "_simple.h"
#include "magmallocProvider.h"

#include <pthread_internals.h>	/* for pthread_lock_t SPI */
#include <pthread.h>		/* for pthread API */

#include <stdint.h>
#include <unistd.h>
#include <mach/vm_statistics.h>
#include <mach/mach_init.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/param.h>

#if defined(__i386__) || defined(__x86_64__)
#define __APPLE_API_PRIVATE
#include <machine/cpu_capabilities.h>
#define _COMM_PAGE_VERSION_REQD 9
#undef __APPLE_API_PRIVATE
#else
#include <sys/sysctl.h>
#endif

#include <libkern/OSAtomic.h>
#include <mach-o/dyld.h>	/* for NSVersionOfLinkTimeLibrary() */
#include <mach-o/dyld_priv.h>	/* for _dyld_get_image_slide() */
#include <crt_externs.h>	/* for _NSGetMachExecuteHeader() */
#include <mach/vm_param.h>
#include <sys/vmparam.h>

#include <CrashReporterClient.h>

/*********************	DEFINITIONS	************************/

#define DEBUG_MALLOC			0 // set to one to debug malloc itself

#define DEBUG_CLIENT			0 // set to one to debug malloc client

#define DEBUG_MADVISE			0

// <rdar://problem/10397726>
#define RELAXED_INVARIANT_CHECKS 1

#if DEBUG_MALLOC
#warning DEBUG_MALLOC ENABLED
# define INLINE
# define ALWAYSINLINE
# define CHECK_MAGAZINE_PTR_LOCKED(szone, mag_ptr, fun)				\
do {										\
    if (__is_threaded && TRY_LOCK(mag_ptr->magazine_lock)) {			\
	malloc_printf("*** magazine_lock was not set %p in %s\n",		\
	mag_ptr->magazine_lock, fun);						\
    }										\
} while (0)
#else
# define INLINE	__inline__
# define ALWAYSINLINE __attribute__((always_inline))
# define CHECK_MAGAZINE_PTR_LOCKED(szone, mag_ptr, fun)	{}
#endif

# define NOINLINE __attribute__((noinline))

#if defined(__i386__) || defined(__x86_64__)
#define CACHE_ALIGN __attribute__ ((aligned (128) )) /* Future-proofing at 128B */
#elif defined(__ppc__) || defined(__ppc64__)
#define CACHE_ALIGN __attribute__ ((aligned (128) ))
#else
#define CACHE_ALIGN
#endif

#if !__LP64__
#define ASLR_INTERNAL			1
#endif

/*
 * Access to global variables is slow, so optimise our handling of vm_page_size
 * and vm_page_shift.
 */
#define _vm_page_size			vm_page_size	/* to get to the originals */
#define _vm_page_shift			vm_page_shift
#define vm_page_size			4096		/* our normal working sizes */
#define vm_page_shift			12

/*
 * msize - a type to refer to the number of quanta of a tiny or small
 * allocation.  A tiny block with an msize of 3 would be 3 << SHIFT_TINY_QUANTUM 
 * bytes in size.
 */
typedef unsigned short msize_t;

typedef union {
    void	*p;
    uintptr_t	u;
} ptr_union;

typedef struct {
    ptr_union	previous;
    ptr_union	next;
} free_list_t;

typedef unsigned int grain_t; // N.B. wide enough to index all free slots

typedef int mag_index_t;

#define CHECK_REGIONS			(1 << 31)
#define DISABLE_ASLR			(1 << 30)

#define MAX_RECORDER_BUFFER		256

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

#define SHIFT_TINY_QUANTUM		4	// Required for AltiVec
#define	TINY_QUANTUM			(1 << SHIFT_TINY_QUANTUM)

#define FOLLOWING_TINY_PTR(ptr,msize)	(((unsigned char *)(ptr)) + ((msize) << SHIFT_TINY_QUANTUM))

#ifdef __LP64__
#define NUM_TINY_SLOTS			64	// number of slots for free-lists
#else
#define NUM_TINY_SLOTS			32	// number of slots for free-lists
#endif

#define NUM_TINY_BLOCKS			64520
#define SHIFT_TINY_CEIL_BLOCKS		16 // ceil(log2(NUM_TINY_BLOCKS))
#define NUM_TINY_CEIL_BLOCKS		(1 << SHIFT_TINY_CEIL_BLOCKS)
#define TINY_BLOCKS_ALIGN		(SHIFT_TINY_CEIL_BLOCKS + SHIFT_TINY_QUANTUM) // 20

#define TINY_ENTROPY_BITS		15
#define TINY_ENTROPY_MASK		((1 << TINY_ENTROPY_BITS) - 1)

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
#define CEIL_NUM_TINY_BLOCKS_WORDS	(((NUM_TINY_BLOCKS + 31) & ~31) >> 5)
#define TINY_METADATA_SIZE		(sizeof(region_trailer_t) + sizeof(tiny_header_inuse_pair_t) * CEIL_NUM_TINY_BLOCKS_WORDS)
#define TINY_REGION_SIZE							\
    ((NUM_TINY_BLOCKS * TINY_QUANTUM + TINY_METADATA_SIZE + vm_page_size - 1) & ~ (vm_page_size - 1))

#define TINY_METADATA_START		(NUM_TINY_BLOCKS * TINY_QUANTUM)

/*
 * Beginning and end pointers for a region's heap.
 */
#define TINY_REGION_ADDRESS(region)	((void *)(region))
#define TINY_REGION_END(region)		((void *)(((uintptr_t)(region)) + (NUM_TINY_BLOCKS * TINY_QUANTUM)))

/*
 * Locate the heap base for a pointer known to be within a tiny region.
 */
#define TINY_REGION_FOR_PTR(_p)		((void *)((uintptr_t)(_p) & ~((1 << TINY_BLOCKS_ALIGN) - 1)))

/*
 * Convert between byte and msize units.
 */
#define TINY_BYTES_FOR_MSIZE(_m)	((_m) << SHIFT_TINY_QUANTUM)
#define TINY_MSIZE_FOR_BYTES(_b)	((_b) >> SHIFT_TINY_QUANTUM)

#ifdef __LP64__
# define TINY_FREE_SIZE(ptr)		(((msize_t *)(ptr))[8])
#else
# define TINY_FREE_SIZE(ptr)		(((msize_t *)(ptr))[4])
#endif
#define TINY_PREVIOUS_MSIZE(ptr)	((msize_t *)(ptr))[-1]

/*
 * Layout of a tiny region
 */
typedef uint32_t tiny_block_t[4]; // assert(TINY_QUANTUM == sizeof(tiny_block_t))

typedef struct tiny_header_inuse_pair
{
    uint32_t	header;
    uint32_t	inuse;
} tiny_header_inuse_pair_t;

typedef struct region_trailer
{
    struct region_trailer	*prev;
    struct region_trailer	*next;
    boolean_t			recirc_suitable;
    boolean_t			failedREUSE;
    volatile int		pinned_to_depot;
    unsigned			bytes_used;
    mag_index_t			mag_index;
} region_trailer_t;

typedef struct tiny_region
{
    tiny_block_t blocks[NUM_TINY_BLOCKS];

    region_trailer_t trailer;
    
    // The interleaved bit arrays comprising the header and inuse bitfields.
    // The unused bits of each component in the last pair will be initialized to sentinel values.
    tiny_header_inuse_pair_t pairs[CEIL_NUM_TINY_BLOCKS_WORDS];
    
    uint8_t pad[TINY_REGION_SIZE - (NUM_TINY_BLOCKS * sizeof(tiny_block_t)) - TINY_METADATA_SIZE];
} *tiny_region_t;

/*
 * Per-region meta data for tiny allocator
 */
#define REGION_TRAILER_FOR_TINY_REGION(r)	(&(((tiny_region_t)(r))->trailer))
#define MAGAZINE_INDEX_FOR_TINY_REGION(r)	(REGION_TRAILER_FOR_TINY_REGION(r)->mag_index)
#define BYTES_USED_FOR_TINY_REGION(r)		(REGION_TRAILER_FOR_TINY_REGION(r)->bytes_used)

/*
 * Locate the block header for a pointer known to be within a tiny region.
 */
#define TINY_BLOCK_HEADER_FOR_PTR(_p)	((void *)&(((tiny_region_t)TINY_REGION_FOR_PTR(_p))->pairs))

/*
 * Locate the inuse map for a given block header pointer.
 */ 
#define TINY_INUSE_FOR_HEADER(_h)	((void *)&(((tiny_header_inuse_pair_t *)(_h))->inuse))

/*
 * Compute the bitmap index for a pointer known to be within a tiny region.
 */
#define TINY_INDEX_FOR_PTR(_p) 		(((uintptr_t)(_p) >> SHIFT_TINY_QUANTUM) & (NUM_TINY_CEIL_BLOCKS - 1))

#define TINY_CACHE			1	// This governs a last-free cache of 1 that bypasses the free-list

#if ! TINY_CACHE
#warning TINY_CACHE turned off
#endif

#define TINY_REGION_PAYLOAD_BYTES	(NUM_TINY_BLOCKS * TINY_QUANTUM)

/*********************	DEFINITIONS for small	************************/

/*
 * Memory in the Small range is allocated from regions (heaps) pointed to by the szone's hashed_regions
 * pointer.
 *
 * Each region is laid out as a heap, followed by the metadata array, all within an 8MB (2^23) block.
 * The array is arranged as an array of shorts, one for each SMALL_QUANTUM in the heap.
 * This means there are 16320 512-blocks and the array is 16320*2 bytes, which totals 8388480, leaving
 * 128 bytes unused.
 *
 * The MSB of each short is set for the first quantum in a free block.  The low 15 bits encode the
 * block size (in SMALL_QUANTUM units), or are zero if the quantum is not the first in a block.
 *
 * The szone maintains an array of 32 freelists, each of which is used to hold free objects
 * of the corresponding quantum size.
 *
 * A free block is laid out as:
 *
 * Offset (32-bit mode)	(64-bit mode)
 * 0x0          0x0      : previous
 * 0x4          0x08     : next
 * 0x8          0x10     : size (in quantum counts)
 * end - 2	end - 2  : size (in quantum counts)
 * end		end
 *
 * All fields are pointer-sized, except for the size which is an unsigned short.
 *
 */

#define SMALL_IS_FREE			(1 << 15)

#define	SHIFT_SMALL_QUANTUM		(SHIFT_TINY_QUANTUM + 5)	// 9
#define	SMALL_QUANTUM			(1 << SHIFT_SMALL_QUANTUM)	// 512 bytes

#define FOLLOWING_SMALL_PTR(ptr,msize)	(((unsigned char *)(ptr)) + ((msize) << SHIFT_SMALL_QUANTUM))

/*
 * The number of slots in the free-list for small blocks.  To avoid going to 
 * vm system as often on large memory machines, increase the number of free list
 * spots above some amount of RAM installed in the system.
 */
#define NUM_SMALL_SLOTS			32
#define NUM_SMALL_SLOTS_LARGEMEM	256
#define SMALL_BITMAP_WORDS		8

/*
 * We can only represent up to 1<<15 for msize; but we choose to stay even below that to avoid the
 * convention msize=0 => msize = (1<<15)
 */
#define NUM_SMALL_BLOCKS		16320
#define SHIFT_SMALL_CEIL_BLOCKS		14 // ceil(log2(NUM_SMALL_BLOCKs))
#define NUM_SMALL_CEIL_BLOCKS		(1 << SHIFT_SMALL_CEIL_BLOCKS)
#define SMALL_BLOCKS_ALIGN		(SHIFT_SMALL_CEIL_BLOCKS + SHIFT_SMALL_QUANTUM) // 23

#define SMALL_ENTROPY_BITS		13
#define SMALL_ENTROPY_MASK		((1 << SMALL_ENTROPY_BITS) - 1)

/*
 * Avoid having so much entropy that the end of a valid small allocation
 * might overrun the end of the small region.
 */
#if SMALL_ENTROPY_MASK + NUM_SMALL_SLOTS > NUM_SMALL_BLOCKS
#error Too many entropy bits for small region requested
#endif

#define SMALL_METADATA_SIZE		(sizeof(region_trailer_t) + NUM_SMALL_BLOCKS * sizeof(msize_t))
#define SMALL_REGION_SIZE						\
    ((NUM_SMALL_BLOCKS * SMALL_QUANTUM + SMALL_METADATA_SIZE + vm_page_size - 1) & ~ (vm_page_size - 1))

#define SMALL_METADATA_START		(NUM_SMALL_BLOCKS * SMALL_QUANTUM)

/*
 * Beginning and end pointers for a region's heap.
 */
#define SMALL_REGION_ADDRESS(region)	((unsigned char *)region)
#define SMALL_REGION_END(region)	(SMALL_REGION_ADDRESS(region) + (NUM_SMALL_BLOCKS * SMALL_QUANTUM))

/*
 * Locate the heap base for a pointer known to be within a small region.
 */
#define SMALL_REGION_FOR_PTR(_p)	((void *)((uintptr_t)(_p) & ~((1 << SMALL_BLOCKS_ALIGN) - 1)))

/*
 * Convert between byte and msize units.
 */
#define SMALL_BYTES_FOR_MSIZE(_m)	((_m) << SHIFT_SMALL_QUANTUM)
#define SMALL_MSIZE_FOR_BYTES(_b)	((_b) >> SHIFT_SMALL_QUANTUM)

#define SMALL_PREVIOUS_MSIZE(ptr)	((msize_t *)(ptr))[-1]

/*
 * Layout of a small region
 */
typedef uint32_t small_block_t[SMALL_QUANTUM/sizeof(uint32_t)];

typedef struct small_region
{
    small_block_t blocks[NUM_SMALL_BLOCKS];

    region_trailer_t trailer;
    
    msize_t small_meta_words[NUM_SMALL_BLOCKS];
    
    uint8_t pad[SMALL_REGION_SIZE - (NUM_SMALL_BLOCKS * sizeof(small_block_t)) - SMALL_METADATA_SIZE];
} *small_region_t;

/*
 * Per-region meta data for small allocator
 */
#define REGION_TRAILER_FOR_SMALL_REGION(r)	(&(((small_region_t)(r))->trailer))
#define MAGAZINE_INDEX_FOR_SMALL_REGION(r)	(REGION_TRAILER_FOR_SMALL_REGION(r)->mag_index)
#define BYTES_USED_FOR_SMALL_REGION(r)		(REGION_TRAILER_FOR_SMALL_REGION(r)->bytes_used)

/*
 * Locate the metadata base for a pointer known to be within a small region.
 */
#define SMALL_META_HEADER_FOR_PTR(_p)	(((small_region_t)SMALL_REGION_FOR_PTR(_p))->small_meta_words)

/*
 * Compute the metadata index for a pointer known to be within a small region.
 */
#define SMALL_META_INDEX_FOR_PTR(_p)	(((uintptr_t)(_p) >> SHIFT_SMALL_QUANTUM) & (NUM_SMALL_CEIL_BLOCKS - 1))

/*
 * Find the metadata word for a pointer known to be within a small region.
 */
#define SMALL_METADATA_FOR_PTR(_p)	(SMALL_META_HEADER_FOR_PTR(_p) + SMALL_META_INDEX_FOR_PTR(_p))

/*
 * Determine whether a pointer known to be within a small region points to memory which is free.
 */
#define SMALL_PTR_IS_FREE(_p)		(*SMALL_METADATA_FOR_PTR(_p) & SMALL_IS_FREE)

/*
 * Extract the msize value for a pointer known to be within a small region.
 */
#define SMALL_PTR_SIZE(_p)		(*SMALL_METADATA_FOR_PTR(_p) & ~SMALL_IS_FREE)

#define SMALL_CACHE			1
#if !SMALL_CACHE
#warning SMALL_CACHE turned off
#endif

#define SMALL_REGION_PAYLOAD_BYTES	(NUM_SMALL_BLOCKS * SMALL_QUANTUM)

/*************************  DEFINITIONS for large  ****************************/

#define LARGE_THRESHOLD			(15 * 1024) // strictly above this use "large"
#define LARGE_THRESHOLD_LARGEMEM	(127 * 1024)

#if (LARGE_THRESHOLD > NUM_SMALL_SLOTS * SMALL_QUANTUM)
#error LARGE_THRESHOLD should always be less than NUM_SMALL_SLOTS * SMALL_QUANTUM
#endif

#if (LARGE_THRESHOLD_LARGEMEM > NUM_SMALL_SLOTS_LARGEMEM * SMALL_QUANTUM)
#error LARGE_THRESHOLD_LARGEMEM should always be less than NUM_SMALL_SLOTS * SMALL_QUANTUM
#endif

/*
 * When all memory is touched after a copy, vm_copy() is always a lose
 * But if the memory is only read, vm_copy() wins over memmove() at 3 or 4 pages
 * (on a G3/300MHz)
 *
 * This must be larger than LARGE_THRESHOLD
 */
#define VM_COPY_THRESHOLD		(40 * 1024)
#define VM_COPY_THRESHOLD_LARGEMEM	(128 * 1024)

typedef struct {
    vm_address_t address;
    vm_size_t size;
    boolean_t did_madvise_reusable;
} large_entry_t;

#if !TARGET_OS_EMBEDDED
#define LARGE_CACHE			1
#else
#define LARGE_CACHE			0
#endif
#if !LARGE_CACHE
#warning LARGE_CACHE turned off
#endif
#if defined(__LP64__)
#define LARGE_ENTRY_CACHE_SIZE 16
#define LARGE_CACHE_SIZE_LIMIT ((vm_size_t)0x80000000) /* 2Gb */
#else
#define LARGE_ENTRY_CACHE_SIZE 8
#define LARGE_CACHE_SIZE_LIMIT ((vm_size_t)0x02000000) /* 32Mb */
#endif
#define LARGE_CACHE_SIZE_ENTRY_LIMIT (LARGE_CACHE_SIZE_LIMIT/LARGE_ENTRY_CACHE_SIZE)

#define SZONE_FLOTSAM_THRESHOLD_LOW  (1024 * 512)
#define SZONE_FLOTSAM_THRESHOLD_HIGH (1024 * 1024)

/*******************************************************************************
 * Definitions for region hash
 ******************************************************************************/

typedef void * region_t;
typedef region_t * rgnhdl_t; /* A pointer into hashed_regions array. */

#define INITIAL_NUM_REGIONS_SHIFT	6 // log2(INITIAL_NUM_REGIONS)
#define INITIAL_NUM_REGIONS		(1 << INITIAL_NUM_REGIONS_SHIFT) // Must be a power of 2!
#define HASHRING_OPEN_ENTRY		((region_t) 0) // Initial value and sentinel marking end of collision chain
#define HASHRING_REGION_DEALLOCATED	((region_t)-1) // Region at this slot reclaimed by OS
#define HASH_BLOCKS_ALIGN		TINY_BLOCKS_ALIGN // MIN( TINY_BLOCKS_ALIGN, SMALL_BLOCKS_ALIGN, ... )

typedef struct region_hash_generation {
    size_t		num_regions_allocated;
    size_t		num_regions_allocated_shift; // log2(num_regions_allocated)
    region_t		*hashed_regions;  // hashed by location
    struct		region_hash_generation *nextgen;
} region_hash_generation_t;

/*******************************************************************************
 * Per-processor magazine for tiny and small allocators
 ******************************************************************************/

typedef struct {			// vm_allocate()'d, so the array of magazines is page-aligned to begin with.
    // Take magazine_lock first,  Depot lock when needed for recirc, then szone->{tiny,small}_regions_lock when needed for alloc
    pthread_lock_t	magazine_lock CACHE_ALIGN;
    // Protection for the crtical section that does allocate_pages outside the magazine_lock
    volatile boolean_t	alloc_underway;
    
    // One element deep "death row", optimizes malloc/free/malloc for identical size.
    void		*mag_last_free; // low SHIFT_{TINY,SMALL}_QUANTUM bits indicate the msize
    region_t		mag_last_free_rgn; // holds the region for mag_last_free

    free_list_t		*mag_free_list[256]; // assert( 256 >= MAX( NUM_TINY_SLOTS, NUM_SMALL_SLOTS_LARGEMEM ))
    unsigned		mag_bitmap[8]; // assert( sizeof(mag_bitmap) << 3 >= sizeof(mag_free_list)/sizeof(free_list_t) )
    
    // the first and last free region in the last block are treated as big blocks in use that are not accounted for
    size_t		mag_bytes_free_at_end;
    size_t		mag_bytes_free_at_start;
    region_t		mag_last_region; // Valid iff mag_bytes_free_at_end || mag_bytes_free_at_start > 0
    
    // bean counting ...
    unsigned		mag_num_objects;
    size_t		mag_num_bytes_in_objects;
    size_t		num_bytes_in_magazine;
    
    // recirculation list -- invariant: all regions owned by this magazine that meet the emptiness criteria 
    // are located nearer to the head of the list than any region that doesn't satisfy that criteria.
    // Doubly linked list for efficient extraction.
    unsigned		recirculation_entries;
    region_trailer_t	*firstNode;
    region_trailer_t	*lastNode;
    
#if __LP64__
    uint64_t		pad[48]; // So sizeof(magazine_t) is 2560 bytes. FIXME: assert this at compile time
#else
    uint32_t		pad[12]; // So sizeof(magazine_t) is 1280 bytes. FIXME: assert this at compile time
#endif
} magazine_t;

#define TINY_MAX_MAGAZINES		32 /* MUST BE A POWER OF 2! */    
#define TINY_MAGAZINE_PAGED_SIZE						\
    (((sizeof(magazine_t) * (TINY_MAX_MAGAZINES + 1)) + vm_page_size - 1) &\
    ~ (vm_page_size - 1)) /* + 1 for the Depot */
						
#define SMALL_MAX_MAGAZINES		32 /* MUST BE A POWER OF 2! */    
#define SMALL_MAGAZINE_PAGED_SIZE						\
    (((sizeof(magazine_t) * (SMALL_MAX_MAGAZINES + 1)) + vm_page_size - 1) &\
    ~ (vm_page_size - 1)) /* + 1 for the Depot */

#define DEPOT_MAGAZINE_INDEX		-1

/****************************** zone itself ***********************************/

/*
 * Note that objects whose adddress are held in pointers here must be pursued 
 * individually in the {tiny,small}_in_use_enumeration() routines. See for
 * example the treatment of region_hash_generation and tiny_magazines below.
 */

typedef struct szone_s {				// vm_allocate()'d, so page-aligned to begin with.
    malloc_zone_t		basic_zone;		// first page will be given read-only protection
    uint8_t			pad[vm_page_size - sizeof(malloc_zone_t)];
    
    pthread_key_t		cpu_id_key;		// remainder of structure is R/W (contains no function pointers)
    unsigned			debug_flags;
    void			*log_address;

    /* Regions for tiny objects */
    pthread_lock_t		tiny_regions_lock CACHE_ALIGN;
    size_t			num_tiny_regions;
    size_t			num_tiny_regions_dealloc;
    region_hash_generation_t	*tiny_region_generation;
    region_hash_generation_t	trg[2];
    
    int				num_tiny_magazines;
    unsigned			num_tiny_magazines_mask;
    int				num_tiny_magazines_mask_shift;
    magazine_t			*tiny_magazines; // array of per-processor magazines

#if TARGET_OS_EMBEDDED
    uintptr_t			last_tiny_advise;
#endif

    /* Regions for small objects */
    pthread_lock_t		small_regions_lock CACHE_ALIGN;
    size_t			num_small_regions;
    size_t			num_small_regions_dealloc;
    region_hash_generation_t	*small_region_generation;
    region_hash_generation_t	srg[2];
    
    unsigned			num_small_slots; // determined by physmem size
    
    int				num_small_magazines;
    unsigned			num_small_magazines_mask;
    int				num_small_magazines_mask_shift;
    magazine_t			*small_magazines; // array of per-processor magazines

#if TARGET_OS_EMBEDDED
    uintptr_t			last_small_advise;
#endif

    /* large objects: all the rest */
    pthread_lock_t		large_szone_lock CACHE_ALIGN; // One customer at a time for large
    unsigned			num_large_objects_in_use;
    unsigned			num_large_entries;
    large_entry_t		*large_entries; // hashed by location; null entries don't count
    size_t			num_bytes_in_large_objects;
    
#if LARGE_CACHE
    int				large_entry_cache_oldest;
    int				large_entry_cache_newest;
    large_entry_t		large_entry_cache[LARGE_ENTRY_CACHE_SIZE]; // "death row" for large malloc/free
    boolean_t			large_legacy_reset_mprotect;
    size_t			large_entry_cache_reserve_bytes;
    size_t			large_entry_cache_reserve_limit;
    size_t			large_entry_cache_bytes; // total size of death row, bytes
#endif

    /* flag and limits pertaining to altered malloc behavior for systems with
       large amounts of physical memory */
    unsigned  is_largemem;
    unsigned  large_threshold;
    unsigned  vm_copy_threshold;

    /* security cookie */
    uintptr_t cookie;

    /* Initial region list */
    region_t			initial_tiny_regions[INITIAL_NUM_REGIONS];
    region_t			initial_small_regions[INITIAL_NUM_REGIONS];
    
	/* The purgeable zone constructed by create_purgeable_zone() would like to hand off tiny and small
	 * allocations to the default scalable zone. Record the latter as the "helper" zone here. */
    struct szone_s		*helper_zone; 

    boolean_t			flotsam_enabled;
} szone_t;

#define SZONE_PAGED_SIZE		((sizeof(szone_t) + vm_page_size - 1) & ~ (vm_page_size - 1))

#if DEBUG_MALLOC || DEBUG_CLIENT
static void		szone_sleep(void);
#endif
__private_extern__ void malloc_error_break(void);

// msg prints after fmt, ...
static NOINLINE void	szone_error(szone_t *szone, int is_corruption, const char *msg, const void *ptr, const char *fmt, ...)
			__printflike(5, 6);

static void		protect(void *address, size_t size, unsigned protection, unsigned debug_flags);
static void		*allocate_pages(szone_t *szone, size_t size, unsigned char align, unsigned debug_flags,
					int vm_page_label);
static void		*allocate_pages_securely(szone_t *szone, size_t size, unsigned char align,
					int vm_page_label);
static void		deallocate_pages(szone_t *szone, void *addr, size_t size, unsigned debug_flags);
#if TARGET_OS_EMBEDDED
static int		madvise_free_range(szone_t *szone, region_t r, uintptr_t pgLo, uintptr_t pgHi, uintptr_t *last);
#else
static int		madvise_free_range(szone_t *szone, region_t r, uintptr_t pgLo, uintptr_t pgHi);
#endif
static kern_return_t	_szone_default_reader(task_t task, vm_address_t address, vm_size_t size, void **ptr);

static INLINE		mag_index_t mag_get_thread_index(szone_t *szone) ALWAYSINLINE;
static magazine_t	*mag_lock_zine_for_region_trailer(szone_t *szone, magazine_t *magazines, region_trailer_t *trailer, 
							  mag_index_t mag_index);
			
static INLINE rgnhdl_t	hash_lookup_region_no_lock(region_t *regions, size_t num_entries, size_t shift, region_t r) 
			ALWAYSINLINE;
static void		hash_region_insert_no_lock(region_t *regions, size_t num_entries, size_t shift, region_t r);
static region_t		*hash_regions_alloc_no_lock(szone_t *szone, size_t num_entries);
static region_t		*hash_regions_grow_no_lock(szone_t *szone, region_t *regions, size_t old_size, 
						   size_t *mutable_shift, size_t *new_size);

static INLINE uintptr_t free_list_gen_checksum(uintptr_t ptr) ALWAYSINLINE;
static INLINE uintptr_t free_list_checksum_ptr(szone_t *szone, void *p) ALWAYSINLINE;
static INLINE void	*free_list_unchecksum_ptr(szone_t *szone, ptr_union *ptr) ALWAYSINLINE;
static unsigned		free_list_count(szone_t *szone, free_list_t *ptr);

static INLINE void	recirc_list_extract(szone_t *szone, magazine_t *mag_ptr, region_trailer_t *node) ALWAYSINLINE;
static INLINE void	recirc_list_splice_last(szone_t *szone, magazine_t *mag_ptr, region_trailer_t *node) ALWAYSINLINE;
static INLINE void	recirc_list_splice_first(szone_t *szone, magazine_t *mag_ptr, region_trailer_t *node) ALWAYSINLINE;

static INLINE void	BITARRAY_SET(uint32_t *bits, msize_t index) ALWAYSINLINE;
static INLINE void	BITARRAY_CLR(uint32_t *bits, msize_t index) ALWAYSINLINE;
static INLINE boolean_t BITARRAY_BIT(uint32_t *bits, msize_t index) ALWAYSINLINE;

static msize_t		get_tiny_free_size(const void *ptr);
static msize_t		get_tiny_previous_free_msize(const void *ptr);
static INLINE msize_t	get_tiny_meta_header(const void *ptr, boolean_t *is_free) ALWAYSINLINE;
static INLINE void	set_tiny_meta_header_in_use(const void *ptr, msize_t msize) ALWAYSINLINE;
static INLINE void	set_tiny_meta_header_in_use_1(const void *ptr) ALWAYSINLINE;
static INLINE void	set_tiny_meta_header_middle(const void *ptr) ALWAYSINLINE;
static INLINE void	set_tiny_meta_header_free(const void *ptr, msize_t msize) ALWAYSINLINE;
static INLINE boolean_t	tiny_meta_header_is_free(const void *ptr) ALWAYSINLINE;
static INLINE void	*tiny_previous_preceding_free(void *ptr, msize_t *prev_msize) ALWAYSINLINE;

static void		tiny_free_list_add_ptr(szone_t *szone, magazine_t *tiny_mag_ptr, void *ptr, msize_t msize);
static void		tiny_free_list_remove_ptr(szone_t *szone, magazine_t *tiny_mag_ptr, void *ptr, msize_t msize);
static INLINE region_t	tiny_region_for_ptr_no_lock(szone_t *szone, const void *ptr) ALWAYSINLINE;

static void		tiny_finalize_region(szone_t *szone, magazine_t *tiny_mag_ptr);
static int		tiny_free_detach_region(szone_t *szone, magazine_t *tiny_mag_ptr, region_t r);
static size_t		tiny_free_reattach_region(szone_t *szone, magazine_t *tiny_mag_ptr, region_t r);
static void		tiny_free_scan_madvise_free(szone_t *szone, magazine_t *depot_ptr, region_t r);
static region_t		tiny_free_try_depot_unmap_no_lock(szone_t *szone, magazine_t *depot_ptr, region_trailer_t *node);
static boolean_t	tiny_free_do_recirc_to_depot(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index);
static region_t		tiny_find_msize_region(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, msize_t msize);
static boolean_t	tiny_get_region_from_depot(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, msize_t msize);

static INLINE boolean_t	tiny_free_no_lock(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, region_t region, 
					  void *ptr, msize_t msize) ALWAYSINLINE;
static void		*tiny_malloc_from_region_no_lock(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, 
			msize_t msize, void *fresh_region);
static boolean_t	tiny_try_realloc_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_size);
static boolean_t	tiny_check_region(szone_t *szone, region_t region);
static kern_return_t	tiny_in_use_enumerator(task_t task, void *context, unsigned type_mask, szone_t *szone, 
					       memory_reader_t reader, vm_range_recorder_t recorder);
static void		*tiny_malloc_from_free_list(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, 
						    msize_t msize);
static INLINE void	*tiny_malloc_should_clear(szone_t *szone, msize_t msize, boolean_t cleared_requested) ALWAYSINLINE;
static INLINE void	free_tiny(szone_t *szone, void *ptr, region_t tiny_region, size_t known_size) ALWAYSINLINE;
static void		print_tiny_free_list(szone_t *szone);
static void		print_tiny_region(boolean_t verbose, region_t region, size_t bytes_at_start, size_t bytes_at_end);
static boolean_t	tiny_free_list_check(szone_t *szone, grain_t slot);

static INLINE void	small_meta_header_set_is_free(msize_t *meta_headers, unsigned index, msize_t msize) ALWAYSINLINE;
static INLINE void	small_meta_header_set_in_use(msize_t *meta_headers, msize_t index, msize_t msize) ALWAYSINLINE;
static INLINE void	small_meta_header_set_middle(msize_t *meta_headers, msize_t index) ALWAYSINLINE;
static void		small_free_list_add_ptr(szone_t *szone, magazine_t *small_mag_ptr, void *ptr, msize_t msize);
static void		small_free_list_remove_ptr(szone_t *szone, magazine_t *small_mag_ptr, void *ptr, msize_t msize);
static INLINE region_t	small_region_for_ptr_no_lock(szone_t *szone, const void *ptr) ALWAYSINLINE;

static void		small_finalize_region(szone_t *szone, magazine_t *small_mag_ptr);
static int		small_free_detach_region(szone_t *szone, magazine_t *small_mag_ptr, region_t r);
static size_t		small_free_reattach_region(szone_t *szone, magazine_t *small_mag_ptr, region_t r);
static void		small_free_scan_madvise_free(szone_t *szone, magazine_t *depot_ptr, region_t r);
static region_t		small_free_try_depot_unmap_no_lock(szone_t *szone, magazine_t *depot_ptr, region_trailer_t *node);
static boolean_t	small_free_do_recirc_to_depot(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index);
static region_t		small_find_msize_region(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, msize_t msize);
static boolean_t	small_get_region_from_depot(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, msize_t msize);
static INLINE boolean_t	small_free_no_lock(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, region_t region, 
					   void *ptr, msize_t msize) ALWAYSINLINE;
static void		*small_malloc_from_region_no_lock(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, 
			msize_t msize, void *fresh_region);
static boolean_t	small_try_realloc_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_size);
static boolean_t	small_check_region(szone_t *szone, region_t region);
static kern_return_t	small_in_use_enumerator(task_t task, void *context, unsigned type_mask, szone_t *szone,
						memory_reader_t reader, vm_range_recorder_t recorder);
static void		*small_malloc_from_free_list(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, 
						     msize_t msize);
static INLINE void	*small_malloc_should_clear(szone_t *szone, msize_t msize, boolean_t cleared_requested) ALWAYSINLINE;
static INLINE void	free_small(szone_t *szone, void *ptr, region_t small_region, size_t known_size) ALWAYSINLINE;
static void		print_small_free_list(szone_t *szone);
static void		print_small_region(szone_t *szone, boolean_t verbose, region_t region, size_t bytes_at_start, size_t bytes_at_end);
static boolean_t	small_free_list_check(szone_t *szone, grain_t grain);

#if DEBUG_MALLOC
static void		large_debug_print(szone_t *szone);
#endif
static large_entry_t	*large_entry_for_pointer_no_lock(szone_t *szone, const void *ptr);
static void		large_entry_insert_no_lock(szone_t *szone, large_entry_t range);
static INLINE void	large_entries_rehash_after_entry_no_lock(szone_t *szone, large_entry_t *entry) ALWAYSINLINE;
static INLINE large_entry_t *large_entries_alloc_no_lock(szone_t *szone, unsigned num) ALWAYSINLINE;
static void		large_entries_free_no_lock(szone_t *szone, large_entry_t *entries, unsigned num, 
						   vm_range_t *range_to_deallocate);
static large_entry_t	*large_entries_grow_no_lock(szone_t *szone, vm_range_t *range_to_deallocate);
static vm_range_t	large_entry_free_no_lock(szone_t *szone, large_entry_t *entry);
static NOINLINE		kern_return_t	large_in_use_enumerator(task_t task, void *context,
								unsigned type_mask, vm_address_t large_entries_address,
								unsigned num_entries, memory_reader_t reader, 
								vm_range_recorder_t recorder);
static void		*large_malloc(szone_t *szone, size_t num_pages, unsigned char alignment, boolean_t cleared_requested);
static NOINLINE void	free_large(szone_t *szone, void *ptr);
static INLINE int	large_try_realloc_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_size) ALWAYSINLINE;

/*
 * Mark these NOINLINE to avoid bloating the purgeable zone call backs
 */
static NOINLINE void	szone_free(szone_t *szone, void *ptr);
static NOINLINE void	*szone_malloc_should_clear(szone_t *szone, size_t size, boolean_t cleared_requested);
static NOINLINE void	*szone_malloc(szone_t *szone, size_t size);
static NOINLINE void	*szone_calloc(szone_t *szone, size_t num_items, size_t size);
static NOINLINE void	*szone_valloc(szone_t *szone, size_t size);
static NOINLINE size_t	szone_size_try_large(szone_t *szone, const void *ptr);
static NOINLINE size_t	szone_size(szone_t *szone, const void *ptr);
static NOINLINE void	*szone_realloc(szone_t *szone, void *ptr, size_t new_size);
static NOINLINE void	*szone_memalign(szone_t *szone, size_t alignment, size_t size);
static NOINLINE void	szone_free_definite_size(szone_t *szone, void *ptr, size_t size);
static NOINLINE unsigned szone_batch_malloc(szone_t *szone, size_t size, void **results, unsigned count);
static NOINLINE void	szone_batch_free(szone_t *szone, void **to_be_freed, unsigned count);
static void		szone_destroy(szone_t *szone);
static NOINLINE size_t	szone_good_size(szone_t *szone, size_t size);

static NOINLINE boolean_t szone_check_all(szone_t *szone, const char *function);
static boolean_t	szone_check(szone_t *szone);
static kern_return_t	szone_ptr_in_use_enumerator(task_t task, void *context, 
						    unsigned type_mask, vm_address_t zone_address,
						    memory_reader_t reader, vm_range_recorder_t recorder);
static NOINLINE void	szone_print(szone_t *szone, boolean_t verbose);
static void		szone_log(malloc_zone_t *zone, void *log_address);
static void		szone_force_lock(szone_t *szone);
static void		szone_force_unlock(szone_t *szone);
static boolean_t	szone_locked(szone_t *szone);

static void		szone_statistics(szone_t *szone, malloc_statistics_t *stats);

static void		purgeable_free(szone_t *szone, void *ptr);
static void		*purgeable_malloc(szone_t *szone, size_t size);
static void		*purgeable_calloc(szone_t *szone, size_t num_items, size_t size);
static void		*purgeable_valloc(szone_t *szone, size_t size);
static size_t		purgeable_size(szone_t *szone, const void *ptr);
static void		*purgeable_realloc(szone_t *szone, void *ptr, size_t new_size);
static void		*purgeable_memalign(szone_t *szone, size_t alignment, size_t size);
static void		purgeable_free_definite_size(szone_t *szone, void *ptr, size_t size);
static unsigned		purgeable_batch_malloc(szone_t *szone, size_t size, void **results, unsigned count);
static void		purgeable_batch_free(szone_t *szone, void **to_be_freed, unsigned count);
static void		purgeable_destroy(szone_t *szone);
static size_t		purgeable_good_size(szone_t *szone, size_t size);

static boolean_t	purgeable_check(szone_t *szone);
static kern_return_t	purgeable_ptr_in_use_enumerator(task_t task, void *context, 
							unsigned type_mask, vm_address_t zone_address,
							memory_reader_t reader, vm_range_recorder_t recorder);
static void		purgeable_print(szone_t *szone, boolean_t verbose);
static void		purgeable_log(malloc_zone_t *zone, void *log_address);
static void		purgeable_force_lock(szone_t *szone);
static void		purgeable_force_unlock(szone_t *szone);
static boolean_t	purgeable_locked(szone_t *szone);

static void		purgeable_statistics(szone_t *szone, malloc_statistics_t *stats);

static void		*frozen_malloc(szone_t *zone, size_t new_size);
static void		*frozen_calloc(szone_t *zone, size_t num_items, size_t size);
static void		*frozen_valloc(szone_t *zone, size_t new_size);
static void		*frozen_realloc(szone_t *zone, void *ptr, size_t new_size);
static void		frozen_free(szone_t *zone, void *ptr);
static void		frozen_destroy(szone_t *zone);

static volatile uintptr_t entropic_address = 0;
static volatile uintptr_t entropic_limit = 0;
#define ENTROPIC_KABILLION 0x10000000 /* 256Mb */

__private_extern__ uint64_t malloc_entropy[2];

#define SZONE_LOCK(szone)						\
    do {								\
	LOCK(szone->large_szone_lock);					\
    } while (0)
	
#define SZONE_UNLOCK(szone)						\
    do {								\
	UNLOCK(szone->large_szone_lock);				\
    } while (0)

#define SZONE_TRY_LOCK(szone)						\
  TRY_LOCK(szone->large_szone_lock);

#define SZONE_MAGAZINE_PTR_LOCK(szone, mag_ptr)				\
    do {								\
	LOCK(mag_ptr->magazine_lock);					\
    } while(0)

#define SZONE_MAGAZINE_PTR_UNLOCK(szone, mag_ptr)			\
    do {								\
	UNLOCK(mag_ptr->magazine_lock);					\
    } while(0)

#define SZONE_MAGAZINE_PTR_TRY_LOCK(szone, mag_ptr)			\
  TRY_LOCK(mag_ptr->magazine_lock);

#if DEBUG_MALLOC
# define LOG(szone,ptr)							\
    (szone->log_address && (((uintptr_t)szone->log_address == -1) ||	\
    (szone->log_address == (void *)(ptr))))
#else
# define LOG(szone,ptr)		0
#endif

#if DEBUG_MALLOC || DEBUG_CLIENT
# define CHECK(szone,fun)						\
    if ((szone)->debug_flags & CHECK_REGIONS)				\
	szone_check_all(szone, fun)
#else
# define CHECK(szone,fun)						\
    do {} while (0)
#endif

/*********************	VERY LOW LEVEL UTILITIES  ************************/

static void
szone_sleep(void)
{
    if (getenv("MallocErrorStop")) {
	_malloc_printf(ASL_LEVEL_NOTICE, "*** sending SIGSTOP to help debug\n");
	kill(getpid(), SIGSTOP);
    } else if (getenv("MallocErrorSleep")) {
	_malloc_printf(ASL_LEVEL_NOTICE, "*** sleeping to help debug\n");
	sleep(3600); // to help debug
    }
}

// msg prints after fmt, ...
static NOINLINE void
szone_error(szone_t *szone, int is_corruption, const char *msg, const void *ptr, const char *fmt, ...)
{
    va_list ap;
    _SIMPLE_STRING b = _simple_salloc();

    if (szone) SZONE_UNLOCK(szone); // FIXME: unlock magazine and region locks?
    if (b) {
	if (fmt) {
	    va_start(ap, fmt);
	    _simple_vsprintf(b, fmt, ap);
	    va_end(ap);
	}
	if (ptr) {
	    _simple_sprintf(b, "*** error for object %p: %s\n", ptr, msg);
	} else {
	    _simple_sprintf(b, "*** error: %s\n", msg);
	}
	malloc_printf("%s*** set a breakpoint in malloc_error_break to debug\n", _simple_string(b));
    } else {
	/*
	 * Should only get here if vm_allocate() can't get a single page of
	 * memory, implying _simple_asl_log() would also fail.  So we just
	 * print to the file descriptor.
	 */
	if (fmt) {
	    va_start(ap, fmt);
	    _malloc_vprintf(MALLOC_PRINTF_NOLOG, fmt, ap);
	    va_end(ap);
	}
	if (ptr) {
	    _malloc_printf(MALLOC_PRINTF_NOLOG, "*** error for object %p: %s\n", ptr, msg);
	} else {
	    _malloc_printf(MALLOC_PRINTF_NOLOG, "*** error: %s\n", msg);
	}
	_malloc_printf(MALLOC_PRINTF_NOLOG, "*** set a breakpoint in malloc_error_break to debug\n");
    }
    malloc_error_break();
#if DEBUG_MALLOC
    szone_print(szone, 1);
#endif
    szone_sleep();
    // Call abort() if this is a memory corruption error and the abort on 
    // corruption flag is set, or if any error should abort.
    if ((is_corruption && (szone->debug_flags & SCALABLE_MALLOC_ABORT_ON_CORRUPTION)) ||
        (szone->debug_flags & SCALABLE_MALLOC_ABORT_ON_ERROR)) {
        CRSetCrashLogMessage(b ? _simple_string(b) : msg);
        abort();
    } else if (b) {
        _simple_sfree(b);
    }
}

static void
protect(void *address, size_t size, unsigned protection, unsigned debug_flags)
{
    kern_return_t	err;

    if (!(debug_flags & SCALABLE_MALLOC_DONT_PROTECT_PRELUDE)) {
	err = mprotect((void *)((uintptr_t)address - vm_page_size), vm_page_size, protection);
	if (err) {
	    malloc_printf("*** can't protect(%p) region for prelude guard page at %p\n",
	      protection,(uintptr_t)address - (1 << vm_page_shift));
	}
    }
    if (!(debug_flags & SCALABLE_MALLOC_DONT_PROTECT_POSTLUDE)) {
	err = mprotect((void *)((uintptr_t)address + size), vm_page_size, protection);
	if (err) {
	    malloc_printf("*** can't protect(%p) region for postlude guard page at %p\n",
	      protection, (uintptr_t)address + size);
	}
    }
}

static void *
allocate_pages(szone_t *szone, size_t size, unsigned char align, unsigned debug_flags, int vm_page_label)
{
    // align specifies a desired alignment (as a log) or 0 if no alignment requested
    void	*vm_addr;
    uintptr_t	addr = 0, aligned_address;
    boolean_t	add_guard_pages = debug_flags & SCALABLE_MALLOC_ADD_GUARD_PAGES;
    boolean_t	purgeable = debug_flags & SCALABLE_MALLOC_PURGEABLE;
    size_t	allocation_size = round_page(size);
    size_t	delta;
    int		alloc_flags = VM_MAKE_TAG(vm_page_label);

    if (align) add_guard_pages = 0; // too cumbersome to deal with that
    if (!allocation_size) allocation_size = 1 << vm_page_shift;
    if (add_guard_pages) allocation_size += 2 * (1 << vm_page_shift);
    if (align) allocation_size += (size_t)1 << align;
    if (purgeable) alloc_flags |= VM_FLAGS_PURGABLE;
    
    if (allocation_size < size) // size_t arithmetic wrapped!
	return NULL;

    vm_addr = mmap(0 /* addr */,
		   allocation_size /* size */,
		   PROT_READ | PROT_WRITE /* prot */,
		   MAP_ANON | MAP_PRIVATE /* flags */,
		   alloc_flags /* fd being used to pass "purgeable" and "vm_page_label" */,
		   0 /* offset */);
    if ((uintptr_t)vm_addr == -1) {
    	szone_error(szone, 0, "can't allocate region", NULL, "*** mmap(size=%lu) failed (error code=%d)\n",
    	allocation_size, errno);
    	return NULL;
    }
    addr = (uintptr_t)vm_addr;

    if (align) {
	aligned_address = (addr + ((uintptr_t)1 << align) - 1) & ~ (((uintptr_t)1 << align) - 1);
	if (aligned_address != addr) {
	    delta = aligned_address - addr;
	    if (munmap((void *)addr, delta) == -1)
		malloc_printf("*** munmap unaligned header failed with %d\n", errno);
	    addr = aligned_address;
	    allocation_size -= delta;
	}
	if (allocation_size > size) {
	    if (munmap((void *)(addr + size), allocation_size - size) == -1)
		malloc_printf("*** munmap unaligned footer failed with %d\n", errno);
	}
    }
    if (add_guard_pages) {
	addr += (uintptr_t)1 << vm_page_shift;
	protect((void *)addr, size, PROT_NONE, debug_flags);
    }
    return (void *)addr;
}

static void *
allocate_pages_securely(szone_t *szone, size_t size, unsigned char align, int vm_page_label)
{
    // align specifies a desired alignment (as a log) or 0 if no alignment requested
    void	*vm_addr;
    uintptr_t	addr, aligned_address;
    size_t	delta, allocation_size = MAX(round_page(size), vm_page_size);
    int		alloc_flags = VM_MAKE_TAG(vm_page_label);

    if (szone->debug_flags & DISABLE_ASLR)
	return allocate_pages(szone, size, align, 0, vm_page_label);

    if (align) 
	allocation_size += (size_t)1 << align;
    
    if (allocation_size < size) // size_t arithmetic wrapped!
	return NULL;

retry:
    vm_addr = mmap((void *)entropic_address /* kernel finds next available range at or above this address */,
		   allocation_size /* size */,
		   PROT_READ | PROT_WRITE /* prot */,
		   MAP_ANON | MAP_PRIVATE /* flags */,
		   alloc_flags /* fd being used to pass "vm_page_label" */,
		   0 /* offset */);
    if (MAP_FAILED == vm_addr) {
	szone_error(szone, 0, "can't allocate region securely", NULL, "*** mmap(size=%lu) failed (error code=%d)\n",
		    size, errno);
	return NULL;
    }
    addr = (uintptr_t)vm_addr;
    
    // Don't allow allocation to rise above entropic_limit (for tidiness).
    if (addr + allocation_size > entropic_limit) { // Exhausted current range?
	uintptr_t t = entropic_address;
	uintptr_t u = t - ENTROPIC_KABILLION;
	
	if (u < t) { // provided we don't wrap, unmap and retry, in the expanded entropic range
	    munmap((void *)addr, allocation_size);
	    (void)__sync_bool_compare_and_swap(&entropic_address, t, u); // Just one reduction please
	    goto retry;
	}
	// fall through to use what we got
    }

    if (addr < entropic_address) { // mmap wrapped to find this allocation, expand the entropic range
	uintptr_t t = entropic_address;
	uintptr_t u = t - ENTROPIC_KABILLION;
	if (u < t)
	    (void)__sync_bool_compare_and_swap(&entropic_address, t, u); // Just one reduction please
	// fall through to use what we got
    }
    
    // unmap any excess address range used for alignment padding
    if (align) {
	aligned_address = (addr + ((uintptr_t)1 << align) - 1) & ~ (((uintptr_t)1 << align) - 1);
	if (aligned_address != addr) {
	    delta = aligned_address - addr;
	    if (munmap((void *)addr, delta) == -1)
		malloc_printf("*** munmap unaligned header failed with %d\n", errno);
	    addr = aligned_address;
	    allocation_size -= delta;
	}
	if (allocation_size > size) {
	    if (munmap((void *)(addr + size), allocation_size - size) == -1)
		malloc_printf("*** munmap unaligned footer failed with %d\n", errno);
	}
    }
    return (void *)addr;
}

static void
deallocate_pages(szone_t *szone, void *addr, size_t size, unsigned debug_flags)
{
    int		err;
    boolean_t	add_guard_pages = debug_flags & SCALABLE_MALLOC_ADD_GUARD_PAGES;

    if (add_guard_pages) {
	addr = (void *)((uintptr_t)addr - (1 << vm_page_shift));
	size += 2 * (1 << vm_page_shift);
    }
    err = munmap(addr, size);
    if ((err == -1) && szone)
	szone_error(szone, 0, "Can't deallocate_pages region", addr, NULL);
}

static int
#if TARGET_OS_EMBEDDED
madvise_free_range(szone_t *szone, region_t r, uintptr_t pgLo, uintptr_t pgHi, uintptr_t *last)
#else
madvise_free_range(szone_t *szone, region_t r, uintptr_t pgLo, uintptr_t pgHi)
#endif
{
    if (pgHi > pgLo) {
	size_t len = pgHi - pgLo;

#if DEBUG_MALLOC
	if (szone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE)
	    memset((void *)pgLo, 0xed, len); // Scribble on MADV_FREEd memory
#endif

#if TARGET_OS_EMBEDDED
	if (last) {
		if (*last == pgLo)
			return 0;

		*last = pgLo;
	}
#endif

	MAGMALLOC_MADVFREEREGION((void *)szone, (void *)r, (void *)pgLo, len); // DTrace USDT Probe
#if TARGET_OS_EMBEDDED
	if (-1 == madvise((void *)pgLo, len, MADV_FREE)) {
#else
	if (-1 == madvise((void *)pgLo, len, MADV_FREE_REUSABLE)) {
#endif
	    /* -1 return: VM map entry change makes this unfit for reuse. Something evil lurks. */
#if DEBUG_MADVISE
	    szone_error(szone, 0, "madvise_free_range madvise(..., MADV_FREE_REUSABLE) failed", 
	        (void *)pgLo, "length=%d\n", len);
#endif
	}
    }
    return 0;
}

static kern_return_t
_szone_default_reader(task_t task, vm_address_t address, vm_size_t size, void **ptr)
{
    *ptr = (void *)address;
    return 0;
}

// Multiplicative hash where the multiplier is a prime near (ULONG_MAX / phi). [phi = 1.618033...]
// pthread_t's are page aligned, (sometimes even in ascending sequence). These hash well.
// See Knuth TAOCP, Vol. 3. 
#if __LP64__
#define HASH_SELF() \
	((((uintptr_t)pthread_self()) >> vm_page_shift) * 11400714819323198549ULL) >> (64 - szone->num_tiny_magazines_mask_shift)
#else
#define HASH_SELF() \
	((((uintptr_t)pthread_self()) >> vm_page_shift) * 2654435761UL) >> (32 - szone->num_tiny_magazines_mask_shift)
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
/*
 * These commpage routines provide fast access to the logical cpu number
 * of the calling processor assuming no pre-emption occurs. 
 */

static INLINE mag_index_t
mag_get_thread_index(szone_t *szone)
{
    if (!__is_threaded)
	return 0;
    else
	return cpu_number() & (TINY_MAX_MAGAZINES - 1);
}

#else
#warning deriving magazine index from pthread_self() [want processor number]

static INLINE mag_index_t
mag_get_thread_index(szone_t *szone)
{
    if (!__is_threaded)
	return 0;
    else if ((pthread_key_t) -1 == szone->cpu_id_key) { // In case pthread_key_create() failed.
	return HASH_SELF();
    } else {
	mag_index_t idx = (mag_index_t)(intptr_t)pthread_getspecific(szone->cpu_id_key);
	
	// Has this thread been hinted with a non-zero value [i.e. 1 + cpuid()] ?
	// If so, bump down the hint to a zero-based magazine index and return it.
	if (idx) {
	    return idx - 1;
	} else {
	    // No hint available. Contruct a magazine index for this thread ...
	    idx = HASH_SELF();

	    // bump up the hint to exclude zero and try to memorize it ...
	    pthread_setspecific(szone->cpu_id_key, (const void *)((uintptr_t)idx + 1));
	    
	    // and return the (zero-based) magazine index.
	    return idx;
	}
    }
}
#endif

static magazine_t *
mag_lock_zine_for_region_trailer(szone_t *szone, magazine_t *magazines, region_trailer_t *trailer, mag_index_t mag_index) 
{
    mag_index_t refreshed_index;
    magazine_t *mag_ptr = &(magazines[mag_index]);

    // Take the lock  on entry.
    SZONE_MAGAZINE_PTR_LOCK(szone, mag_ptr);
    
    // Now in the time it took to acquire the lock, the region may have migrated
    // from one magazine to another. In which case the magazine lock we obtained 
    // (namely magazines[mag_index].mag_lock) is stale. If so, keep on tryin' ...
    while (mag_index != (refreshed_index = trailer->mag_index)) { // Note assignment
    
	SZONE_MAGAZINE_PTR_UNLOCK(szone, mag_ptr);
	
	mag_index = refreshed_index;
	mag_ptr = &(magazines[mag_index]);
	SZONE_MAGAZINE_PTR_LOCK(szone, mag_ptr);
    }
    
    return mag_ptr;
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
static INLINE rgnhdl_t
hash_lookup_region_no_lock(region_t *regions, size_t num_entries, size_t shift, region_t r) {
    size_t	index, hash_index;
    rgnhdl_t	entry;

    if (!num_entries)
	return 0;

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
	if (*entry == 0)
	    return 0;
	if (*entry == r)
	    return entry;
	if (++index == num_entries)
	    index = 0;
    } while (index != hash_index);
    return 0;
}

/*
 * hash_region_insert_no_lock - Insert a region into the hash ring.
 */
static void
hash_region_insert_no_lock(region_t *regions, size_t num_entries, size_t shift, region_t r) {
    size_t	index, hash_index;
    rgnhdl_t	entry;

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
	if (++index == num_entries)
	    index = 0;
    } while (index != hash_index);
}

/*
 * hash_regions_alloc_no_lock - Allocate space for a number of entries.  This
 * must be a VM allocation as to avoid recursing between allocating a new small
 * region, and asking the small region to allocate space for the new list of
 * regions.
 */
static region_t *
hash_regions_alloc_no_lock(szone_t *szone, size_t num_entries)
{
    size_t	size = num_entries * sizeof(region_t);
    
    return allocate_pages(szone, round_page(size), 0, 0, VM_MEMORY_MALLOC);
}

/*
 * hash_regions_grow_no_lock - Grow the hash ring, and rehash the entries.
 * Return the new region and new size to update the szone.  Do not deallocate
 * the old entries since someone may still be allocating them.
 */
static region_t *
hash_regions_grow_no_lock(szone_t *szone, region_t *regions, size_t old_size, size_t *mutable_shift,
                          size_t *new_size)
{
    // double in size and allocate memory for the regions
    *new_size = old_size + old_size;
    *mutable_shift = *mutable_shift + 1;
    region_t *new_regions = hash_regions_alloc_no_lock(szone, *new_size);

    // rehash the entries into the new list
    size_t index;
    for (index = 0; index < old_size; ++index) {
	region_t r = regions[index];
	if (r != HASHRING_OPEN_ENTRY && r != HASHRING_REGION_DEALLOCATED)
	    hash_region_insert_no_lock(new_regions, *new_size, *mutable_shift, r);
    }
    return new_regions;
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

static NOINLINE void
free_list_checksum_botch(szone_t *szone, free_list_t *ptr)
{
    szone_error(szone, 1, "incorrect checksum for freed object "
    "- object was probably modified after being freed.", ptr, NULL);
}

static INLINE uintptr_t free_list_gen_checksum(uintptr_t ptr)
{
    uint8_t chk;

    chk  = (unsigned char)(ptr >>  0);
    chk += (unsigned char)(ptr >>  8);
    chk += (unsigned char)(ptr >> 16);
    chk += (unsigned char)(ptr >> 24);
#if  __LP64__
    chk += (unsigned char)(ptr >> 32);
    chk += (unsigned char)(ptr >> 40);
    chk += (unsigned char)(ptr >> 48);
    chk += (unsigned char)(ptr >> 56);
#endif

    return chk & (uintptr_t)0xF;
}

#define NYBBLE 4
#if  __LP64__
#define ANTI_NYBBLE (64 - NYBBLE)
#else
#define ANTI_NYBBLE (32 - NYBBLE)
#endif

static INLINE uintptr_t
free_list_checksum_ptr(szone_t *szone, void *ptr) 
{
    uintptr_t p = (uintptr_t)ptr;
    return (p >> NYBBLE) | (free_list_gen_checksum(p ^ szone->cookie) << ANTI_NYBBLE); // compiles to rotate instruction
}

static INLINE void *
free_list_unchecksum_ptr(szone_t *szone, ptr_union *ptr)
{
    ptr_union p;
    uintptr_t t = ptr->u;

    t = (t << NYBBLE) | (t >> ANTI_NYBBLE); // compiles to rotate instruction
    p.u = t &  ~(uintptr_t)0xF;

    if ((t & (uintptr_t)0xF) != free_list_gen_checksum(p.u ^ szone->cookie))
    {
      free_list_checksum_botch(szone, (free_list_t *)ptr);
      return NULL;
    }
    return p.p;
}

#undef ANTI_NYBBLE
#undef NYBBLE

static unsigned
free_list_count(szone_t *szone, free_list_t *ptr)
{
    unsigned	count = 0;

    while (ptr) {
        count++;
        ptr = free_list_unchecksum_ptr(szone, &ptr->next);
    }
    return count;
}

static INLINE void
recirc_list_extract(szone_t *szone, magazine_t *mag_ptr, region_trailer_t *node)
{
    // excise node from list
    if (NULL == node->prev)
	mag_ptr->firstNode = node->next;
    else
	node->prev->next = node->next;
    
    if (NULL == node->next)
	mag_ptr->lastNode = node->prev;
    else
	node->next->prev = node->prev;
	
    mag_ptr->recirculation_entries--;
}

static INLINE void
recirc_list_splice_last(szone_t *szone, magazine_t *mag_ptr, region_trailer_t *node)
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

static INLINE void
recirc_list_splice_first(szone_t *szone, magazine_t *mag_ptr, region_trailer_t *node)
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

/* Macros used to manipulate the uint32_t quantity mag_bitmap. */

/* BITMAPV variants are used by tiny. */
#if defined(__LP64__)
// assert(NUM_SLOTS == 64) in which case (slot >> 5) is either 0 or 1
#define BITMAPV_SET(bitmap,slot) 	(bitmap[(slot) >> 5] |= 1 << ((slot) & 31))
#define BITMAPV_CLR(bitmap,slot) 	(bitmap[(slot) >> 5] &= ~ (1 << ((slot) & 31)))
#define BITMAPV_BIT(bitmap,slot)	((bitmap[(slot) >> 5] >> ((slot) & 31)) & 1)
#define BITMAPV_CTZ(bitmap)		(__builtin_ctzl(bitmap))	
#else
// assert(NUM_SLOTS == 32) in which case (slot >> 5) is always 0, so code it that way
#define BITMAPV_SET(bitmap,slot) 	(bitmap[0] |= 1 << (slot))
#define BITMAPV_CLR(bitmap,slot)	(bitmap[0] &= ~ (1 << (slot)))
#define BITMAPV_BIT(bitmap,slot) 	((bitmap[0] >> (slot)) & 1)
#define BITMAPV_CTZ(bitmap)		(__builtin_ctz(bitmap))
#endif

/* BITMAPN is used by small. (slot >> 5) takes on values from 0 to 7. */
#define BITMAPN_SET(bitmap,slot) 	(bitmap[(slot) >> 5] |= 1 << ((slot) & 31))
#define BITMAPN_CLR(bitmap,slot) 	(bitmap[(slot) >> 5] &= ~ (1 << ((slot) & 31)))
#define BITMAPN_BIT(bitmap,slot)	((bitmap[(slot) >> 5] >> ((slot) & 31)) & 1)

/* returns bit # of least-significant one bit, starting at 0 (undefined if !bitmap) */
#define BITMAP32_CTZ(bitmap)		(__builtin_ctz(bitmap[0]))

/*********************	TINY FREE LIST UTILITIES	************************/

// We encode the meta-headers as follows:
// Each quantum has an associated set of 2 bits:
// block_header when 1 says this block is the beginning of a block
// in_use when 1 says this block is in use
// so a block in use of size 3 is 1-1 0-X 0-X
// for a free block TINY_FREE_SIZE(ptr) carries the size and the bits are 1-0 X-X X-X
// for a block middle the bits are 0-0

// We store the meta-header bit arrays by interleaving them 32 bits at a time.
// Initial 32 bits of block_header, followed by initial 32 bits of in_use, followed
// by next 32 bits of block_header, followed by next 32 bits of in_use, etc.
// This localizes memory references thereby reducing cache and TLB pressures.

static INLINE void
BITARRAY_SET(uint32_t *bits, msize_t index)
{
    // index >> 5 identifies the uint32_t to manipulate in the conceptually contiguous bits array
    // (index >> 5) << 1 identifies the uint32_t allowing for the actual interleaving
    bits[(index >> 5) << 1] |= (1 << (index & 31));
}

static INLINE void
BITARRAY_CLR(uint32_t *bits, msize_t index)
{
    bits[(index >> 5) << 1] &= ~(1 << (index & 31));
}

static INLINE boolean_t
BITARRAY_BIT(uint32_t *bits, msize_t index)
{
    return ((bits[(index >> 5) << 1]) >> (index & 31)) & 1;
}

#if 0
static INLINE void	bitarray_mclr(uint32_t *bits, unsigned start, unsigned end) ALWAYSINLINE;

static INLINE void
bitarray_mclr(uint32_t *bits, unsigned start, unsigned end) 
{
    // start >> 5 identifies the uint32_t to manipulate in the conceptually contiguous bits array
    // (start >> 5) << 1 identifies the uint32_t allowing for the actual interleaving
    uint32_t	*addr = bits + ((start >> 5) << 1);
    
    uint32_t	span = end - start;
    start = start & 31;
    end = start + span;
    
    if (end > 31) {
	addr[0] &= (0xFFFFFFFFU >> (31 - start)) >> 1;
	addr[2] &= (0xFFFFFFFFU << (end - 32));
    } else {
	unsigned mask = (0xFFFFFFFFU >> (31 - start)) >> 1;
	mask |= (0xFFFFFFFFU << end);
	addr[0] &= mask;
    }
}
#endif

/*
 * Obtain the size of a free tiny block (in msize_t units).
 */
static msize_t
get_tiny_free_size(const void *ptr)
{
    void	*next_block = (void *)((uintptr_t)ptr + TINY_QUANTUM);
    void	*region_end = TINY_REGION_END(TINY_REGION_FOR_PTR(ptr));

    // check whether the next block is outside the tiny region or a block header
    // if so, then the size of this block is one, and there is no stored size.
    if (next_block < region_end)
    {
        uint32_t	*next_header = TINY_BLOCK_HEADER_FOR_PTR(next_block);
        msize_t		next_index  = TINY_INDEX_FOR_PTR(next_block);
        
        if (!BITARRAY_BIT(next_header, next_index))
            return TINY_FREE_SIZE(ptr);
    }
    return 1;
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
    if (ptr != TINY_REGION_FOR_PTR(ptr))
    {
        void		*prev_block = (void *)((uintptr_t)ptr - TINY_QUANTUM);
        uint32_t	*prev_header = TINY_BLOCK_HEADER_FOR_PTR(prev_block);
        msize_t		prev_index  = TINY_INDEX_FOR_PTR(prev_block);
        if (BITARRAY_BIT(prev_header, prev_index))
            return 1;
        return TINY_PREVIOUS_MSIZE(ptr);
    }
    // don't read possibly unmapped memory before the beginning of the region
    return 0;
}

static INLINE msize_t
get_tiny_meta_header(const void *ptr, boolean_t *is_free)
{
    // returns msize and is_free
    // may return 0 for the msize component (meaning 65536)
    uint32_t	*block_header;
    msize_t	index;

    block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
    index = TINY_INDEX_FOR_PTR(ptr);

    msize_t midx = (index >> 5) << 1;
    uint32_t mask = 1 << (index & 31);
    *is_free = 0;
    if (0 == (block_header[midx] & mask)) // if (!BITARRAY_BIT(block_header, index))
	return 0;
    if (0 == (block_header[midx + 1] & mask)) { // if (!BITARRAY_BIT(in_use, index))
        *is_free = 1;
        return get_tiny_free_size(ptr);
    }
    
    // index >> 5 identifies the uint32_t to manipulate in the conceptually contiguous bits array
    // (index >> 5) << 1 identifies the uint32_t allowing for the actual interleaving
#if defined(__LP64__)
    // The return value, msize, is computed as the distance to the next 1 bit in block_header.
    // That's guaranteed to be somewhwere in the next 64 bits. And those bits could span three
    // uint32_t block_header elements. Collect the bits into a single uint64_t and measure up with ffsl.
    uint32_t	*addr = ((uint32_t *)block_header) + ((index >> 5) << 1);
    uint32_t	bitidx = index & 31;
    uint64_t	word_lo = addr[0];
    uint64_t	word_mid = addr[2];
    uint64_t	word_hi = addr[4];
    uint64_t	word_lomid = (word_lo >> bitidx) | (word_mid << (32 - bitidx));
    uint64_t	word = bitidx ? word_lomid | (word_hi << (64 - bitidx)) : word_lomid;
    uint32_t	result = __builtin_ffsl(word >> 1);
#else
    // The return value, msize, is computed as the distance to the next 1 bit in block_header.
    // That's guaranteed to be somwhwere in the next 32 bits. And those bits could span two
    // uint32_t block_header elements. Collect the bits into a single uint32_t and measure up with ffs.
    uint32_t	*addr = ((uint32_t *)block_header) + ((index >> 5) << 1);
    uint32_t	bitidx = index & 31;
    uint32_t	word = bitidx ? (addr[0] >> bitidx) | (addr[2] << (32 - bitidx)) : addr[0];
    uint32_t	result = __builtin_ffs(word >> 1);
#endif
    return result;
}

static INLINE void
set_tiny_meta_header_in_use(const void *ptr, msize_t msize)
{
    uint32_t	*block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
    msize_t	index = TINY_INDEX_FOR_PTR(ptr);
    msize_t	clr_msize = msize - 1;
    msize_t	midx = (index >> 5) << 1;
    uint32_t	val = (1 << (index & 31));
    
#if DEBUG_MALLOC
    if (msize >= NUM_TINY_SLOTS)
	malloc_printf("set_tiny_meta_header_in_use() invariant broken %p %d\n", ptr, msize);
    if ((unsigned)index + (unsigned)msize > 0x10000)
	malloc_printf("set_tiny_meta_header_in_use() invariant broken (2) %p %d\n", ptr, msize);
#endif

    block_header[midx] |= val; // BITARRAY_SET(block_header, index);
    block_header[midx + 1] |= val; // BITARRAY_SET(in_use, index);

    // bitarray_mclr(block_header, index, end_bit);
    // bitarray_mclr(in_use, index, end_bit);

    index++;
    midx = (index >> 5) << 1;
    
    unsigned	start = index & 31;
    unsigned	end = start + clr_msize;

#if defined(__LP64__)
    if (end > 63) {
	unsigned mask0 = (0xFFFFFFFFU >> (31 - start)) >> 1;
	unsigned mask1 = (0xFFFFFFFFU << (end - 64));
	block_header[midx + 0] &= mask0; // clear header
	block_header[midx + 1] &= mask0; // clear in_use
	block_header[midx + 2] = 0; // clear header
	block_header[midx + 3] = 0; // clear in_use
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
	msize_t	mf;
	
	mf = get_tiny_meta_header(ptr, &ff);
	if (msize != mf) {
	    malloc_printf("setting header for tiny in_use %p : %d\n", ptr, msize);
	    malloc_printf("reading header for tiny %p : %d %d\n", ptr, mf, ff);
	}
    }
#endif
}

static INLINE void
set_tiny_meta_header_in_use_1(const void *ptr) // As above with msize == 1
{
    uint32_t	*block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
    msize_t	index = TINY_INDEX_FOR_PTR(ptr);
    msize_t	midx = (index >> 5) << 1;
    uint32_t	val = (1 << (index & 31));
    
    block_header[midx] |= val; // BITARRAY_SET(block_header, index);
    block_header[midx + 1] |= val; // BITARRAY_SET(in_use, index);

    index++;
    midx = (index >> 5) << 1;
    val = (1 << (index & 31));
       
    block_header[midx] |= val; // BITARRAY_SET(block_header, (index+clr_msize))
}

static INLINE void
set_tiny_meta_header_middle(const void *ptr)
{
    // indicates this block is in the middle of an in use block
    uint32_t	*block_header;
    uint32_t	*in_use;
    msize_t	index;

    block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
    in_use = TINY_INUSE_FOR_HEADER(block_header);
    index = TINY_INDEX_FOR_PTR(ptr);

    BITARRAY_CLR(block_header, index);
    BITARRAY_CLR(in_use, index); 
}

static INLINE void
set_tiny_meta_header_free(const void *ptr, msize_t msize)
{
    // !msize is acceptable and means 65536
    uint32_t	*block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
    msize_t	index = TINY_INDEX_FOR_PTR(ptr);
    msize_t	midx = (index >> 5) << 1;
    uint32_t	val = (1 << (index & 31));

#if DEBUG_MALLOC
    if ((unsigned)index + (unsigned)msize > 0x10000) {
	malloc_printf("setting header for tiny free %p msize too large: %d\n", ptr, msize);
    }
#endif

    block_header[midx] |= val; // BITARRAY_SET(block_header, index);
    block_header[midx + 1] &= ~val; // BITARRAY_CLR(in_use, index); 
    
    // mark the end of this block if msize is > 1.  For msize == 0, the whole
    // region is free, so there is no following block. For msize == 1, there is
    // no space to write the size on 64 bit systems.  The size for 1 quantum
    // blocks is computed from the metadata bitmaps.
    if (msize > 1) {
        void	*follower = FOLLOWING_TINY_PTR(ptr, msize);
        TINY_PREVIOUS_MSIZE(follower) = msize;
        TINY_FREE_SIZE(ptr) = msize;
    }
    if (msize == 0) {
        TINY_FREE_SIZE(ptr) = msize;
    }
#if DEBUG_MALLOC
    boolean_t	ff;
    msize_t	mf = get_tiny_meta_header(ptr, &ff);
    if ((msize != mf) || !ff) {
	malloc_printf("setting header for tiny free %p : %u\n", ptr, msize);
	malloc_printf("reading header for tiny %p : %u %u\n", ptr, mf, ff);
    }
#endif
}

static INLINE boolean_t
tiny_meta_header_is_free(const void *ptr)
{
    uint32_t	*block_header;
    uint32_t	*in_use;
    msize_t	index;

    block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
    in_use = TINY_INUSE_FOR_HEADER(block_header);
    index = TINY_INDEX_FOR_PTR(ptr);
    if (!BITARRAY_BIT(block_header, index))
	return 0;
    return !BITARRAY_BIT(in_use, index);
}

static INLINE void *
tiny_previous_preceding_free(void *ptr, msize_t *prev_msize)
{
    // returns the previous block, assuming and verifying it's free
    uint32_t	*block_header;
    uint32_t	*in_use;
    msize_t	index;
    msize_t	previous_msize;
    msize_t	previous_index;
    void	*previous_ptr;

    block_header = TINY_BLOCK_HEADER_FOR_PTR(ptr);
    in_use = TINY_INUSE_FOR_HEADER(block_header);
    index = TINY_INDEX_FOR_PTR(ptr);
    
    if (!index)
        return NULL;
    if ((previous_msize = get_tiny_previous_free_msize(ptr)) > index)
        return NULL;
    
    previous_index = index - previous_msize;
    previous_ptr = (void *)((uintptr_t)TINY_REGION_FOR_PTR(ptr) + TINY_BYTES_FOR_MSIZE(previous_index));
    if (!BITARRAY_BIT(block_header, previous_index))
        return NULL;
    if (BITARRAY_BIT(in_use, previous_index))
        return NULL;
    if (get_tiny_free_size(previous_ptr) != previous_msize)
        return NULL;
    
    // conservative check did match true check
    *prev_msize = previous_msize;
    return previous_ptr;
}

/* 
 * Adds an item to the proper free list, and also marks the meta-header of the 
 * block properly.
 * Assumes szone has been locked
 */ 
static void
tiny_free_list_add_ptr(szone_t *szone, magazine_t *tiny_mag_ptr, void *ptr, msize_t msize)
{
    grain_t	slot = (!msize || (msize >= NUM_TINY_SLOTS)) ? NUM_TINY_SLOTS - 1 : msize - 1;
    free_list_t	*free_ptr = ptr;
    free_list_t	*free_head = tiny_mag_ptr->mag_free_list[slot];

#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
        malloc_printf("in %s, ptr=%p, msize=%d\n", __FUNCTION__, ptr, msize);
    }
    if (((uintptr_t)ptr) & (TINY_QUANTUM - 1)) {
        szone_error(szone, 1, "tiny_free_list_add_ptr: Unaligned ptr", ptr, NULL);
    }
#endif
    set_tiny_meta_header_free(ptr, msize);
    if (free_head) {
#if DEBUG_MALLOC
        if (free_list_unchecksum_ptr(szone, &free_head->previous)) {
            szone_error(szone, 1, "tiny_free_list_add_ptr: Internal invariant broken (free_head->previous)", ptr,
            "ptr=%p slot=%d free_head=%p previous=%p\n", ptr, slot, (void *)free_head, free_head->previous.p);
        }
        if (! tiny_meta_header_is_free(free_head)) {
            szone_error(szone, 1, "tiny_free_list_add_ptr: Internal invariant broken (free_head is not a free pointer)", ptr,
            "ptr=%p slot=%d free_head=%p\n", ptr, slot, (void *)free_head);
        }
#endif
        free_head->previous.u = free_list_checksum_ptr(szone, free_ptr);
    } else {
        BITMAPV_SET(tiny_mag_ptr->mag_bitmap, slot);
    }
    free_ptr->previous.u = free_list_checksum_ptr(szone, NULL);
    free_ptr->next.u = free_list_checksum_ptr(szone, free_head);

    tiny_mag_ptr->mag_free_list[slot] = free_ptr;
}

/* 
 * Removes the item pointed to by ptr in the proper free list. 
 * Assumes szone has been locked
 */ 
static void
tiny_free_list_remove_ptr(szone_t *szone, magazine_t *tiny_mag_ptr, void *ptr, msize_t msize)
{
    grain_t	slot = (!msize || (msize >= NUM_TINY_SLOTS)) ? NUM_TINY_SLOTS - 1 : msize - 1;
    free_list_t	*free_ptr = ptr, *next, *previous;
    
    next = free_list_unchecksum_ptr(szone, &free_ptr->next);
    previous = free_list_unchecksum_ptr(szone, &free_ptr->previous);

#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
        malloc_printf("In %s, ptr=%p, msize=%d\n", __FUNCTION__, ptr, msize);
    }
#endif
    if (!previous) { 
    // The block to remove is the head of the free list
#if DEBUG_MALLOC
        if (tiny_mag_ptr->mag_free_list[slot] != ptr) {
            szone_error(szone, 1, "tiny_free_list_remove_ptr: Internal invariant broken (tiny_mag_ptr->mag_free_list[slot])", ptr,
              "ptr=%p slot=%d msize=%d tiny_mag_ptr->mag_free_list[slot]=%p\n",
              ptr, slot, msize, (void *)tiny_mag_ptr->mag_free_list[slot]);
            return;
        }
#endif
        tiny_mag_ptr->mag_free_list[slot] = next;
        if (!next) BITMAPV_CLR(tiny_mag_ptr->mag_bitmap, slot);
    } else {
        // We know free_ptr is already checksummed, so we don't need to do it
        // again.
        previous->next = free_ptr->next;
    }
    if (next) {
        // We know free_ptr is already checksummed, so we don't need to do it
        // again.
        next->previous = free_ptr->previous;
    }
}

/*
 * tiny_region_for_ptr_no_lock - Returns the tiny region containing the pointer,
 * or NULL if not found.
 */
static INLINE region_t
tiny_region_for_ptr_no_lock(szone_t *szone, const void *ptr)
{
    rgnhdl_t r = hash_lookup_region_no_lock(szone->tiny_region_generation->hashed_regions,
	szone->tiny_region_generation->num_regions_allocated,
	szone->tiny_region_generation->num_regions_allocated_shift,
	TINY_REGION_FOR_PTR(ptr));
    return r ? *r : r;
}

static void
tiny_finalize_region(szone_t *szone, magazine_t *tiny_mag_ptr) {
    void      *last_block, *previous_block;
    uint32_t  *last_header;
    msize_t   last_msize, previous_msize, last_index;
       
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
    last_block = (void *)
        ((uintptr_t)TINY_REGION_END(tiny_mag_ptr->mag_last_region) - tiny_mag_ptr->mag_bytes_free_at_end);
    last_msize = TINY_MSIZE_FOR_BYTES(tiny_mag_ptr->mag_bytes_free_at_end);
    last_header = TINY_BLOCK_HEADER_FOR_PTR(last_block);
    last_index  = TINY_INDEX_FOR_PTR(last_block);

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
    if (last_index != (NUM_TINY_BLOCKS - 1))
        BITARRAY_CLR(last_header, (last_index + 1));

    previous_block = tiny_previous_preceding_free(last_block, &previous_msize);
    if (previous_block) {
        set_tiny_meta_header_middle(last_block);
        tiny_free_list_remove_ptr(szone, tiny_mag_ptr, previous_block, previous_msize);
        last_block = previous_block;
        last_msize += previous_msize;
    }

    // splice last_block into the free list
    tiny_free_list_add_ptr(szone, tiny_mag_ptr, last_block, last_msize); 
    tiny_mag_ptr->mag_bytes_free_at_end = 0;
    }

#if ASLR_INTERNAL
    // Coalesce the big free block at start with any following free blocks
    if (tiny_mag_ptr->mag_bytes_free_at_start) {
        last_block = TINY_REGION_ADDRESS(tiny_mag_ptr->mag_last_region);
        last_msize = TINY_MSIZE_FOR_BYTES(tiny_mag_ptr->mag_bytes_free_at_start);

        void *next_block = (void *) ((uintptr_t)last_block + tiny_mag_ptr->mag_bytes_free_at_start);

        // clear the in use bit we were using to mark the end of the big start block
        set_tiny_meta_header_middle((uintptr_t)next_block - TINY_QUANTUM);

        // Coalesce the big start block with any following free blocks
        if (tiny_meta_header_is_free(next_block)) {
            msize_t next_msize = get_tiny_free_size(next_block);
            set_tiny_meta_header_middle(next_block);
            tiny_free_list_remove_ptr(szone, tiny_mag_ptr, next_block, next_msize);
            last_msize += next_msize;
        }

        // splice last_block into the free list
        tiny_free_list_add_ptr(szone, tiny_mag_ptr, last_block, last_msize); 
        tiny_mag_ptr->mag_bytes_free_at_start = 0;
    }
#endif

    tiny_mag_ptr->mag_last_region = NULL;
}

static int
tiny_free_detach_region(szone_t *szone, magazine_t *tiny_mag_ptr, region_t r) {
    uintptr_t	start = (uintptr_t)TINY_REGION_ADDRESS(r);
    uintptr_t	current = start;
    uintptr_t	limit =  (uintptr_t)TINY_REGION_END(r);
    boolean_t	is_free;
    msize_t	msize;
    int		total_alloc = 0;

    while (current < limit) {
	msize = get_tiny_meta_header((void *)current, &is_free);
	if (is_free && !msize && (current == start)) {
	    // first block is all free
	    break;
	}
	if (!msize) {
#if DEBUG_MALLOC
	    malloc_printf("*** tiny_free_detach_region error with %p: msize=%d is_free =%d\n", 
	    (void *)current, msize, is_free);
#endif
	    break;
	}
	if (is_free) {
	    tiny_free_list_remove_ptr(szone, tiny_mag_ptr, (void *)current, msize);
	} else {
	    total_alloc++;
	}
	current += TINY_BYTES_FOR_MSIZE(msize);
    }
    return total_alloc;
}

static size_t
tiny_free_reattach_region(szone_t *szone, magazine_t *tiny_mag_ptr, region_t r) {
    uintptr_t	start = (uintptr_t)TINY_REGION_ADDRESS(r);
    uintptr_t	current = start;
    uintptr_t	limit =  (uintptr_t)TINY_REGION_END(r);
    boolean_t	is_free;
    msize_t	msize;
    size_t	total_alloc = 0;

    while (current < limit) {
	msize = get_tiny_meta_header((void *)current, &is_free);
	if (is_free && !msize && (current == start)) {
	    // first block is all free
	    break;
	}
	if (!msize) {
#if DEBUG_MALLOC
	    malloc_printf("*** tiny_free_reattach_region error with %p: msize=%d is_free =%d\n", 
	    (void *)current, msize, is_free);
#endif
	    break;
	}
	if (is_free) {
	    tiny_free_list_add_ptr(szone, tiny_mag_ptr, (void *)current, msize);
        } else {
	    total_alloc += TINY_BYTES_FOR_MSIZE(msize);
	}
	current += TINY_BYTES_FOR_MSIZE(msize);
    }
    return total_alloc;
}

typedef struct {
    uint8_t pnum, size;
} tiny_pg_pair_t;

static void NOINLINE /* want private stack frame for automatic array */
tiny_free_scan_madvise_free(szone_t *szone, magazine_t *depot_ptr, region_t r) {
    uintptr_t	start = (uintptr_t)TINY_REGION_ADDRESS(r);
    uintptr_t	current = start;
    uintptr_t	limit =  (uintptr_t)TINY_REGION_END(r);
    boolean_t	is_free;
    msize_t	msize;
    tiny_pg_pair_t advisory[((TINY_REGION_PAYLOAD_BYTES + vm_page_size - 1) >> vm_page_shift) >> 1]; // 256bytes stack allocated
    int		advisories = 0;

    // Scan the metadata identifying blocks which span one or more pages. Mark the pages MADV_FREE taking care to preserve free list
    // management data. 
    while (current < limit) {
	msize = get_tiny_meta_header((void *)current, &is_free);
	if (is_free && !msize && (current == start)) {
	    // first block is all free
#if DEBUG_MALLOC
	    malloc_printf("*** tiny_free_scan_madvise_free first block is all free! %p: msize=%d is_free =%d\n", 
	    (void *)current, msize, is_free);
#endif	    
	    uintptr_t pgLo = round_page(start + sizeof(free_list_t) + sizeof(msize_t));
	    uintptr_t pgHi = trunc_page(start + TINY_REGION_SIZE - sizeof(msize_t));
	    
	    if (pgLo < pgHi) {
		advisory[advisories].pnum = (pgLo - start) >> vm_page_shift;
		advisory[advisories].size = (pgHi - pgLo) >> vm_page_shift;
		advisories++;
	    }
	    break;
	}
	if (!msize) {
#if DEBUG_MALLOC
	    malloc_printf("*** tiny_free_scan_madvise_free error with %p: msize=%d is_free =%d\n", 
	    (void *)current, msize, is_free);
#endif
	    break;
	}
	if (is_free) {
	    uintptr_t pgLo = round_page(current + sizeof(free_list_t) + sizeof(msize_t));
	    uintptr_t pgHi = trunc_page(current + TINY_BYTES_FOR_MSIZE(msize) - sizeof(msize_t));
	    
	    if (pgLo < pgHi) {
		advisory[advisories].pnum = (pgLo - start) >> vm_page_shift;
		advisory[advisories].size = (pgHi - pgLo) >> vm_page_shift;
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
	SZONE_MAGAZINE_PTR_UNLOCK(szone, depot_ptr);
	for (i = 0; i < advisories; ++i) {
	    uintptr_t addr = (advisory[i].pnum << vm_page_shift) + start;
	    size_t size = advisory[i].size << vm_page_shift;
	    
#if TARGET_OS_EMBEDDED
		madvise_free_range(szone, r, addr, addr + size, NULL);
#else
		madvise_free_range(szone, r, addr, addr + size);
#endif
	}
	SZONE_MAGAZINE_PTR_LOCK(szone, depot_ptr);
	OSAtomicDecrement32Barrier(&(REGION_TRAILER_FOR_TINY_REGION(r)->pinned_to_depot));
    }
}

static region_t
tiny_free_try_depot_unmap_no_lock(szone_t *szone, magazine_t *depot_ptr, region_trailer_t *node)
{
    if (0 < node->bytes_used || 
        0 < node->pinned_to_depot ||
	depot_ptr->recirculation_entries < (szone->num_tiny_magazines * 2)) {
	return NULL;
    }
    
    // disconnect node from Depot
    recirc_list_extract(szone, depot_ptr, node);
    
    // Iterate the region pulling its free entries off the (locked) Depot's free list
    region_t sparse_region = TINY_REGION_FOR_PTR(node);
    int objects_in_use = tiny_free_detach_region(szone, depot_ptr, sparse_region);
    
    if (0 == objects_in_use) {
	// Invalidate the hash table entry for this region with HASHRING_REGION_DEALLOCATED.
	// Using HASHRING_REGION_DEALLOCATED preserves the collision chain, using HASHRING_OPEN_ENTRY (0) would not.
	rgnhdl_t pSlot = hash_lookup_region_no_lock(szone->tiny_region_generation->hashed_regions, 
				     szone->tiny_region_generation->num_regions_allocated, 
				     szone->tiny_region_generation->num_regions_allocated_shift, sparse_region); 
	if (NULL == pSlot) {
	    szone_error(szone, 1, "tiny_free_try_depot_unmap_no_lock hash lookup failed:", NULL, "%p\n", sparse_region);
	    return NULL;
	}
	*pSlot = HASHRING_REGION_DEALLOCATED;
	depot_ptr->num_bytes_in_magazine -= TINY_REGION_PAYLOAD_BYTES;
	__sync_fetch_and_add( &(szone->num_tiny_regions_dealloc), 1); // Atomically increment num_tiny_regions_dealloc
	
	// Caller will transfer ownership of the region back to the OS with no locks held
	MAGMALLOC_DEALLOCREGION((void *)szone, (void *)sparse_region, TINY_REGION_SIZE); // DTrace USDT Probe
	return sparse_region;
    } else {
	szone_error(szone, 1, "tiny_free_try_depot_unmap_no_lock objects_in_use not zero:", NULL, "%d\n", objects_in_use);
	return NULL;
    }
}

static boolean_t
tiny_free_do_recirc_to_depot(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index)
{	
	// The entire magazine crossed the "emptiness threshold". Transfer a region
	// from this magazine to the Depot. Choose a region that itself has crossed the emptiness threshold (i.e
	// is at least fraction "f" empty.) Such a region will be marked "suitable" on the recirculation list.
	region_trailer_t *node = tiny_mag_ptr->firstNode;
	
	while (node && !node->recirc_suitable) {
	    node = node->next;
	}
	
	if (NULL == node) {
#if DEBUG_MALLOC
	    malloc_printf("*** tiny_free_do_recirc_to_depot end of list\n");
#endif	
	    return TRUE; // Caller must SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	}

	region_t sparse_region = TINY_REGION_FOR_PTR(node);

	// Deal with unclaimed memory -- mag_bytes_free_at_end or mag_bytes_free_at_start
	if (sparse_region == tiny_mag_ptr->mag_last_region && (tiny_mag_ptr->mag_bytes_free_at_end || tiny_mag_ptr->mag_bytes_free_at_start)) {
	    tiny_finalize_region(szone, tiny_mag_ptr);
	}
	
	// disconnect "suitable" node from magazine
	recirc_list_extract(szone, tiny_mag_ptr, node);

	// Iterate the region pulling its free entries off its (locked) magazine's free list
	int objects_in_use = tiny_free_detach_region(szone, tiny_mag_ptr, sparse_region);
	magazine_t *depot_ptr = &(szone->tiny_magazines[DEPOT_MAGAZINE_INDEX]);
	
	// hand over the region to the (locked) Depot
	SZONE_MAGAZINE_PTR_LOCK(szone,depot_ptr);
	// this will cause tiny_free_list_add_ptr called by tiny_free_reattach_region to use 
	// the depot as its target magazine, rather than magazine formerly associated with sparse_region
	MAGAZINE_INDEX_FOR_TINY_REGION(sparse_region) = DEPOT_MAGAZINE_INDEX;
	node->pinned_to_depot = 0;

	// Iterate the region putting its free entries on Depot's free list
	size_t bytes_inplay = tiny_free_reattach_region(szone, depot_ptr, sparse_region);
	
	tiny_mag_ptr->mag_num_bytes_in_objects -= bytes_inplay;
	tiny_mag_ptr->num_bytes_in_magazine -= TINY_REGION_PAYLOAD_BYTES;
	tiny_mag_ptr->mag_num_objects -= objects_in_use;
	
	SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr); // Unlock the originating magazine
	
	depot_ptr->mag_num_bytes_in_objects += bytes_inplay;
	depot_ptr->num_bytes_in_magazine += TINY_REGION_PAYLOAD_BYTES;
	depot_ptr->mag_num_objects += objects_in_use;
	
	// connect to Depot as last node
	recirc_list_splice_last(szone, depot_ptr, node);

	MAGMALLOC_RECIRCREGION((void *)szone, (int)mag_index, (void *)sparse_region, TINY_REGION_SIZE, 
	    (int)BYTES_USED_FOR_TINY_REGION(sparse_region));  // DTrace USDT Probe

	// Mark free'd dirty pages with MADV_FREE to reduce memory pressure
	tiny_free_scan_madvise_free(szone, depot_ptr, sparse_region);
	
	// If the region is entirely empty vm_deallocate() it outside the depot lock
	region_t r_dealloc = tiny_free_try_depot_unmap_no_lock(szone, depot_ptr, node);
	SZONE_MAGAZINE_PTR_UNLOCK(szone,depot_ptr);
	if (r_dealloc)
	    deallocate_pages(szone, r_dealloc, TINY_REGION_SIZE, 0);
	return FALSE; // Caller need not unlock the originating magazine
}    

static region_t
tiny_find_msize_region(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, msize_t msize)
{
    free_list_t		*ptr;
    grain_t		slot = msize - 1;
    free_list_t		**free_list = tiny_mag_ptr->mag_free_list;
    free_list_t		**the_slot = free_list + slot;
    free_list_t		**limit;
#if defined(__LP64__)
    uint64_t		bitmap;
#else
    uint32_t		bitmap;
#endif
    // Assumes we've locked the magazine
    CHECK_MAGAZINE_PTR_LOCKED(szone, tiny_mag_ptr, __PRETTY_FUNCTION__);
    
    // Look for an exact match by checking the freelist for this msize.
    ptr = *the_slot;
    if (ptr)
	return TINY_REGION_FOR_PTR(ptr);

    // Mask off the bits representing slots holding free blocks smaller than the
    // size we need.  If there are no larger free blocks, try allocating from
    // the free space at the end of the tiny region.
#if defined(__LP64__)
    bitmap = ((uint64_t *)(tiny_mag_ptr->mag_bitmap))[0] & ~ ((1ULL << slot) - 1);
#else
    bitmap = tiny_mag_ptr->mag_bitmap[0] & ~ ((1 << slot) - 1);
#endif
    if (!bitmap)
        return NULL;
        
    slot = BITMAPV_CTZ(bitmap);
    limit = free_list + NUM_TINY_SLOTS - 1;
    free_list += slot;
    
    if (free_list < limit) {
        ptr = *free_list;
        if (ptr)
	    return TINY_REGION_FOR_PTR(ptr);
	else {
	    /* Shouldn't happen. Fall through to look at last slot. */
#if DEBUG_MALLOC
	    malloc_printf("in tiny_find_msize_region(), mag_bitmap out of sync, slot=%d\n",slot);
#endif
	}
    }

    // We are now looking at the last slot, which contains blocks equal to, or
    // due to coalescing of free blocks, larger than (NUM_TINY_SLOTS - 1) * tiny quantum size.
    ptr = *limit;
    if (ptr)
	return TINY_REGION_FOR_PTR(ptr);

    return NULL;
}

static boolean_t
tiny_get_region_from_depot(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, msize_t msize)
{
    magazine_t *depot_ptr = &(szone->tiny_magazines[DEPOT_MAGAZINE_INDEX]);

    /* FIXME: Would Uniprocessor benefit from recirc and MADV_FREE? */
    if (szone->num_tiny_magazines == 1) // Uniprocessor, single magazine, so no recirculation necessary
	return 0;

#if DEBUG_MALLOC
    if (DEPOT_MAGAZINE_INDEX == mag_index) {
	szone_error(szone, 1, "tiny_get_region_from_depot called for magazine index -1", NULL, NULL);
	return 0;
    }
#endif
	
    SZONE_MAGAZINE_PTR_LOCK(szone,depot_ptr);

    // Appropriate a Depot'd region that can satisfy requested msize.
    region_trailer_t *node;
    region_t sparse_region;
	
    while (1) {
	sparse_region = tiny_find_msize_region(szone, depot_ptr, DEPOT_MAGAZINE_INDEX, msize);
	if (NULL == sparse_region) { // Depot empty?
        SZONE_MAGAZINE_PTR_UNLOCK(szone,depot_ptr);
	return 0;
	}
    
	node = REGION_TRAILER_FOR_TINY_REGION(sparse_region);
	if (0 >= node->pinned_to_depot)
	    break;
	
	SZONE_MAGAZINE_PTR_UNLOCK(szone,depot_ptr);
	pthread_yield_np();
	SZONE_MAGAZINE_PTR_LOCK(szone,depot_ptr);
    }

    // disconnect node from Depot
    recirc_list_extract(szone, depot_ptr, node);
    
    // Iterate the region pulling its free entries off the (locked) Depot's free list
    int objects_in_use = tiny_free_detach_region(szone, depot_ptr, sparse_region);

    // Transfer ownership of the region
    MAGAZINE_INDEX_FOR_TINY_REGION(sparse_region) = mag_index;
    node->pinned_to_depot = 0;
    
    // Iterate the region putting its free entries on its new (locked) magazine's free list
    size_t bytes_inplay = tiny_free_reattach_region(szone, tiny_mag_ptr, sparse_region);
    
    depot_ptr->mag_num_bytes_in_objects -= bytes_inplay;
    depot_ptr->num_bytes_in_magazine -= TINY_REGION_PAYLOAD_BYTES;
    depot_ptr->mag_num_objects -= objects_in_use;
	
    tiny_mag_ptr->mag_num_bytes_in_objects += bytes_inplay;
    tiny_mag_ptr->num_bytes_in_magazine += TINY_REGION_PAYLOAD_BYTES;
    tiny_mag_ptr->mag_num_objects += objects_in_use;
    
    // connect to magazine as first node
    recirc_list_splice_first(szone, tiny_mag_ptr, node);
    	
    SZONE_MAGAZINE_PTR_UNLOCK(szone,depot_ptr);

    // madvise() outside the Depot lock
#if TARGET_OS_EMBEDDED
    if (node->failedREUSE) {
#else
    if (node->failedREUSE ||
	-1 == madvise((void *)sparse_region, TINY_REGION_PAYLOAD_BYTES, MADV_FREE_REUSE)) {
#endif
	/* -1 return: VM map entry change makes this unfit for reuse. Something evil lurks. */
#if DEBUG_MADVISE
	szone_error(szone, 0, "tiny_get_region_from_depot madvise(..., MADV_FREE_REUSE) failed", 
	    sparse_region, "length=%d\n", TINY_REGION_PAYLOAD_BYTES);
#endif
	node->failedREUSE = TRUE;
    }

    MAGMALLOC_DEPOTREGION((void *)szone, (int)mag_index, (void *)sparse_region, TINY_REGION_SIZE, 
        (int)BYTES_USED_FOR_TINY_REGION(sparse_region)); // DTrace USDT Probe
    
    return 1;
}

#define K 1.5 // headroom measured in number of 1Mb regions
#define DENSITY_THRESHOLD(a) \
    ((a) - ((a) >> 2)) // "Emptiness" f = 0.25, so "Density" is (1 - f)*a. Generally: ((a) - ((a) >> -log2(f))) 

static INLINE boolean_t
tiny_free_no_lock(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, region_t region, void *ptr, 
		  msize_t msize)
{
    void	*original_ptr = ptr;
    size_t	original_size = TINY_BYTES_FOR_MSIZE(msize);
    void	*next_block = ((unsigned char *)ptr + original_size);
    msize_t	previous_msize, next_msize;
    void	*previous;
    free_list_t	*big_free_block;
    free_list_t	*after_next_block;
    free_list_t	*before_next_block;

#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
	malloc_printf("in tiny_free_no_lock(), ptr=%p, msize=%d\n", ptr, msize);
    }
    if (!msize) {
	szone_error(szone, 1, "trying to free tiny block that is too small", ptr,
	    "in tiny_free_no_lock(), ptr=%p, msize=%d\n", ptr, msize);
    }
#endif
    
    // We try to coalesce this block with the preceeding one
    previous = tiny_previous_preceding_free(ptr, &previous_msize);
    if (previous) {
#if DEBUG_MALLOC
        if (LOG(szone, ptr) || LOG(szone,previous)) { 
            malloc_printf("in tiny_free_no_lock(), coalesced backwards for %p previous=%p\n", ptr, previous);
        }
#endif

        // clear the meta_header since this is no longer the start of a block
        set_tiny_meta_header_middle(ptr);
        tiny_free_list_remove_ptr(szone, tiny_mag_ptr, previous, previous_msize);
        ptr = previous;
        msize += previous_msize;
    }
    // We try to coalesce with the next block
    if ((next_block < TINY_REGION_END(region)) && tiny_meta_header_is_free(next_block)) {
        next_msize = get_tiny_free_size(next_block);
#if DEBUG_MALLOC
        if (LOG(szone, ptr) || LOG(szone, next_block)) {
            malloc_printf("in tiny_free_no_lock(), for ptr=%p, msize=%d coalesced forward=%p next_msize=%d\n",
              ptr, msize, next_block, next_msize);
        }
#endif
        // If we are coalescing with the next block, and the next block is in
        // the last slot of the free list, then we optimize this case here to 
        // avoid removing next_block from the slot (NUM_TINY_SLOTS - 1) and then adding ptr back
        // to slot (NUM_TINY_SLOTS - 1).
        if (next_msize >= NUM_TINY_SLOTS) {
            msize += next_msize;
	    
            big_free_block = (free_list_t *)next_block;
            after_next_block = free_list_unchecksum_ptr(szone, &big_free_block->next);
            before_next_block = free_list_unchecksum_ptr(szone, &big_free_block->previous);
	    
            if (!before_next_block) {
                tiny_mag_ptr->mag_free_list[NUM_TINY_SLOTS-1] = ptr;
            } else {
                before_next_block->next.u = free_list_checksum_ptr(szone, ptr);
            }
	    
            if (after_next_block) {
                after_next_block->previous.u = free_list_checksum_ptr(szone, ptr);
            }
	    
            // we don't need to checksum these since they are already checksummed
            ((free_list_t *)ptr)->previous = big_free_block->previous;
            ((free_list_t *)ptr)->next = big_free_block->next;
            
            // clear the meta_header to enable coalescing backwards
            set_tiny_meta_header_middle(big_free_block);
            set_tiny_meta_header_free(ptr, msize);
	    
            goto tiny_free_ending;
        }
        tiny_free_list_remove_ptr(szone, tiny_mag_ptr, next_block, next_msize);
        set_tiny_meta_header_middle(next_block); // clear the meta_header to enable coalescing backwards
        msize += next_msize;
    }

    // The tiny cache already scribbles free blocks as they go through the
    // cache whenever msize < TINY_QUANTUM , so we do not need to do it here.
    if ((szone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE) && msize && (msize >= TINY_QUANTUM))
        memset(ptr, 0x55, TINY_BYTES_FOR_MSIZE(msize));

    tiny_free_list_add_ptr(szone, tiny_mag_ptr, ptr, msize);
    
  tiny_free_ending:
    
    tiny_mag_ptr->mag_num_objects--;
    // we use original_size and not msize to avoid double counting the coalesced blocks
    tiny_mag_ptr->mag_num_bytes_in_objects -= original_size;
    
    // Update this region's bytes in use count
    region_trailer_t *node = REGION_TRAILER_FOR_TINY_REGION(region);
    size_t bytes_used = node->bytes_used - original_size;
    node->bytes_used = bytes_used;
    
#if !TARGET_OS_EMBEDDED // Always madvise for embedded platforms
    /* FIXME: Would Uniprocessor benefit from recirc and MADV_FREE? */
    if (szone->num_tiny_magazines == 1) { // Uniprocessor, single magazine, so no recirculation necessary
	/* NOTHING */
    } else if (DEPOT_MAGAZINE_INDEX != mag_index) {
	// Emptiness discriminant
	if (bytes_used < DENSITY_THRESHOLD(TINY_REGION_PAYLOAD_BYTES)) {
	    /* Region has crossed threshold from density to sparsity. Mark it "suitable" on the
	       recirculation candidates list. */
	    node->recirc_suitable = TRUE;
	} else {
	    /* After this free, we've found the region is still dense, so it must have been even more so before
	       the free. That implies the region is already correctly marked. Do nothing. */
	}
    
	// Has the entire magazine crossed the "emptiness threshold"? If so, transfer a region
	// from this magazine to the Depot. Choose a region that itself has crossed the emptiness threshold (i.e
	// is at least fraction "f" empty.) Such a region will be marked "suitable" on the recirculation list.
	size_t a = tiny_mag_ptr->num_bytes_in_magazine; // Total bytes allocated to this magazine
	size_t u = tiny_mag_ptr->mag_num_bytes_in_objects; // In use (malloc'd) from this magaqzine

	if (a - u > ((3 * TINY_REGION_PAYLOAD_BYTES) / 2) && u < DENSITY_THRESHOLD(a)) {
	    return tiny_free_do_recirc_to_depot(szone, tiny_mag_ptr, mag_index);
	}
	    
    } else {
#endif
	// Freed to Depot. N.B. Lock on tiny_magazines[DEPOT_MAGAZINE_INDEX] is already held
	// Calcuate the first page in the coalesced block that would be safe to mark MADV_FREE
	uintptr_t safe_ptr = (uintptr_t)ptr + sizeof(free_list_t) + sizeof(msize_t);
	uintptr_t round_safe = round_page(safe_ptr);
	
	// Calcuate the last page in the coalesced block that would be safe to mark MADV_FREE
	uintptr_t safe_extent = (uintptr_t)ptr + TINY_BYTES_FOR_MSIZE(msize) - sizeof(msize_t);
	uintptr_t trunc_extent = trunc_page(safe_extent);

	// The newly freed block may complete a span of bytes that cover a page. Mark it with MADV_FREE.
	if (round_safe < trunc_extent) { // Safe area covers a page (perhaps many)
            uintptr_t lo = trunc_page((uintptr_t)original_ptr);
            uintptr_t hi = round_page((uintptr_t)original_ptr + original_size);
	
	    tiny_free_list_remove_ptr(szone, tiny_mag_ptr, ptr, msize);
	    set_tiny_meta_header_in_use(ptr, msize);

	    OSAtomicIncrement32Barrier(&(node->pinned_to_depot));
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
#if TARGET_OS_EMBEDDED
	    madvise_free_range(szone, region, MAX(round_safe, lo), MIN(trunc_extent, hi), &szone->last_tiny_advise);
#else
	    madvise_free_range(szone, region, MAX(round_safe, lo), MIN(trunc_extent, hi));
#endif
	    SZONE_MAGAZINE_PTR_LOCK(szone, tiny_mag_ptr);
	    OSAtomicDecrement32Barrier(&(node->pinned_to_depot));

	    set_tiny_meta_header_free(ptr, msize);
	    tiny_free_list_add_ptr(szone, tiny_mag_ptr, ptr, msize);
        }
	
#if !TARGET_OS_EMBEDDED
	if (0 < bytes_used || 0 < node->pinned_to_depot) {
	    /* Depot'd region is still live. Leave it in place on the Depot's recirculation list
	       so as to avoid thrashing between the Depot's free list and a magazines's free list
	       with detach_region/reattach_region */
	} else {
	    /* Depot'd region is just now empty. Consider return to OS. */
	    region_t r_dealloc = tiny_free_try_depot_unmap_no_lock(szone, tiny_mag_ptr, node);
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	    if (r_dealloc)
		deallocate_pages(szone, r_dealloc, TINY_REGION_SIZE, 0);
	    return FALSE; // Caller need not unlock
	}
    }
#endif
    
    return TRUE; // Caller must do SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr)
}

// Allocates from the last region or a freshly allocated region
static void *
tiny_malloc_from_region_no_lock(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, 
    msize_t msize, void * aligned_address)
{
    void	*ptr;

    // Deal with unclaimed memory -- mag_bytes_free_at_end or mag_bytes_free_at_start
    if (tiny_mag_ptr->mag_bytes_free_at_end || tiny_mag_ptr->mag_bytes_free_at_start)
	tiny_finalize_region(szone, tiny_mag_ptr);

    // We set the unused bits of the header in the last pair to be all ones, and those of the inuse to zeroes.
    ((tiny_region_t)aligned_address)->pairs[CEIL_NUM_TINY_BLOCKS_WORDS-1].header = 
	(NUM_TINY_BLOCKS & 31) ? (0xFFFFFFFFU << (NUM_TINY_BLOCKS & 31)) : 0;
    ((tiny_region_t)aligned_address)->pairs[CEIL_NUM_TINY_BLOCKS_WORDS-1].inuse = 0;
    
    // Here find the only place in tinyland that (infrequently) takes the tiny_regions_lock.
    // Only one thread at a time should be permitted to assess the density of the hash
    // ring and adjust if needed. 
    // Only one thread at a time should be permitted to insert its new region on
    // the hash ring.
    // It is safe for all other threads to read the hash ring (hashed_regions) and
    // the associated sizes (num_regions_allocated and num_tiny_regions).
    
    LOCK(szone->tiny_regions_lock);
    
    // Check to see if the hash ring of tiny regions needs to grow.  Try to
    // avoid the hash ring becoming too dense.
    if (szone->tiny_region_generation->num_regions_allocated < (2 * szone->num_tiny_regions)) {
	region_t *new_regions;
	size_t new_size;
	size_t new_shift = szone->tiny_region_generation->num_regions_allocated_shift; // In/Out parameter
	new_regions = hash_regions_grow_no_lock(szone, szone->tiny_region_generation->hashed_regions,
				      szone->tiny_region_generation->num_regions_allocated,
				      &new_shift,
				      &new_size);
	// Do not deallocate the current hashed_regions allocation since someone may
	// be iterating it.  Instead, just leak it.

	// Prepare to advance to the "next generation" of the hash ring.
	szone->tiny_region_generation->nextgen->hashed_regions = new_regions;
	szone->tiny_region_generation->nextgen->num_regions_allocated = new_size;
	szone->tiny_region_generation->nextgen->num_regions_allocated_shift = new_shift;

	// Throw the switch to atomically advance to the next generation.
	szone->tiny_region_generation = szone->tiny_region_generation->nextgen;
	// Ensure everyone sees the advance.
	OSMemoryBarrier();
    }
    // Tag the region at "aligned_address" as belonging to us, 
    // and so put it under the protection of the magazine lock we are holding.
    // Do this before advertising "aligned_address" on the hash ring(!)
    MAGAZINE_INDEX_FOR_TINY_REGION(aligned_address) = mag_index;

    // Insert the new region into the hash ring, and update malloc statistics
    hash_region_insert_no_lock(szone->tiny_region_generation->hashed_regions, 
                               szone->tiny_region_generation->num_regions_allocated,
			       szone->tiny_region_generation->num_regions_allocated_shift,
                               aligned_address);
    
    szone->num_tiny_regions++;
    UNLOCK(szone->tiny_regions_lock);
    
    tiny_mag_ptr->mag_last_region = aligned_address;
    BYTES_USED_FOR_TINY_REGION(aligned_address) = TINY_BYTES_FOR_MSIZE(msize);
#if ASLR_INTERNAL
    int offset_msize = malloc_entropy[0] & TINY_ENTROPY_MASK;
#if DEBUG_MALLOC
    if (getenv("MallocASLRForce")) offset_msize = strtol(getenv("MallocASLRForce"), NULL, 0) & TINY_ENTROPY_MASK;
    if (getenv("MallocASLRPrint")) malloc_printf("Region: %p offset: %d\n", aligned_address, offset_msize);
#endif
#else
    int offset_msize = 0;
#endif
    ptr = (void *)((uintptr_t) aligned_address + TINY_BYTES_FOR_MSIZE(offset_msize)); 
    set_tiny_meta_header_in_use(ptr, msize);
    tiny_mag_ptr->mag_num_objects++;
    tiny_mag_ptr->mag_num_bytes_in_objects += TINY_BYTES_FOR_MSIZE(msize);
    tiny_mag_ptr->num_bytes_in_magazine += TINY_REGION_PAYLOAD_BYTES;
 
    // We put a header on the last block so that it appears in use (for coalescing, etc...)
    set_tiny_meta_header_in_use_1((void *)((uintptr_t)ptr + TINY_BYTES_FOR_MSIZE(msize)));
    tiny_mag_ptr->mag_bytes_free_at_end = TINY_BYTES_FOR_MSIZE(NUM_TINY_BLOCKS - msize - offset_msize);

#if ASLR_INTERNAL
    // Put a header on the previous block for same reason
    tiny_mag_ptr->mag_bytes_free_at_start = TINY_BYTES_FOR_MSIZE(offset_msize);
    if (offset_msize) {
	set_tiny_meta_header_in_use_1((void *)((uintptr_t)ptr - TINY_QUANTUM));
    }
#else
    tiny_mag_ptr->mag_bytes_free_at_start = 0;
#endif

    // connect to magazine as last node
    recirc_list_splice_last(szone, tiny_mag_ptr, REGION_TRAILER_FOR_TINY_REGION(aligned_address));

#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
	malloc_printf("in tiny_malloc_from_region_no_lock(), ptr=%p, msize=%d\n", ptr, msize);
    }
#endif
    return ptr;
}

static INLINE void *
tiny_try_shrink_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_good_size)
{
    msize_t new_msize = TINY_MSIZE_FOR_BYTES(new_good_size);
    msize_t mshrinkage = TINY_MSIZE_FOR_BYTES(old_size) - new_msize;

    if (mshrinkage) {
	void *q = (void *)((uintptr_t)ptr + TINY_BYTES_FOR_MSIZE(new_msize));
	magazine_t *tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->tiny_magazines,
		    REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr)), 
		    MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr)));

	// Mark q as block header and in-use, thus creating two blocks.
	set_tiny_meta_header_in_use(q, mshrinkage);
	tiny_mag_ptr->mag_num_objects++;

	SZONE_MAGAZINE_PTR_UNLOCK(szone,tiny_mag_ptr);
	szone_free(szone, q); // avoid inlining free_tiny(szone, q, ...);
    }
    return ptr;
}

static INLINE boolean_t
tiny_try_realloc_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_size)
{
    // returns 1 on success
    msize_t	index;
    msize_t	old_msize;
    unsigned	next_index;
    void	*next_block;
    boolean_t	is_free;
    msize_t	next_msize, coalesced_msize, leftover_msize;
    void	*leftover;

    index = TINY_INDEX_FOR_PTR(ptr);
    old_msize = TINY_MSIZE_FOR_BYTES(old_size);
    next_index = index + old_msize;
    
    if (next_index >= NUM_TINY_BLOCKS) {
	return 0;
    }
    next_block = (char *)ptr + old_size;
    
    magazine_t	*tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->tiny_magazines,
				    REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr)), 
				    MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr)));    
    
    /*
     * Look for a free block immediately afterwards.  If it's large enough, we can consume (part of)
     * it.
     */
    is_free = tiny_meta_header_is_free(next_block);
    if (!is_free) {
	SZONE_MAGAZINE_PTR_UNLOCK(szone,tiny_mag_ptr);
	return 0; // next_block is in use;
    }
    next_msize = get_tiny_free_size(next_block);
    if (old_size + TINY_BYTES_FOR_MSIZE(next_msize) < new_size) {
	SZONE_MAGAZINE_PTR_UNLOCK(szone,tiny_mag_ptr);
	return 0; // even with next block, not enough
    }
    /*
     * The following block is big enough; pull it from its freelist and chop off enough to satisfy
     * our needs.
     */
    tiny_free_list_remove_ptr(szone, tiny_mag_ptr, next_block, next_msize);
    set_tiny_meta_header_middle(next_block); // clear the meta_header to enable coalescing backwards
    coalesced_msize = TINY_MSIZE_FOR_BYTES(new_size - old_size + TINY_QUANTUM - 1);
    leftover_msize = next_msize - coalesced_msize;
    if (leftover_msize) {
	/* there's some left, so put the remainder back */
	leftover = (void *)((uintptr_t)next_block + TINY_BYTES_FOR_MSIZE(coalesced_msize));
	
	tiny_free_list_add_ptr(szone, tiny_mag_ptr, leftover, leftover_msize);
    }
    set_tiny_meta_header_in_use(ptr, old_msize + coalesced_msize);
#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
	malloc_printf("in tiny_try_realloc_in_place(), ptr=%p, msize=%d\n", ptr, old_msize + coalesced_msize);
    }
#endif
    tiny_mag_ptr->mag_num_bytes_in_objects += TINY_BYTES_FOR_MSIZE(coalesced_msize);

    // Update this region's bytes in use count
    region_trailer_t *node = REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr));
    size_t bytes_used = node->bytes_used + TINY_BYTES_FOR_MSIZE(coalesced_msize);
    node->bytes_used = bytes_used;

    // Emptiness discriminant
    if (bytes_used < DENSITY_THRESHOLD(TINY_REGION_PAYLOAD_BYTES)) {
        /* After this reallocation the region is still sparse, so it must have been even more so before
	   the reallocation. That implies the region is already correctly marked. Do nothing. */
    } else {
	/* Region has crossed threshold from sparsity to density. Mark it not "suitable" on the
	   recirculation candidates list. */
	node->recirc_suitable = FALSE;
   }

    SZONE_MAGAZINE_PTR_UNLOCK(szone,tiny_mag_ptr);
    CHECK(szone, __PRETTY_FUNCTION__);
    return 1;
}

static boolean_t
tiny_check_region(szone_t *szone, region_t region)
{
    uintptr_t	start, ptr, region_end;
    boolean_t	prev_free = 0;
    boolean_t	is_free;
    msize_t	msize;
    free_list_t	*free_head;
    void	*follower, *previous, *next;
    mag_index_t	mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(region);
    magazine_t	*tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);

    // Assumes locked
    CHECK_MAGAZINE_PTR_LOCKED(szone, tiny_mag_ptr, __PRETTY_FUNCTION__);
    
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
		malloc_printf("*** invariant broken for leader block %p - %d %d\n", ptr - TINY_QUANTUM, msize, is_free);
	    }
	}
    }
    region_end = (uintptr_t)TINY_REGION_END(region);

    /*
     * The last region may have a trailing chunk which has not been converted into inuse/freelist
     * blocks yet.
     */
    if (region == tiny_mag_ptr->mag_last_region)
	    region_end -= tiny_mag_ptr->mag_bytes_free_at_end;

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
	    malloc_printf("*** invariant broken for tiny block %p this msize=%d - size is too small\n",
	      ptr, msize);
	    return 0;
	}

	if (!is_free) {
	    /*
	     * In use blocks cannot be more than (NUM_TINY_SLOTS - 1) quanta large.
	     */
	    prev_free = 0;
	    if (msize > (NUM_TINY_SLOTS - 1)) {
		malloc_printf("*** invariant broken for %p this tiny msize=%d - size is too large\n",
		  ptr, msize);
		return 0;
	    }
	    /* move to next block */
	    ptr += TINY_BYTES_FOR_MSIZE(msize);
	} else {
#if !RELAXED_INVARIANT_CHECKS
	    /*
	     * Free blocks must have been coalesced, we cannot have a free block following another
	     * free block.
	     */
	    if (prev_free) {
		malloc_printf("*** invariant broken for free block %p this tiny msize=%d: two free blocks in a row\n",
		  ptr, msize);
		return 0;
	    }
#endif // RELAXED_INVARIANT_CHECKS
	    prev_free = 1;
	    /*
	     * Check the integrity of this block's entry in its freelist.
	     */
	    free_head = (free_list_t *)ptr;
	    previous = free_list_unchecksum_ptr(szone, &free_head->previous);
	    next = free_list_unchecksum_ptr(szone, &free_head->next);
	    if (previous && !tiny_meta_header_is_free(previous)) {
		malloc_printf("*** invariant broken for %p (previous %p is not a free pointer)\n",
		  ptr, previous);
		return 0;
	    }
	    if (next && !tiny_meta_header_is_free(next)) {
		malloc_printf("*** invariant broken for %p (next in free list %p is not a free pointer)\n",
		  ptr, next);
		return 0;
	    }
	    /*
	     * Check the free block's trailing size value.
	     */
	    follower = FOLLOWING_TINY_PTR(ptr, msize);
	    if (((uintptr_t)follower != region_end) && (get_tiny_previous_free_msize(follower) != msize)) {
		malloc_printf("*** invariant broken for tiny free %p followed by %p in region [%p-%p] "
		  "(end marker incorrect) should be %d; in fact %d\n",
		  ptr, follower, TINY_REGION_ADDRESS(region), region_end, msize, get_tiny_previous_free_msize(follower));
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
	malloc_printf("*** invariant broken for region end %p - %p\n", ptr, region_end);
	return 0;
    }
    /*
     * Check the trailing block's integrity.
     */
    if (region == tiny_mag_ptr->mag_last_region) {
	if (tiny_mag_ptr->mag_bytes_free_at_end) {
	    msize = get_tiny_meta_header((void *)ptr, &is_free);
	    if (is_free || (msize != 1)) {
		malloc_printf("*** invariant broken for blocker block %p - %d %d\n", ptr, msize, is_free);
	    }
	}
    }
    return 1;
}

static kern_return_t
tiny_in_use_enumerator(task_t task, void *context, unsigned type_mask, szone_t *szone,
		       memory_reader_t reader, vm_range_recorder_t recorder)
{
    size_t		num_regions;
    size_t		index;
    region_t		*regions;
    vm_range_t		buffer[MAX_RECORDER_BUFFER];
    unsigned		count = 0;
    kern_return_t	err;
    region_t		region;
    vm_range_t		range;
    vm_range_t		admin_range;
    vm_range_t		ptr_range;
    unsigned char	*mapped_region;
    uint32_t		*block_header;
    uint32_t		*in_use;
    unsigned		block_index;
    unsigned		block_limit;
    boolean_t		is_free;
    msize_t		msize;
    void		*mapped_ptr;
    unsigned 		bit;
    magazine_t          *tiny_mag_base = NULL;
    
    region_hash_generation_t *trg_ptr;
    err = reader(task, (vm_address_t)szone->tiny_region_generation, sizeof(region_hash_generation_t), (void **)&trg_ptr);
    if (err) return err;

    num_regions = trg_ptr->num_regions_allocated;
    err = reader(task, (vm_address_t)trg_ptr->hashed_regions, sizeof(region_t) * num_regions, (void **)&regions);
    if (err) return err;

    if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
        // Map in all active magazines. Do this outside the iteration over regions.
        err = reader(task, (vm_address_t)(szone->tiny_magazines),
                szone->num_tiny_magazines*sizeof(magazine_t),(void **)&tiny_mag_base);
        if (err) return err;
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
		void 		*mag_last_free;
                vm_address_t	mag_last_free_ptr = 0;
                msize_t		mag_last_free_msize = 0;
                
		err = reader(task, range.address, range.size, (void **)&mapped_region);
		if (err)
		    return err;

		mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(mapped_region);
		magazine_t *tiny_mag_ptr = tiny_mag_base + mag_index;

		if (DEPOT_MAGAZINE_INDEX != mag_index) {
		    mag_last_free = tiny_mag_ptr->mag_last_free;
		    if (mag_last_free) {
			mag_last_free_ptr = (uintptr_t) mag_last_free & ~(TINY_QUANTUM - 1);
			mag_last_free_msize = (uintptr_t) mag_last_free & (TINY_QUANTUM - 1);
		    }
		} else {
		    for (mag_index = 0; mag_index < szone->num_tiny_magazines; mag_index++) {
			if ((void *)range.address == (tiny_mag_base + mag_index)->mag_last_free_rgn) {
			    mag_last_free = (tiny_mag_base + mag_index)->mag_last_free;
			    if (mag_last_free) {
				mag_last_free_ptr = (uintptr_t) mag_last_free & ~(TINY_QUANTUM - 1);
				mag_last_free_msize = (uintptr_t) mag_last_free & (TINY_QUANTUM - 1);
			    }
			}
		    }
		}

		block_header = (uint32_t *)(mapped_region + TINY_METADATA_START + sizeof(region_trailer_t));
		in_use = TINY_INUSE_FOR_HEADER(block_header);
		block_index = 0;
		block_limit = NUM_TINY_BLOCKS;
		if (region == tiny_mag_ptr->mag_last_region) {
		    block_index += TINY_MSIZE_FOR_BYTES(tiny_mag_ptr->mag_bytes_free_at_start);
		    block_limit -= TINY_MSIZE_FOR_BYTES(tiny_mag_ptr->mag_bytes_free_at_end);
		}

		while (block_index < block_limit) {
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
			if (!BITARRAY_BIT(block_header, (block_index+1)))
			    msize = TINY_FREE_SIZE(mapped_ptr);
			else
			    msize = 1;

		    } else if (range.address + block_offset != mag_last_free_ptr) {
			msize = 1; 
			bit = block_index + 1;
			while (! BITARRAY_BIT(block_header, bit)) {
			    bit++;
			    msize ++;
			}
			buffer[count].address = range.address + block_offset;
			buffer[count].size = TINY_BYTES_FOR_MSIZE(msize);
			count++;
			if (count >= MAX_RECORDER_BUFFER) {
			    recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE, buffer, count);
			    count = 0;
			}
		    } else {
			// Block is not free but it matches mag_last_free_ptr so even
			// though it is not marked free in the bitmap, we treat it as if
			// it is and move on
			msize = mag_last_free_msize;
		    }

		    if (!msize)
			return KERN_FAILURE; // Somethings amiss. Avoid looping at this block_index.

		    block_index += msize;
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

static void *
tiny_malloc_from_free_list(szone_t *szone, magazine_t *tiny_mag_ptr, mag_index_t mag_index, msize_t msize)
{
    free_list_t		*ptr;
    msize_t		this_msize;
    grain_t		slot = msize - 1;
    free_list_t		**free_list = tiny_mag_ptr->mag_free_list;
    free_list_t		**the_slot = free_list + slot;
    free_list_t		*next;
    free_list_t		**limit;
#if defined(__LP64__)
    uint64_t		bitmap;
#else
    uint32_t		bitmap;
#endif
    msize_t		leftover_msize;
    free_list_t		*leftover_ptr;

    // Assumes we've locked the region
    CHECK_MAGAZINE_PTR_LOCKED(szone, tiny_mag_ptr, __PRETTY_FUNCTION__);
    
    // Look for an exact match by checking the freelist for this msize.
    // 
    ptr = *the_slot;
    if (ptr) {
        next = free_list_unchecksum_ptr(szone, &ptr->next);
        if (next) {
            next->previous = ptr->previous;
        } else {
            BITMAPV_CLR(tiny_mag_ptr->mag_bitmap, slot);
        }
        *the_slot = next;
        this_msize = msize;
#if DEBUG_MALLOC
        if (LOG(szone, ptr)) {
            malloc_printf("in tiny_malloc_from_free_list(), exact match ptr=%p, this_msize=%d\n", ptr, this_msize);
        }
#endif
        goto return_tiny_alloc;
    }

    // Mask off the bits representing slots holding free blocks smaller than the
    // size we need.  If there are no larger free blocks, try allocating from
    // the free space at the end of the tiny region.
#if defined(__LP64__)
    bitmap = ((uint64_t *)(tiny_mag_ptr->mag_bitmap))[0] & ~ ((1ULL << slot) - 1);
#else
    bitmap = tiny_mag_ptr->mag_bitmap[0] & ~ ((1 << slot) - 1);
#endif
    if (!bitmap)
        goto try_tiny_malloc_from_end;
        
    slot = BITMAPV_CTZ(bitmap);
    limit = free_list + NUM_TINY_SLOTS - 1;
    free_list += slot;
    
    if (free_list < limit) {
        ptr = *free_list;
        if (ptr) {
            next = free_list_unchecksum_ptr(szone, &ptr->next);
            *free_list = next;
            if (next) {
                next->previous = ptr->previous;
            } else {
                BITMAPV_CLR(tiny_mag_ptr->mag_bitmap, slot);
            }
            this_msize = get_tiny_free_size(ptr);
            goto add_leftover_and_proceed;
        }
#if DEBUG_MALLOC
	malloc_printf("in tiny_malloc_from_free_list(), mag_bitmap out of sync, slot=%d\n",slot);
#endif
    }

    // We are now looking at the last slot, which contains blocks equal to, or
    // due to coalescing of free blocks, larger than (NUM_TINY_SLOTS - 1) * tiny quantum size.
    // If the last freelist is not empty, and the head contains a block that is
    // larger than our request, then the remainder is put back on the free list.
    ptr = *limit;
    if (ptr) {
        this_msize = get_tiny_free_size(ptr);
        next = free_list_unchecksum_ptr(szone, &ptr->next);
        if (this_msize - msize >= NUM_TINY_SLOTS) {
            // the leftover will go back to the free list, so we optimize by
            // modifying the free list rather than a pop and push of the head
            leftover_msize = this_msize - msize;
            leftover_ptr = (free_list_t *)((unsigned char *)ptr + TINY_BYTES_FOR_MSIZE(msize));
            *limit = leftover_ptr;
            if (next) {
                next->previous.u = free_list_checksum_ptr(szone, leftover_ptr);
            }
            leftover_ptr->previous = ptr->previous;
            leftover_ptr->next = ptr->next;
            set_tiny_meta_header_free(leftover_ptr, leftover_msize);
#if DEBUG_MALLOC
            if (LOG(szone,ptr)) {
            malloc_printf("in tiny_malloc_from_free_list(), last slot ptr=%p, msize=%d this_msize=%d\n", 
			  ptr, msize, this_msize);
            }
#endif
            this_msize = msize;
            goto return_tiny_alloc;
        }
        if (next) {
            next->previous = ptr->previous;
        }
        *limit = next;
        goto add_leftover_and_proceed;
	/* NOTREACHED */
    }

try_tiny_malloc_from_end:
    // Let's see if we can use tiny_mag_ptr->mag_bytes_free_at_end
    if (tiny_mag_ptr->mag_bytes_free_at_end >= TINY_BYTES_FOR_MSIZE(msize)) {
        ptr = (free_list_t *)((uintptr_t)TINY_REGION_END(tiny_mag_ptr->mag_last_region) - 
			      tiny_mag_ptr->mag_bytes_free_at_end);
        tiny_mag_ptr->mag_bytes_free_at_end -= TINY_BYTES_FOR_MSIZE(msize);
        if (tiny_mag_ptr->mag_bytes_free_at_end) {
            // let's add an in use block after ptr to serve as boundary
            set_tiny_meta_header_in_use_1((unsigned char *)ptr + TINY_BYTES_FOR_MSIZE(msize));
        }
        this_msize = msize;
#if DEBUG_MALLOC
        if (LOG(szone, ptr)) {
            malloc_printf("in tiny_malloc_from_free_list(), from end ptr=%p, msize=%d\n", ptr, msize);
        }
#endif
        goto return_tiny_alloc;
    }
#if ASLR_INTERNAL
    // Try from start if nothing left at end
    if (tiny_mag_ptr->mag_bytes_free_at_start >= TINY_BYTES_FOR_MSIZE(msize)) {
        ptr = (free_list_t *)(TINY_REGION_ADDRESS(tiny_mag_ptr->mag_last_region) +
			      tiny_mag_ptr->mag_bytes_free_at_start - TINY_BYTES_FOR_MSIZE(msize));
        tiny_mag_ptr->mag_bytes_free_at_start -= TINY_BYTES_FOR_MSIZE(msize);
        if (tiny_mag_ptr->mag_bytes_free_at_start) {
            // let's add an in use block before ptr to serve as boundary
            set_tiny_meta_header_in_use_1((unsigned char *)ptr - TINY_QUANTUM);
        }
        this_msize = msize;
#if DEBUG_MALLOC
        if (LOG(szone, ptr)) {
            malloc_printf("in tiny_malloc_from_free_list(), from start ptr=%p, msize=%d\n", ptr, msize);
        }
#endif
        goto return_tiny_alloc;
    }
#endif
    return NULL;

add_leftover_and_proceed:
    if (!this_msize || (this_msize > msize)) {
        leftover_msize = this_msize - msize;
        leftover_ptr = (free_list_t *)((unsigned char *)ptr + TINY_BYTES_FOR_MSIZE(msize));
#if DEBUG_MALLOC
        if (LOG(szone,ptr)) {
            malloc_printf("in tiny_malloc_from_free_list(), adding leftover ptr=%p, this_msize=%d\n", ptr, this_msize);
        }
#endif
        tiny_free_list_add_ptr(szone, tiny_mag_ptr, leftover_ptr, leftover_msize);
        this_msize = msize;
    }
    
return_tiny_alloc:
    tiny_mag_ptr->mag_num_objects++;
    tiny_mag_ptr->mag_num_bytes_in_objects += TINY_BYTES_FOR_MSIZE(this_msize);

    // Update this region's bytes in use count
    region_trailer_t *node = REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr));
    size_t bytes_used = node->bytes_used + TINY_BYTES_FOR_MSIZE(this_msize);
    node->bytes_used = bytes_used;

    // Emptiness discriminant
    if (bytes_used < DENSITY_THRESHOLD(TINY_REGION_PAYLOAD_BYTES)) {
        /* After this allocation the region is still sparse, so it must have been even more so before
	   the allocation. That implies the region is already correctly marked. Do nothing. */
    } else {
	/* Region has crossed threshold from sparsity to density. Mark it not "suitable" on the
	   recirculation candidates list. */
	node->recirc_suitable = FALSE;
   }
#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
	malloc_printf("in tiny_malloc_from_free_list(), ptr=%p, this_msize=%d, msize=%d\n", ptr, this_msize, msize);
    }
#endif
    if (this_msize > 1)
	set_tiny_meta_header_in_use(ptr, this_msize);
    else
	set_tiny_meta_header_in_use_1(ptr);
    return ptr;
}
#undef DENSITY_THRESHOLD
#undef K

static INLINE void *
tiny_malloc_should_clear(szone_t *szone, msize_t msize, boolean_t cleared_requested)
{
    void	*ptr;
    mag_index_t	mag_index = mag_get_thread_index(szone);
    magazine_t	*tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);
 
#if DEBUG_MALLOC
    if (DEPOT_MAGAZINE_INDEX == mag_index) {
	szone_error(szone, 1, "malloc called for magazine index -1", NULL, NULL);
	return(NULL);
    }
    
    if (!msize) {
	szone_error(szone, 1, "invariant broken (!msize) in allocation (region)", NULL, NULL);
	return(NULL);
    }
#endif

    SZONE_MAGAZINE_PTR_LOCK(szone, tiny_mag_ptr);

#if TINY_CACHE
    ptr = tiny_mag_ptr->mag_last_free;

    if ((((uintptr_t)ptr) & (TINY_QUANTUM - 1)) == msize) {
	// we have a winner
	tiny_mag_ptr->mag_last_free = NULL;
	tiny_mag_ptr->mag_last_free_rgn = NULL;
	SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	CHECK(szone, __PRETTY_FUNCTION__);
	ptr = (void *)((uintptr_t)ptr & ~ (TINY_QUANTUM - 1));
	if (cleared_requested) {
	    memset(ptr, 0, TINY_BYTES_FOR_MSIZE(msize));
	}
#if DEBUG_MALLOC
	if (LOG(szone,ptr)) {
	    malloc_printf("in tiny_malloc_should_clear(), tiny cache ptr=%p, msize=%d\n", ptr, msize);
	}
#endif
	return ptr;
    }
#endif /* TINY_CACHE */

    while (1) {
	ptr = tiny_malloc_from_free_list(szone, tiny_mag_ptr, mag_index, msize);
	if (ptr) {
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	    CHECK(szone, __PRETTY_FUNCTION__);
	    if (cleared_requested) {
		memset(ptr, 0, TINY_BYTES_FOR_MSIZE(msize));
	    }
	    return ptr;
	}

    if (tiny_get_region_from_depot(szone, tiny_mag_ptr, mag_index, msize)) {
	    ptr = tiny_malloc_from_free_list(szone, tiny_mag_ptr, mag_index, msize);
	    if (ptr) {
		SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		CHECK(szone, __PRETTY_FUNCTION__);
		if (cleared_requested) {
		    memset(ptr, 0, TINY_BYTES_FOR_MSIZE(msize));
		}
		return ptr;
	    }
	}

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
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	    fresh_region = allocate_pages_securely(szone, TINY_REGION_SIZE, TINY_BLOCKS_ALIGN, VM_MEMORY_MALLOC_TINY);    
	    SZONE_MAGAZINE_PTR_LOCK(szone, tiny_mag_ptr);

	    MAGMALLOC_ALLOCREGION((void *)szone, (int)mag_index, fresh_region, TINY_REGION_SIZE); // DTrace USDT Probe

	    if (!fresh_region) { // out of memory!
		tiny_mag_ptr->alloc_underway = FALSE;
		OSMemoryBarrier();
		SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		return NULL;
	    }

	    ptr = tiny_malloc_from_region_no_lock(szone, tiny_mag_ptr, mag_index, msize, fresh_region);
    
	    // we don't clear because this freshly allocated space is pristine
	    tiny_mag_ptr->alloc_underway = FALSE;
	    OSMemoryBarrier();
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	    CHECK(szone, __PRETTY_FUNCTION__);
	   return ptr;
	} else {
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	    pthread_yield_np();
	    SZONE_MAGAZINE_PTR_LOCK(szone, tiny_mag_ptr);
	}
    }
    /* NOTREACHED */
}

static NOINLINE void
free_tiny_botch(szone_t *szone, free_list_t *ptr)
{
    mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr));
    magazine_t	*tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);
    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
    szone_error(szone, 1, "double free", ptr, NULL);
}

static INLINE void
free_tiny(szone_t *szone, void *ptr, region_t tiny_region, size_t known_size)
{
    msize_t	msize;
    boolean_t	is_free;
    mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(tiny_region);
    magazine_t	*tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);

    // ptr is known to be in tiny_region
    if (known_size) {
	msize = TINY_MSIZE_FOR_BYTES(known_size + TINY_QUANTUM - 1);
    } else {
	msize = get_tiny_meta_header(ptr, &is_free);
	if (is_free) {
	    free_tiny_botch(szone, ptr);
	    return;
	}
    }
#if DEBUG_MALLOC
    if (!msize) {
	malloc_printf("*** free_tiny() block in use is too large: %p\n", ptr);
	return;
    }
#endif
    
    SZONE_MAGAZINE_PTR_LOCK(szone, tiny_mag_ptr);

#if TINY_CACHE
    // Depot does not participate in TINY_CACHE since it can't be directly malloc()'d
    if (DEPOT_MAGAZINE_INDEX != mag_index) {
	if (msize < TINY_QUANTUM) { // to see if the bits fit in the last 4 bits
	    void *ptr2 = tiny_mag_ptr->mag_last_free; // Might be NULL
	    region_t rgn2 = tiny_mag_ptr->mag_last_free_rgn;
	
	    /* check that we don't already have this pointer in the cache */
	    if (ptr == (void *)((uintptr_t)ptr2 & ~ (TINY_QUANTUM - 1))) {
		free_tiny_botch(szone, ptr);
		return;
	    }
	    
	    if ((szone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE) && msize)
		memset(ptr, 0x55, TINY_BYTES_FOR_MSIZE(msize));
		
	    tiny_mag_ptr->mag_last_free = (void *)(((uintptr_t)ptr) | msize);
	    tiny_mag_ptr->mag_last_free_rgn = tiny_region;
    
	    if (!ptr2) {
		SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		CHECK(szone, __PRETTY_FUNCTION__);
		return;
	    }
	
	    msize = (uintptr_t)ptr2 & (TINY_QUANTUM - 1);
	    ptr = (void *)(((uintptr_t)ptr2) & ~(TINY_QUANTUM - 1));
	    tiny_region = rgn2;
	}
    }
#endif /* TINY_CACHE */

    // Now in the time it took to acquire the lock, the region may have migrated
    // from one magazine to another. I.e. trailer->mag_index is volatile.
    // In which case the magazine lock we obtained (namely magazines[mag_index].mag_lock) 
    // is stale. If so, keep on tryin' ...
    region_trailer_t *trailer = REGION_TRAILER_FOR_TINY_REGION(tiny_region);
    mag_index_t refreshed_index;
    
    while (mag_index != (refreshed_index = trailer->mag_index)) { // Note assignment
    
	SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	
	mag_index = refreshed_index;
	tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);
	SZONE_MAGAZINE_PTR_LOCK(szone, tiny_mag_ptr);
    }
    			    
    if (tiny_free_no_lock(szone, tiny_mag_ptr, mag_index, tiny_region, ptr, msize))
    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	
    CHECK(szone, __PRETTY_FUNCTION__);
}

static void
print_tiny_free_list(szone_t *szone)
{
    free_list_t	*ptr;
    _SIMPLE_STRING b = _simple_salloc();
    mag_index_t mag_index;

    if (b) {
	_simple_sappend(b, "tiny free sizes:\n");
	for (mag_index = -1; mag_index < szone->num_tiny_magazines; mag_index++) {
	    grain_t	slot = 0;
	    _simple_sprintf(b,"\tMagazine %d: ", mag_index);
	    while (slot < NUM_TINY_SLOTS) {
		ptr = szone->tiny_magazines[mag_index].mag_free_list[slot];
		if (ptr) {
		    _simple_sprintf(b, "%s%y[%d]; ", (slot == NUM_TINY_SLOTS-1) ? ">=" : "", 
				    (slot+1)*TINY_QUANTUM, free_list_count(szone, ptr));
		}
		slot++;
	    }
	    _simple_sappend(b,"\n");
	}
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%s\n", _simple_string(b));
	_simple_sfree(b);
    }
}

static void
print_tiny_region(boolean_t verbose, region_t region, size_t bytes_at_start, size_t bytes_at_end)
{
    unsigned		counts[1024];
    unsigned		in_use = 0;
    uintptr_t		start = (uintptr_t)TINY_REGION_ADDRESS(region);
    uintptr_t		current = start + bytes_at_end;
    uintptr_t		limit =  (uintptr_t)TINY_REGION_END(region) - bytes_at_end;
    boolean_t		is_free;
    msize_t		msize;
    unsigned		ci;
    _SIMPLE_STRING	b;
    uintptr_t		pgTot = 0;
    
    if (region == HASHRING_REGION_DEALLOCATED) {
	if ((b = _simple_salloc()) != NULL) {
	    _simple_sprintf(b, "Tiny region [unknown address] was returned to the OS\n");
	    _malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%s\n", _simple_string(b));
	    _simple_sfree(b);
	}
        return;
    }
    
    memset(counts, 0, sizeof(counts));
    while (current < limit) {
	msize = get_tiny_meta_header((void *)current, &is_free);
	if (is_free & !msize && (current == start)) {
	    // first block is all free
	    uintptr_t pgLo = round_page(start + sizeof(free_list_t) + sizeof(msize_t));
	    uintptr_t pgHi = trunc_page(start + TINY_REGION_SIZE - sizeof(msize_t));
	    
	    if (pgLo < pgHi) {
		pgTot += (pgHi - pgLo);
	    }
	    break;
	}
	if (!msize) {
	    malloc_printf("*** error with %p: msize=%d\n", (void *)current, (unsigned)msize);
	    break;
	}
	if (!is_free) {
	    // block in use
	    if (msize > NUM_TINY_SLOTS)
		malloc_printf("*** error at %p msize for in_use is %d\n", (void *)current, msize);
	    if (msize < 1024)
		counts[msize]++;
	    in_use++;
	} else {
	    uintptr_t pgLo = round_page(current + sizeof(free_list_t) + sizeof(msize_t));
	    uintptr_t pgHi = trunc_page(current + TINY_BYTES_FOR_MSIZE(msize) - sizeof(msize_t));
	    
	    if (pgLo < pgHi) {
		pgTot += (pgHi - pgLo);
	    }
	}
	current += TINY_BYTES_FOR_MSIZE(msize);
    }
    if ((b = _simple_salloc()) != NULL) {
	_simple_sprintf(b, "Tiny region [%p-%p, %y] \t", (void *)start, TINY_REGION_END(region), (int)TINY_REGION_SIZE);
	_simple_sprintf(b, "Magazine=%d \t", MAGAZINE_INDEX_FOR_TINY_REGION(region));
	_simple_sprintf(b, "Allocations in use=%d \t Bytes in use=%ly \t", in_use, BYTES_USED_FOR_TINY_REGION(region));
	if (bytes_at_end || bytes_at_start)
	    _simple_sprintf(b, "Untouched=%ly ", bytes_at_end + bytes_at_start);
	if (DEPOT_MAGAZINE_INDEX == MAGAZINE_INDEX_FOR_TINY_REGION(region)) {
	    _simple_sprintf(b, "Advised MADV_FREE=%ly", pgTot);
	} else {
	    _simple_sprintf(b, "Fragments subject to reclamation=%ly", pgTot);
	}
	if (verbose && in_use) {
	    _simple_sappend(b, "\n\tSizes in use: ");
	    for (ci = 0; ci < 1024; ci++)
		if (counts[ci])
		    _simple_sprintf(b, "%d[%d] ", TINY_BYTES_FOR_MSIZE(ci), counts[ci]);
	}
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%s\n", _simple_string(b));
	_simple_sfree(b);
    }
}

static boolean_t
tiny_free_list_check(szone_t *szone, grain_t slot)
{
    mag_index_t mag_index;

    for (mag_index = -1; mag_index < szone->num_tiny_magazines; mag_index++) {
	magazine_t	*tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);
	SZONE_MAGAZINE_PTR_LOCK(szone, tiny_mag_ptr);

	unsigned	count = 0;
	free_list_t	*ptr = szone->tiny_magazines[mag_index].mag_free_list[slot];
	boolean_t	is_free;
        free_list_t	*previous = NULL;
      
	while (ptr) {
	    is_free = tiny_meta_header_is_free(ptr);
	    if (! is_free) {
		malloc_printf("*** in-use ptr in free list slot=%d count=%d ptr=%p\n", slot, count, ptr);
		SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		return 0;
	    }
	    if (((uintptr_t)ptr) & (TINY_QUANTUM - 1)) {
		malloc_printf("*** unaligned ptr in free list slot=%d  count=%d ptr=%p\n", slot, count, ptr);
		SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		return 0;
	    }
	    if (!tiny_region_for_ptr_no_lock(szone, ptr)) {
		malloc_printf("*** ptr not in szone slot=%d  count=%d ptr=%p\n", slot, count, ptr);
		SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		return 0;
	    }
	    if (free_list_unchecksum_ptr(szone, &ptr->previous) != previous) {
		malloc_printf("*** previous incorrectly set slot=%d  count=%d ptr=%p\n", slot, count, ptr);
		SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		return 0;
	    }
	    previous = ptr;
	    ptr = free_list_unchecksum_ptr(szone, &ptr->next);
	    count++;
	}

	SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
    }
    return 1;
}

/*********************	SMALL FREE LIST UTILITIES	************************/

/*
 * Mark a block as free.  Only the first quantum of a block is marked thusly,
 * the remainder are marked "middle".
 */
static INLINE void
small_meta_header_set_is_free(msize_t *meta_headers, unsigned index, msize_t msize)
{
    meta_headers[index] = msize | SMALL_IS_FREE;
}

/*
 * Mark a block as in use.  Only the first quantum of a block is marked thusly,
 * the remainder are marked "middle".
 */
static INLINE void
small_meta_header_set_in_use(msize_t *meta_headers, msize_t index, msize_t msize)
{
    meta_headers[index] = msize;
}

/*
 * Mark a quantum as being the second or later in a block.
 */
static INLINE void
small_meta_header_set_middle(msize_t *meta_headers, msize_t index)
{
    meta_headers[index] = 0;
}

/* 
 * Adds an item to the proper free list, and also marks the meta-header of the 
 * block properly.
 * Assumes szone has been locked
 */ 
static void
small_free_list_add_ptr(szone_t *szone, magazine_t *small_mag_ptr, void *ptr, msize_t msize)
{
    grain_t	slot = (msize <= szone->num_small_slots) ? msize - 1 : szone->num_small_slots - 1;
    free_list_t	*free_ptr = ptr;
    free_list_t	*free_head = small_mag_ptr->mag_free_list[slot];
    void	*follower;

#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
        malloc_printf("in %s, ptr=%p, msize=%d\n", __FUNCTION__, ptr, msize);
    }
    if (((uintptr_t)ptr) & (SMALL_QUANTUM - 1)) {
        szone_error(szone, 1, "small_free_list_add_ptr: Unaligned ptr", ptr, NULL);
    }
#endif
    small_meta_header_set_is_free(SMALL_META_HEADER_FOR_PTR(ptr), SMALL_META_INDEX_FOR_PTR(ptr), msize);

    if (free_head) {
#if DEBUG_MALLOC
        if (free_list_unchecksum_ptr(szone, &free_head->previous)) {
            szone_error(szone, 1, "small_free_list_add_ptr: Internal invariant broken (free_head->previous)", ptr,
            "ptr=%p slot=%d free_head=%p previous=%p\n", ptr, slot, (void *)free_head, free_head->previous.p);
        }
        if (!SMALL_PTR_IS_FREE(free_head)) {
            szone_error(szone, 1, "small_free_list_add_ptr: Internal invariant broken (free_head is not a free pointer)", ptr,
            "ptr=%p slot=%d free_head=%p\n", ptr, slot, (void *)free_head);
        }
#endif
        free_head->previous.u = free_list_checksum_ptr(szone, free_ptr);
    } else {
        BITMAPN_SET(small_mag_ptr->mag_bitmap, slot);
    }
    free_ptr->previous.u = free_list_checksum_ptr(szone, NULL);
    free_ptr->next.u = free_list_checksum_ptr(szone, free_head);
    
    small_mag_ptr->mag_free_list[slot] = free_ptr;

    // Store msize at the end of the block denoted by "ptr" (i.e. at a negative offset from "follower")
    follower = (void *)((uintptr_t)ptr + SMALL_BYTES_FOR_MSIZE(msize));
    SMALL_PREVIOUS_MSIZE(follower) = msize;
}

/* 
 * Removes the item pointed to by ptr in the proper free list. 
 * Assumes szone has been locked
 */ 
static void
small_free_list_remove_ptr(szone_t *szone, magazine_t *small_mag_ptr, void *ptr, msize_t msize)
{
    grain_t	slot = (msize <= szone->num_small_slots) ? msize - 1 : szone->num_small_slots - 1;
    free_list_t	*free_ptr = ptr, *next, *previous;
    
    next = free_list_unchecksum_ptr(szone, &free_ptr->next);
    previous = free_list_unchecksum_ptr(szone, &free_ptr->previous);

#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
        malloc_printf("In %s, ptr=%p, msize=%d\n", __FUNCTION__, ptr, msize);
    }
#endif

    if (!previous) { 
    // The block to remove is the head of the free list
#if DEBUG_MALLOC
        if (small_mag_ptr->mag_free_list[slot] != ptr) {
            szone_error(szone, 1, "small_free_list_remove_ptr: Internal invariant broken (small_mag_ptr->mag_free_list[slot])", ptr,
              "ptr=%p slot=%d msize=%d small_mag_ptr->mag_free_list[slot]=%p\n",
              ptr, slot, msize, (void *)small_mag_ptr->mag_free_list[slot]);
            return;
        }
#endif
        small_mag_ptr->mag_free_list[slot] = next;
        if (!next) BITMAPN_CLR(small_mag_ptr->mag_bitmap, slot);
    } else {
        // We know free_ptr is already checksummed, so we don't need to do it
        // again.
        previous->next = free_ptr->next;
    }
    if (next) {
        // We know free_ptr is already checksummed, so we don't need to do it
        // again.
        next->previous = free_ptr->previous;
    }
}

/*
 * small_region_for_ptr_no_lock - Returns the small region containing the pointer,
 * or NULL if not found.
 */
static INLINE region_t
small_region_for_ptr_no_lock(szone_t *szone, const void *ptr)
{
    rgnhdl_t r = hash_lookup_region_no_lock(szone->small_region_generation->hashed_regions,
	szone->small_region_generation->num_regions_allocated,
	szone->small_region_generation->num_regions_allocated_shift,
	SMALL_REGION_FOR_PTR(ptr));
    return r ? *r : r;
}

static void
small_finalize_region(szone_t *szone, magazine_t *small_mag_ptr) {
    void      *last_block, *previous_block;
    msize_t   last_msize, previous_msize, last_index;

    // It is possible that the block prior to the last block in the region has
    // been free'd, but was not coalesced with the free bytes at the end of the
    // block, since we treat the bytes at the end of the region as "in use" in
    // the meta headers. Attempt to coalesce the last block with the previous 
    // block, so we don't violate the "no consecutive free blocks" invariant.
    //
    // FIXME: If we could calculate the previous small free size in the same
    //        manner as tiny_previous_preceding_free, it would eliminate the
    //        index & previous msize checks, which are a guard against reading 
    //        bogus data out of in-use or written-on-freed memory.
    //
    // FIXME: Need to investigate how much work would be required to increase
    //        'mag_bytes_free_at_end' when freeing the preceding block, rather
    //        than performing this workaround.
    //
    if (small_mag_ptr->mag_bytes_free_at_end) {
        last_block = SMALL_REGION_END(small_mag_ptr->mag_last_region) - small_mag_ptr->mag_bytes_free_at_end;
        last_msize = SMALL_MSIZE_FOR_BYTES(small_mag_ptr->mag_bytes_free_at_end);

    last_index = SMALL_META_INDEX_FOR_PTR(last_block);
    previous_msize = SMALL_PREVIOUS_MSIZE(last_block);

    if (last_index && (previous_msize <= last_index)) {
        previous_block = (void *)((uintptr_t)last_block - SMALL_BYTES_FOR_MSIZE(previous_msize));
        if (*SMALL_METADATA_FOR_PTR(previous_block) == (previous_msize | SMALL_IS_FREE)) {
            msize_t *meta_headers = SMALL_META_HEADER_FOR_PTR(last_block);
            
            small_meta_header_set_middle(meta_headers, last_index);
            small_free_list_remove_ptr(szone, small_mag_ptr, previous_block, previous_msize);
            last_block = (void *)((uintptr_t)last_block - SMALL_BYTES_FOR_MSIZE(previous_msize));
            last_msize += previous_msize;
        }
    }
    
    // splice last_block into the free list
    small_free_list_add_ptr(szone, small_mag_ptr, last_block, last_msize);
    small_mag_ptr->mag_bytes_free_at_end = 0;
    }

#if ASLR_INTERNAL
    if (small_mag_ptr->mag_bytes_free_at_start) {
        last_block = SMALL_REGION_ADDRESS(small_mag_ptr->mag_last_region);
        last_msize = SMALL_MSIZE_FOR_BYTES(small_mag_ptr->mag_bytes_free_at_start);

        void *next_block = (void *) ((uintptr_t)last_block + small_mag_ptr->mag_bytes_free_at_start);
        if (SMALL_PTR_IS_FREE(next_block)) {
            msize_t next_msize = SMALL_PTR_SIZE(next_block);

            small_meta_header_set_middle(SMALL_META_HEADER_FOR_PTR(next_block), SMALL_META_INDEX_FOR_PTR(next_block));
            small_free_list_remove_ptr(szone, small_mag_ptr, next_block, next_msize);
            last_msize += next_msize;
        }

        // splice last_block into the free list
        small_free_list_add_ptr(szone, small_mag_ptr, last_block, last_msize);
        small_mag_ptr->mag_bytes_free_at_start = 0;
    }
#endif

    // TODO: Will we ever need to coalesce the blocks at the beginning and end when we finalize?

    small_mag_ptr->mag_last_region = NULL;
}

static int
small_free_detach_region(szone_t *szone, magazine_t *small_mag_ptr, region_t r) {
    unsigned char	*ptr = SMALL_REGION_ADDRESS(r);
    msize_t		*meta_headers = SMALL_META_HEADER_FOR_PTR(ptr);
    uintptr_t		start = (uintptr_t)SMALL_REGION_ADDRESS(r);
    uintptr_t		current = start;
    uintptr_t		limit =  (uintptr_t)SMALL_REGION_END(r);
    int			total_alloc = 0;

    while (current < limit) {
	unsigned	index = SMALL_META_INDEX_FOR_PTR(current);
	msize_t		msize_and_free = meta_headers[index];
	boolean_t	is_free = msize_and_free & SMALL_IS_FREE;
	msize_t		msize = msize_and_free & ~ SMALL_IS_FREE;
	
	if (!msize) {
#if DEBUG_MALLOC
	    malloc_printf("*** small_free_detach_region error with %p: msize=%d is_free =%d\n", 
	    (void *)current, msize, is_free);
#endif
	    break;
	}
	if (is_free) {
	    small_free_list_remove_ptr(szone, small_mag_ptr, (void *)current, msize);
	} else {
	    total_alloc++;
	}
	current += SMALL_BYTES_FOR_MSIZE(msize);
    }
    return total_alloc;
}

static size_t
small_free_reattach_region(szone_t *szone, magazine_t *small_mag_ptr, region_t r) {
    unsigned char	*ptr = SMALL_REGION_ADDRESS(r);
    msize_t		*meta_headers = SMALL_META_HEADER_FOR_PTR(ptr);
    uintptr_t		start = (uintptr_t)SMALL_REGION_ADDRESS(r);
    uintptr_t		current = start;
    uintptr_t		limit =  (uintptr_t)SMALL_REGION_END(r);
    size_t		total_alloc = 0;

    while (current < limit) {
	unsigned	index = SMALL_META_INDEX_FOR_PTR(current);
	msize_t		msize_and_free = meta_headers[index];
	boolean_t	is_free = msize_and_free & SMALL_IS_FREE;
	msize_t		msize = msize_and_free & ~ SMALL_IS_FREE;
	
	if (!msize) {
#if DEBUG_MALLOC
	    malloc_printf("*** small_free_reattach_region error with %p: msize=%d is_free =%d\n", 
	    (void *)current, msize, is_free);
#endif
	    break;
	}
	if (is_free) {
	    small_free_list_add_ptr(szone, small_mag_ptr, (void *)current, msize);
        } else {
	    total_alloc += SMALL_BYTES_FOR_MSIZE(msize);
	}
	current += SMALL_BYTES_FOR_MSIZE(msize);
    }
    return total_alloc;
}

typedef struct {
    uint16_t pnum, size;
} small_pg_pair_t;

static void NOINLINE /* want private stack frame for automatic array */
small_free_scan_madvise_free(szone_t *szone, magazine_t *depot_ptr, region_t r) {
    uintptr_t	start = (uintptr_t)SMALL_REGION_ADDRESS(r);
    uintptr_t	current = start;
    uintptr_t	limit =  (uintptr_t)SMALL_REGION_END(r);
    msize_t	*meta_headers = SMALL_META_HEADER_FOR_PTR(start);
    small_pg_pair_t advisory[((SMALL_REGION_PAYLOAD_BYTES + vm_page_size - 1) >> vm_page_shift) >> 1]; // 4096bytes stack allocated
    int		advisories = 0;

    // Scan the metadata identifying blocks which span one or more pages. Mark the pages MADV_FREE taking care to preserve free list
    // management data. 
    while (current < limit) {
	unsigned	index = SMALL_META_INDEX_FOR_PTR(current);
	msize_t		msize_and_free = meta_headers[index];
	boolean_t	is_free = msize_and_free & SMALL_IS_FREE;
	msize_t		msize = msize_and_free & ~ SMALL_IS_FREE;

	if (is_free && !msize && (current == start)) {
#if DEBUG_MALLOC
	    // first block is all free
	    malloc_printf("*** small_free_scan_madvise_free first block is all free! %p: msize=%d is_free =%d\n", 
	    (void *)current, msize, is_free);
#endif	    
	    uintptr_t pgLo = round_page(start + sizeof(free_list_t) + sizeof(msize_t));
	    uintptr_t pgHi = trunc_page(start + SMALL_REGION_SIZE - sizeof(msize_t));
	    
	    if (pgLo < pgHi) {
		advisory[advisories].pnum = (pgLo - start) >> vm_page_shift;
		advisory[advisories].size = (pgHi - pgLo) >> vm_page_shift;
		advisories++;
	    }
	    break;
	}
	if (!msize) {
#if DEBUG_MALLOC
	    malloc_printf("*** small_free_scan_madvise_free error with %p: msize=%d is_free =%d\n", 
	    (void *)current, msize, is_free);
#endif
	    break;
	}
	if (is_free) {
	    uintptr_t pgLo = round_page(current + sizeof(free_list_t) + sizeof(msize_t));
	    uintptr_t pgHi = trunc_page(current + SMALL_BYTES_FOR_MSIZE(msize) - sizeof(msize_t));
	    
	    if (pgLo < pgHi) {
		advisory[advisories].pnum = (pgLo - start) >> vm_page_shift;
		advisory[advisories].size = (pgHi - pgLo) >> vm_page_shift;
		advisories++;
	    }
	}
	current += SMALL_BYTES_FOR_MSIZE(msize);
    }
    
    if (advisories > 0) {
	int i;
	
	OSAtomicIncrement32Barrier(&(REGION_TRAILER_FOR_SMALL_REGION(r)->pinned_to_depot));
	SZONE_MAGAZINE_PTR_UNLOCK(szone, depot_ptr);
	for (i = 0; i < advisories; ++i) {
	    uintptr_t addr = (advisory[i].pnum << vm_page_shift) + start;
	    size_t size = advisory[i].size << vm_page_shift;
	    
#if TARGET_OS_EMBEDDED
		madvise_free_range(szone, r, addr, addr + size, NULL);
#else
		madvise_free_range(szone, r, addr, addr + size);
#endif
	}
	SZONE_MAGAZINE_PTR_LOCK(szone, depot_ptr);
	OSAtomicDecrement32Barrier(&(REGION_TRAILER_FOR_SMALL_REGION(r)->pinned_to_depot));
    }
}

static region_t
small_free_try_depot_unmap_no_lock(szone_t *szone, magazine_t *depot_ptr, region_trailer_t *node)
{
    if (0 < node->bytes_used || 
        0 < node->pinned_to_depot ||
	depot_ptr->recirculation_entries < (szone->num_small_magazines * 2)) {
	return NULL;
    }
    
    // disconnect first node from Depot
    recirc_list_extract(szone, depot_ptr, node);
    
    // Iterate the region pulling its free entries off the (locked) Depot's free list
    region_t sparse_region = SMALL_REGION_FOR_PTR(node);
    int objects_in_use = small_free_detach_region(szone, depot_ptr, sparse_region);
    
    if (0 == objects_in_use) {
	// Invalidate the hash table entry for this region with HASHRING_REGION_DEALLOCATED.
	// Using HASHRING_REGION_DEALLOCATED preserves the collision chain, using HASHRING_OPEN_ENTRY (0) would not.
	rgnhdl_t pSlot = hash_lookup_region_no_lock(szone->small_region_generation->hashed_regions, 
				     szone->small_region_generation->num_regions_allocated, 
				     szone->small_region_generation->num_regions_allocated_shift, sparse_region); 
	if (NULL == pSlot) {
	    szone_error(szone, 1, "small_free_try_depot_unmap_no_lock hash lookup failed:", NULL, "%p\n", sparse_region);
	    return NULL;
	}
	*pSlot = HASHRING_REGION_DEALLOCATED;
	depot_ptr->num_bytes_in_magazine -= SMALL_REGION_PAYLOAD_BYTES;
	__sync_fetch_and_add( &(szone->num_small_regions_dealloc), 1); // Atomically increment num_small_regions_dealloc
	
	// Caller will transfer ownership of the region back to the OS with no locks held
	MAGMALLOC_DEALLOCREGION((void *)szone, (void *)sparse_region, SMALL_REGION_SIZE); // DTrace USDT Probe
	return sparse_region;
	
    } else {
	szone_error(szone, 1, "small_free_try_depot_unmap_no_lock objects_in_use not zero:", NULL, "%d\n", objects_in_use);
	return NULL;
    }
}

static boolean_t
small_free_do_recirc_to_depot(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index)
{
	// The entire magazine crossed the "emptiness threshold". Transfer a region
	// from this magazine to the Depot. Choose a region that itself has crossed the emptiness threshold (i.e
	// is at least fraction "f" empty.) Such a region will be marked "suitable" on the recirculation list.
	region_trailer_t *node = small_mag_ptr->firstNode;
	
	while (node && !node->recirc_suitable) {
	    node = node->next;
	}
	
	if (NULL == node) {
#if DEBUG_MALLOC
	    malloc_printf("*** small_free_do_recirc_to_depot end of list\n");
#endif
	    return TRUE; // Caller must SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	}
	
	region_t sparse_region = SMALL_REGION_FOR_PTR(node);

	// Deal with unclaimed memory -- mag_bytes_free_at_end or mag_bytes_free_at start
	if (sparse_region == small_mag_ptr->mag_last_region && (small_mag_ptr->mag_bytes_free_at_end || small_mag_ptr->mag_bytes_free_at_start)) {
	    small_finalize_region(szone, small_mag_ptr);
	}
	
	// disconnect "suitable" node from magazine
	recirc_list_extract(szone, small_mag_ptr, node);

	// Iterate the region pulling its free entries off its (locked) magazine's free list
	int objects_in_use = small_free_detach_region(szone, small_mag_ptr, sparse_region);
	magazine_t *depot_ptr = &(szone->small_magazines[DEPOT_MAGAZINE_INDEX]);
	
	// hand over the region to the (locked) Depot
	SZONE_MAGAZINE_PTR_LOCK(szone,depot_ptr);
	// this will cause small_free_list_add_ptr called by small_free_reattach_region to use 
	// the depot as its target magazine, rather than magazine formerly associated with sparse_region
	MAGAZINE_INDEX_FOR_SMALL_REGION(sparse_region) = DEPOT_MAGAZINE_INDEX;
	node->pinned_to_depot = 0;

	// Iterate the region putting its free entries on Depot's free list
	size_t bytes_inplay = small_free_reattach_region(szone, depot_ptr, sparse_region);
	
	small_mag_ptr->mag_num_bytes_in_objects -= bytes_inplay;
	small_mag_ptr->num_bytes_in_magazine -= SMALL_REGION_PAYLOAD_BYTES;
	small_mag_ptr->mag_num_objects -= objects_in_use;
	
	SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr); // Unlock the originating magazine
	
	depot_ptr->mag_num_bytes_in_objects += bytes_inplay;
	depot_ptr->num_bytes_in_magazine += SMALL_REGION_PAYLOAD_BYTES;
	depot_ptr->mag_num_objects += objects_in_use;
	
	// connect to Depot as last node
	recirc_list_splice_last(szone, depot_ptr, node);

	MAGMALLOC_RECIRCREGION((void *)szone, (int)mag_index, (void *)sparse_region, SMALL_REGION_SIZE, 
	    (int)BYTES_USED_FOR_SMALL_REGION(sparse_region)); // DTrace USDT Probe

	// Mark free'd dirty pages with MADV_FREE to reduce memory pressure
	small_free_scan_madvise_free(szone, depot_ptr, sparse_region);

	// If the region is entirely empty vm_deallocate() it outside the depot lock
	region_t r_dealloc = small_free_try_depot_unmap_no_lock(szone, depot_ptr, node);
	SZONE_MAGAZINE_PTR_UNLOCK(szone,depot_ptr);
	if (r_dealloc)
	    deallocate_pages(szone, r_dealloc, SMALL_REGION_SIZE, 0);
	return FALSE; // Caller need not unlock the originating magazine
}    

static region_t
small_find_msize_region(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, msize_t msize)
{
    free_list_t		*ptr;
    grain_t		slot = (msize <= szone->num_small_slots) ? msize - 1 : szone->num_small_slots - 1;
    free_list_t		**free_list = small_mag_ptr->mag_free_list;
    free_list_t		**the_slot = free_list + slot;
    free_list_t		**limit;
    unsigned		bitmap;

    // Assumes we've locked the magazine
    CHECK_MAGAZINE_PTR_LOCKED(szone, small_mag_ptr, __PRETTY_FUNCTION__);

    // Look for an exact match by checking the freelist for this msize.
    ptr = *the_slot;
    if (ptr)
	return SMALL_REGION_FOR_PTR(ptr);
	 
    // Mask off the bits representing slots holding free blocks smaller than
    // the size we need.
    if (szone->is_largemem) {
	// BITMAPN_CTZ implementation
	unsigned idx = slot >> 5;
	bitmap = 0;
	unsigned mask = ~ ((1 << (slot & 31)) - 1);
	for ( ; idx < SMALL_BITMAP_WORDS; ++idx ) {
	    bitmap = small_mag_ptr->mag_bitmap[idx] & mask;
	    if (bitmap != 0)
		break;
	    mask = ~0U;
	}
	// Check for fallthrough: No bits set in bitmap
	if ((bitmap == 0) && (idx == SMALL_BITMAP_WORDS))
            return NULL;

	// Start looking at the first set bit, plus 32 bits for every word of
	// zeroes or entries that were too small.
	slot = BITMAP32_CTZ((&bitmap)) + (idx * 32);
    } else {
 	bitmap = small_mag_ptr->mag_bitmap[0] & ~ ((1 << slot) - 1);
	if (!bitmap) 
	    return NULL;

	slot = BITMAP32_CTZ((&bitmap));
    }
    limit = free_list + szone->num_small_slots - 1;
    free_list += slot;

    if (free_list < limit) {
        ptr = *free_list;
        if (ptr) 
	    return SMALL_REGION_FOR_PTR(ptr);
	else {
	    /* Shouldn't happen. Fall through to look at last slot. */
#if DEBUG_MALLOC
	    malloc_printf("in small_malloc_from_free_list(), mag_bitmap out of sync, slot=%d\n",slot);
#endif
	}
    }

    // We are now looking at the last slot, which contains blocks equal to, or
    // due to coalescing of free blocks, larger than (num_small_slots - 1) * (small quantum size).
    ptr = *limit;
    if (ptr) 
	return SMALL_REGION_FOR_PTR(ptr);
    
    return NULL;
}

static boolean_t
small_get_region_from_depot(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, msize_t msize)
{
    magazine_t *depot_ptr = &(szone->small_magazines[DEPOT_MAGAZINE_INDEX]);

    /* FIXME: Would Uniprocessor benefit from recirc and MADV_FREE? */
    if (szone->num_small_magazines == 1) // Uniprocessor, single magazine, so no recirculation necessary
	return 0;
	
#if DEBUG_MALLOC
    if (DEPOT_MAGAZINE_INDEX == mag_index) {
	szone_error(szone, 1, "small_get_region_from_depot called for magazine index -1", NULL, NULL);
	return 0;
    }
#endif
	
    SZONE_MAGAZINE_PTR_LOCK(szone,depot_ptr);

    // Appropriate a Depot'd region that can satisfy requested msize.
    region_trailer_t *node;
    region_t sparse_region;
	
    while (1) {
	sparse_region = small_find_msize_region(szone, depot_ptr, DEPOT_MAGAZINE_INDEX, msize);
	if (NULL == sparse_region) { // Depot empty?
            SZONE_MAGAZINE_PTR_UNLOCK(szone,depot_ptr);
	    return 0;
        }
    
	node = REGION_TRAILER_FOR_SMALL_REGION(sparse_region);
	if (0 >= node->pinned_to_depot)
	    break;
	
	SZONE_MAGAZINE_PTR_UNLOCK(szone,depot_ptr);
	pthread_yield_np();
	SZONE_MAGAZINE_PTR_LOCK(szone,depot_ptr);
    }

    // disconnect node from Depot
    recirc_list_extract(szone, depot_ptr, node);
    
    // Iterate the region pulling its free entries off the (locked) Depot's free list
    int objects_in_use = small_free_detach_region(szone, depot_ptr, sparse_region);

    // Transfer ownership of the region
    MAGAZINE_INDEX_FOR_SMALL_REGION(sparse_region) = mag_index;
    node->pinned_to_depot = 0;
    
    // Iterate the region putting its free entries on its new (locked) magazine's free list
    size_t bytes_inplay = small_free_reattach_region(szone, small_mag_ptr, sparse_region);
    
    depot_ptr->mag_num_bytes_in_objects -= bytes_inplay;
    depot_ptr->num_bytes_in_magazine -= SMALL_REGION_PAYLOAD_BYTES;
    depot_ptr->mag_num_objects -= objects_in_use;
    
    small_mag_ptr->mag_num_bytes_in_objects += bytes_inplay;
    small_mag_ptr->num_bytes_in_magazine += SMALL_REGION_PAYLOAD_BYTES;
    small_mag_ptr->mag_num_objects += objects_in_use;
    
    // connect to magazine as first node
    recirc_list_splice_first(szone, small_mag_ptr, node);
    	
    SZONE_MAGAZINE_PTR_UNLOCK(szone,depot_ptr);

    // madvise() outside the Depot lock
#if TARGET_OS_EMBEDDED
    if (node->failedREUSE) {
#else
    if (node->failedREUSE ||
	-1 == madvise((void *)sparse_region, SMALL_REGION_PAYLOAD_BYTES, MADV_FREE_REUSE)) {
#endif
	/* -1 return: VM map entry change makes this unfit for reuse. Something evil lurks. */
#if DEBUG_MADVISE
	szone_error(szone, 0, "small_get_region_from_depot madvise(..., MADV_FREE_REUSE) failed", 
	    sparse_region, "length=%d\n", SMALL_REGION_PAYLOAD_BYTES);
#endif
	node->failedREUSE = TRUE;
    }

    MAGMALLOC_DEPOTREGION((void *)szone, (int)mag_index, (void *)sparse_region, SMALL_REGION_SIZE, 
        (int)BYTES_USED_FOR_SMALL_REGION(sparse_region)); // DTrace USDT Probe
	
    return 1;
}

#define K 1.5 // headroom measured in number of 8Mb regions
#define DENSITY_THRESHOLD(a) \
    ((a) - ((a) >> 2)) // "Emptiness" f = 0.25, so "Density" is (1 - f)*a. Generally: ((a) - ((a) >> -log2(f))) 

static INLINE boolean_t
small_free_no_lock(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, region_t region, void *ptr, msize_t msize)
{
    msize_t		*meta_headers = SMALL_META_HEADER_FOR_PTR(ptr);
    unsigned		index = SMALL_META_INDEX_FOR_PTR(ptr);
    void		*original_ptr = ptr;
    size_t		original_size = SMALL_BYTES_FOR_MSIZE(msize);
    unsigned char	*next_block = ((unsigned char *)ptr + original_size);
    msize_t		next_index = index + msize;
    msize_t		previous_msize, next_msize;
    void		*previous;

#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
	malloc_printf("in small_free_no_lock(), ptr=%p, msize=%d\n", ptr, msize);
    }
    if (!msize) {
	szone_error(szone, 1, "trying to free small block that is too small", ptr,
	    "in small_free_no_lock(), ptr=%p, msize=%d\n", ptr, msize);
    }
#endif

    // We try to coalesce this block with the preceeding one
    if (index && (SMALL_PREVIOUS_MSIZE(ptr) <= index)) {
	previous_msize = SMALL_PREVIOUS_MSIZE(ptr);
	if (meta_headers[index - previous_msize] == (previous_msize | SMALL_IS_FREE)) {
	    previous = (void *)((uintptr_t)ptr - SMALL_BYTES_FOR_MSIZE(previous_msize));
	    // previous is really to be coalesced
#if DEBUG_MALLOC
	    if (LOG(szone, ptr) || LOG(szone,previous)) { 
		malloc_printf("in small_free_no_lock(), coalesced backwards for %p previous=%p\n", ptr, previous);
	    }
#endif
	    small_free_list_remove_ptr(szone, small_mag_ptr, previous, previous_msize);
	    small_meta_header_set_middle(meta_headers, index);
	    ptr = previous;
	    msize += previous_msize;
	    index -= previous_msize;
	}
    }
    // We try to coalesce with the next block
    if ((next_block < SMALL_REGION_END(region)) && (meta_headers[next_index] & SMALL_IS_FREE)) {
	// next block is free, we coalesce
	next_msize = meta_headers[next_index] & ~ SMALL_IS_FREE;
#if DEBUG_MALLOC
	if (LOG(szone,ptr)) 
	    malloc_printf("In small_free_no_lock(), for ptr=%p, msize=%d coalesced next block=%p next_msize=%d\n",
			  ptr, msize, next_block, next_msize);
#endif
	small_free_list_remove_ptr(szone, small_mag_ptr, next_block, next_msize);
	small_meta_header_set_middle(meta_headers, next_index);
	msize += next_msize;
    }
    if (szone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE) {
	if (!msize) {
	    szone_error(szone, 1, "incorrect size information - block header was damaged", ptr, NULL);
	} else {
	    memset(ptr, 0x55, SMALL_BYTES_FOR_MSIZE(msize));
	}
    }
    small_free_list_add_ptr(szone, small_mag_ptr, ptr, msize);
    small_mag_ptr->mag_num_objects--;
    // we use original_size and not msize to avoid double counting the coalesced blocks
    small_mag_ptr->mag_num_bytes_in_objects -= original_size;
    
    // Update this region's bytes in use count
    region_trailer_t *node = REGION_TRAILER_FOR_SMALL_REGION(region);
    size_t bytes_used = node->bytes_used - original_size;
    node->bytes_used = bytes_used;
    
#if !TARGET_OS_EMBEDDED // Always madvise for embedded platforms
    /* FIXME: Would Uniprocessor benefit from recirc and MADV_FREE? */
    if (szone->num_small_magazines == 1) { // Uniprocessor, single magazine, so no recirculation necessary
	/* NOTHING */
    } else if (DEPOT_MAGAZINE_INDEX != mag_index) {
	// Emptiness discriminant
	if (bytes_used < DENSITY_THRESHOLD(SMALL_REGION_PAYLOAD_BYTES)) {
	    /* Region has crossed threshold from density to sparsity. Mark it "suitable" on the
	       recirculation candidates list. */
	    node->recirc_suitable = TRUE;
	} else {
	    /* After this free, we've found the region is still dense, so it must have been even more so before
	       the free. That implies the region is already correctly marked. Do nothing. */
	}
    
	// Has the entire magazine crossed the "emptiness threshold"? If so, transfer a region
	// from this magazine to the Depot. Choose a region that itself has crossed the emptiness threshold (i.e
	// is at least fraction "f" empty.) Such a region will be marked "suitable" on the recirculation list.

	size_t a = small_mag_ptr->num_bytes_in_magazine; // Total bytes allocated to this magazine
	size_t u = small_mag_ptr->mag_num_bytes_in_objects; // In use (malloc'd) from this magaqzine

	if (a - u > ((3 * SMALL_REGION_PAYLOAD_BYTES) / 2) && u < DENSITY_THRESHOLD(a)) {
	    return small_free_do_recirc_to_depot(szone, small_mag_ptr, mag_index);
	}

    } else {
#endif
	// Freed to Depot. N.B. Lock on small_magazines[DEPOT_MAGAZINE_INDEX] is already held
	// Calcuate the first page in the coalesced block that would be safe to mark MADV_FREE
	uintptr_t safe_ptr = (uintptr_t)ptr + sizeof(free_list_t) + sizeof(msize_t);
	uintptr_t round_safe = round_page(safe_ptr);
	
	// Calcuate the last page in the coalesced block that would be safe to mark MADV_FREE
	uintptr_t safe_extent = (uintptr_t)ptr + SMALL_BYTES_FOR_MSIZE(msize) - sizeof(msize_t);
	uintptr_t trunc_extent = trunc_page(safe_extent);
		
	// The newly freed block may complete a span of bytes that cover one or more pages. Mark the span with MADV_FREE.
	if (round_safe < trunc_extent) { // Safe area covers a page (perhaps many)
            uintptr_t lo = trunc_page((uintptr_t)original_ptr);
            uintptr_t hi = round_page((uintptr_t)original_ptr + original_size);

	    small_free_list_remove_ptr(szone, small_mag_ptr, ptr, msize);
	    small_meta_header_set_in_use(meta_headers, index, msize);

	    OSAtomicIncrement32Barrier(&(node->pinned_to_depot));
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
#if TARGET_OS_EMBEDDED
	    madvise_free_range(szone, region, MAX(round_safe, lo), MIN(trunc_extent, hi), &szone->last_small_advise);
#else
	    madvise_free_range(szone, region, MAX(round_safe, lo), MIN(trunc_extent, hi));
#endif
	    SZONE_MAGAZINE_PTR_LOCK(szone, small_mag_ptr);
	    OSAtomicDecrement32Barrier(&(node->pinned_to_depot));

	    small_meta_header_set_is_free(meta_headers, index, msize);
	    small_free_list_add_ptr(szone, small_mag_ptr, ptr, msize);
	}

#if !TARGET_OS_EMBEDDED
	if (0 < bytes_used || 0 < node->pinned_to_depot) {
	    /* Depot'd region is still live. Leave it in place on the Depot's recirculation list
	       so as to avoid thrashing between the Depot's free list and a magazines's free list
	       with detach_region/reattach_region */
	} else {
	    /* Depot'd region is just now empty. Consider return to OS. */
	    region_t r_dealloc = small_free_try_depot_unmap_no_lock(szone, small_mag_ptr, node);
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	    if (r_dealloc)
		deallocate_pages(szone, r_dealloc, SMALL_REGION_SIZE, 0);
	    return FALSE; // Caller need not unlock
	}
    }
#endif

    return TRUE; // Caller must do SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr)
}

// Allocates from the last region or a freshly allocated region
static void *
small_malloc_from_region_no_lock(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, 
    msize_t msize, void *aligned_address)
{
    void	*ptr;

    // Before anything we transform the mag_bytes_free_at_end or mag_bytes_free_at_start - if any - to a regular free block
    /* FIXME: last_block needs to be coalesced with previous entry if free, <rdar://5462322> */
    if (small_mag_ptr->mag_bytes_free_at_end || small_mag_ptr->mag_bytes_free_at_start)
	small_finalize_region(szone, small_mag_ptr);

    // Here find the only place in smallville that (infrequently) takes the small_regions_lock.
    // Only one thread at a time should be permitted to assess the density of the hash
    // ring and adjust if needed. 
    // Only one thread at a time should be permitted to insert its new region on
    // the hash ring.
    // It is safe for all other threads to read the hash ring (hashed_regions) and
    // the associated sizes (num_regions_allocated and num_small_regions).
    
    LOCK(szone->small_regions_lock);
    // Check to see if the hash ring of small regions needs to grow.  Try to
    // avoid the hash ring becoming too dense.
    if (szone->small_region_generation->num_regions_allocated < (2 * szone->num_small_regions)) {
	region_t *new_regions;
	size_t new_size;
	size_t new_shift = szone->small_region_generation->num_regions_allocated_shift; // In/Out parameter
	new_regions = hash_regions_grow_no_lock(szone, szone->small_region_generation->hashed_regions,
					      szone->small_region_generation->num_regions_allocated,
					      &new_shift,
					      &new_size);
	// Do not deallocate the current hashed_regions allocation since someone
	// may be iterating it.  Instead, just leak it.

	// Prepare to advance to the "next generation" of the hash ring.
	szone->small_region_generation->nextgen->hashed_regions = new_regions;
	szone->small_region_generation->nextgen->num_regions_allocated = new_size;
	szone->small_region_generation->nextgen->num_regions_allocated_shift = new_shift;

	// Throw the switch to atomically advance to the next generation.
	szone->small_region_generation = szone->small_region_generation->nextgen;
	// Ensure everyone sees the advance.
	OSMemoryBarrier();
    }
    // Tag the region at "aligned_address" as belonging to us, 
    // and so put it under the protection of the magazine lock we are holding.
    // Do this before advertising "aligned_address" on the hash ring(!)
    MAGAZINE_INDEX_FOR_SMALL_REGION(aligned_address) = mag_index;

    // Insert the new region into the hash ring, and update malloc statistics
    hash_region_insert_no_lock(szone->small_region_generation->hashed_regions, 
                               szone->small_region_generation->num_regions_allocated,
			       szone->small_region_generation->num_regions_allocated_shift,
                               aligned_address);
                               
    szone->num_small_regions++;

    UNLOCK(szone->small_regions_lock);

    small_mag_ptr->mag_last_region = aligned_address;
    BYTES_USED_FOR_SMALL_REGION(aligned_address) = SMALL_BYTES_FOR_MSIZE(msize);
#if ASLR_INTERNAL
    int offset_msize = malloc_entropy[1] & SMALL_ENTROPY_MASK;
#if DEBUG_MALLOC
    if (getenv("MallocASLRForce")) offset_msize = strtol(getenv("MallocASLRForce"), NULL, 0) & SMALL_ENTROPY_MASK;
    if (getenv("MallocASLRPrint")) malloc_printf("Region: %p offset: %d\n", aligned_address, offset_msize);
#endif
#else
    int offset_msize = 0;
#endif
    ptr = (void *)((uintptr_t) aligned_address + SMALL_BYTES_FOR_MSIZE(offset_msize));
    small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(ptr), offset_msize, msize);
    small_mag_ptr->mag_num_objects++;
    small_mag_ptr->mag_num_bytes_in_objects += SMALL_BYTES_FOR_MSIZE(msize);
    small_mag_ptr->num_bytes_in_magazine += SMALL_REGION_PAYLOAD_BYTES;
    
    // add a big free block at the end
    small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(ptr), offset_msize + msize, NUM_SMALL_BLOCKS - msize - offset_msize);    
    small_mag_ptr->mag_bytes_free_at_end = SMALL_BYTES_FOR_MSIZE(NUM_SMALL_BLOCKS - msize - offset_msize);

#if ASLR_INTERNAL
    // add a big free block at the start
    small_mag_ptr->mag_bytes_free_at_start = SMALL_BYTES_FOR_MSIZE(offset_msize);
    if (offset_msize) {
	small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(ptr), 0, offset_msize);
    }
#else
    small_mag_ptr->mag_bytes_free_at_start = 0;
#endif

    // connect to magazine as last node
    recirc_list_splice_last(szone, small_mag_ptr, REGION_TRAILER_FOR_SMALL_REGION(aligned_address));

    return ptr;
}

static INLINE void *
small_try_shrink_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_good_size)
{
    msize_t new_msize = SMALL_MSIZE_FOR_BYTES(new_good_size);
    msize_t mshrinkage = SMALL_MSIZE_FOR_BYTES(old_size) - new_msize;

    if (mshrinkage) {
	void *q = (void *)((uintptr_t)ptr + SMALL_BYTES_FOR_MSIZE(new_msize));
	magazine_t *small_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->small_magazines,
		    REGION_TRAILER_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr)), 
		    MAGAZINE_INDEX_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr)));
		    
	// Mark q as block header and in-use, thus creating two blocks.
	small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(ptr), SMALL_META_INDEX_FOR_PTR(ptr), new_msize);
	small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(q), SMALL_META_INDEX_FOR_PTR(q), mshrinkage);
	small_mag_ptr->mag_num_objects++;

	SZONE_MAGAZINE_PTR_UNLOCK(szone,small_mag_ptr);
	szone_free(szone, q); // avoid inlining free_small(szone, q, ...);
    }

    return ptr;
}

static INLINE boolean_t
small_try_realloc_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_size)
{
    // returns 1 on success
    msize_t	*meta_headers = SMALL_META_HEADER_FOR_PTR(ptr);
    unsigned	index;
    msize_t	old_msize, new_msize;
    unsigned	next_index;
    void	*next_block;
    msize_t	next_msize_and_free;
    boolean_t	is_free;
    msize_t	next_msize, leftover_msize;
    void	*leftover;

    index = SMALL_META_INDEX_FOR_PTR(ptr);
    old_msize = SMALL_MSIZE_FOR_BYTES(old_size);
    new_msize = SMALL_MSIZE_FOR_BYTES(new_size + SMALL_QUANTUM - 1);
    next_index = index + old_msize;
    
    if (next_index >= NUM_SMALL_BLOCKS) {
	return 0;
    }
    next_block = (char *)ptr + old_size;
    
#if DEBUG_MALLOC
    if ((uintptr_t)next_block & (SMALL_QUANTUM - 1)) {
	szone_error(szone, 1, "internal invariant broken in realloc(next_block)", next_block, NULL);
    }
    if (meta_headers[index] != old_msize)
	malloc_printf("*** small_try_realloc_in_place incorrect old %d %d\n",
	  meta_headers[index], old_msize);
#endif

    magazine_t	*small_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->small_magazines,
				    REGION_TRAILER_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr)), 
				    MAGAZINE_INDEX_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr)));    

    /*
     * Look for a free block immediately afterwards.  If it's large enough, we can consume (part of)
     * it.
     */
    next_msize_and_free = meta_headers[next_index];
    is_free = next_msize_and_free & SMALL_IS_FREE;
    if (!is_free) {
	SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	return 0; // next_block is in use;
    }
    next_msize = next_msize_and_free & ~ SMALL_IS_FREE;
    if (old_msize + next_msize < new_msize) {
	SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	return 0; // even with next block, not enough
    }
    /*
     * The following block is big enough; pull it from its freelist and chop off enough to satisfy
     * our needs.
     */
    small_free_list_remove_ptr(szone, small_mag_ptr, next_block, next_msize);
    small_meta_header_set_middle(meta_headers, next_index);
    leftover_msize = old_msize + next_msize - new_msize;
    if (leftover_msize) {
	/* there's some left, so put the remainder back */
	leftover = (unsigned char *)ptr + SMALL_BYTES_FOR_MSIZE(new_msize);
	
	small_free_list_add_ptr(szone, small_mag_ptr, leftover, leftover_msize);
    }
#if DEBUG_MALLOC
    if (SMALL_BYTES_FOR_MSIZE(new_msize) > szone->large_threshold) {
	malloc_printf("*** realloc in place for %p exceeded msize=%d\n", new_msize);
    }
#endif
    small_meta_header_set_in_use(meta_headers, index, new_msize);
#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
	malloc_printf("in small_try_realloc_in_place(), ptr=%p, msize=%d\n", ptr, *SMALL_METADATA_FOR_PTR(ptr));
    }
#endif
    small_mag_ptr->mag_num_bytes_in_objects += SMALL_BYTES_FOR_MSIZE(new_msize - old_msize);

    // Update this region's bytes in use count
    region_trailer_t *node = REGION_TRAILER_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr));
    size_t bytes_used = node->bytes_used + SMALL_BYTES_FOR_MSIZE(new_msize - old_msize);
    node->bytes_used = bytes_used;

    // Emptiness discriminant
    if (bytes_used < DENSITY_THRESHOLD(SMALL_REGION_PAYLOAD_BYTES)) {
        /* After this reallocation the region is still sparse, so it must have been even more so before
	   the reallocation. That implies the region is already correctly marked. Do nothing. */
    } else {
	/* Region has crossed threshold from sparsity to density. Mark it not "suitable" on the
	   recirculation candidates list. */
	node->recirc_suitable = FALSE;
   }

    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
    CHECK(szone, __PRETTY_FUNCTION__);
    return 1;
}

static boolean_t
small_check_region(szone_t *szone, region_t region)
{
    unsigned char	*ptr = SMALL_REGION_ADDRESS(region);
    msize_t		*meta_headers = SMALL_META_HEADER_FOR_PTR(ptr);
    unsigned char	*region_end = SMALL_REGION_END(region);
    msize_t		prev_free = 0;
    unsigned		index;
    msize_t		msize_and_free;
    msize_t		msize;
    free_list_t		*free_head;
    void		*previous, *next;
    msize_t		*follower;
    mag_index_t		mag_index = MAGAZINE_INDEX_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr));
    magazine_t		*small_mag_ptr = &(szone->small_magazines[mag_index]);

    // Assumes locked
    CHECK_MAGAZINE_PTR_LOCKED(szone, small_mag_ptr, __PRETTY_FUNCTION__);

    if (region == small_mag_ptr->mag_last_region) {
	ptr += small_mag_ptr->mag_bytes_free_at_start;
	region_end -= small_mag_ptr->mag_bytes_free_at_end;
    }
	
    while (ptr < region_end) {
	index = SMALL_META_INDEX_FOR_PTR(ptr);
	msize_and_free = meta_headers[index];
	if (!(msize_and_free & SMALL_IS_FREE)) {
	    // block is in use
	    msize = msize_and_free;
	    if (!msize) {
		malloc_printf("*** invariant broken: null msize ptr=%p num_small_regions=%d end=%p\n",
		  ptr, szone->num_small_regions, region_end);
		return 0;
	    }
#if !RELAXED_INVARIANT_CHECKS
	    if (SMALL_BYTES_FOR_MSIZE(msize) > szone->large_threshold) {
		malloc_printf("*** invariant broken for %p this small msize=%d - size is too large\n",
		  ptr, msize_and_free);
		return 0;
	    }
#endif // RELAXED_INVARIANT_CHECKS
	    ptr += SMALL_BYTES_FOR_MSIZE(msize);
	    prev_free = 0;
	} else {
	    // free pointer
	    msize = msize_and_free & ~ SMALL_IS_FREE;
	    free_head = (free_list_t *)ptr;
	    follower = (msize_t *)FOLLOWING_SMALL_PTR(ptr, msize);
	    if (!msize) {
		malloc_printf("*** invariant broken for free block %p this msize=%d\n", ptr, msize);
		return 0;
	    }
#if !RELAXED_INVARIANT_CHECKS
	    if (prev_free) {
		malloc_printf("*** invariant broken for %p (2 free in a row)\n", ptr);
		return 0;
	    }
#endif
	    previous = free_list_unchecksum_ptr(szone, &free_head->previous);
	    next = free_list_unchecksum_ptr(szone, &free_head->next);
	    if (previous && !SMALL_PTR_IS_FREE(previous)) {
		malloc_printf("*** invariant broken for %p (previous %p is not a free pointer)\n",
		  ptr, free_head->previous);
		return 0;
	    }
	    if (next && !SMALL_PTR_IS_FREE(next)) {
		malloc_printf("*** invariant broken for %p (next is not a free pointer)\n", ptr);
		return 0;
	    }
	    if (SMALL_PREVIOUS_MSIZE(follower) != msize) {
		malloc_printf("*** invariant broken for small free %p followed by %p in region [%p-%p] "
		  "(end marker incorrect) should be %d; in fact %d\n",
		  ptr, follower, SMALL_REGION_ADDRESS(region), region_end, msize, SMALL_PREVIOUS_MSIZE(follower));
		return 0;
	    }
	    ptr = (unsigned char *)follower;
	    prev_free = SMALL_IS_FREE;
	}
    }
    return 1;
}

static kern_return_t
small_in_use_enumerator(task_t task, void *context, unsigned type_mask, szone_t *szone, 
			memory_reader_t reader, vm_range_recorder_t recorder)
{
    size_t		num_regions;
    size_t		index;
    region_t		*regions;
    vm_range_t		buffer[MAX_RECORDER_BUFFER];
    unsigned		count = 0;
    kern_return_t	err;
    region_t		region;
    vm_range_t		range;
    vm_range_t		admin_range;
    vm_range_t		ptr_range;
    unsigned char	*mapped_region;
    msize_t		*block_header;
    unsigned		block_index;
    unsigned		block_limit;
    msize_t		msize_and_free;
    msize_t		msize;
    magazine_t          *small_mag_base = NULL;
    
    region_hash_generation_t *srg_ptr;
    err = reader(task, (vm_address_t)szone->small_region_generation, sizeof(region_hash_generation_t), (void **)&srg_ptr);
    if (err) return err;

    num_regions = srg_ptr->num_regions_allocated;
    err = reader(task, (vm_address_t)srg_ptr->hashed_regions, sizeof(region_t) * num_regions, (void **)&regions);
    if (err) return err;

    if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
        // Map in all active magazines. Do this outside the iteration over regions.
        err = reader(task, (vm_address_t)(szone->small_magazines),
                szone->num_small_magazines*sizeof(magazine_t),(void **)&small_mag_base);
        if (err) return err;
    }
    
    for (index = 0; index < num_regions; ++index) {
	region = regions[index];
	if (HASHRING_OPEN_ENTRY != region && HASHRING_REGION_DEALLOCATED != region) {
	    range.address = (vm_address_t)SMALL_REGION_ADDRESS(region);
	    range.size = SMALL_REGION_SIZE;
	    if (type_mask & MALLOC_ADMIN_REGION_RANGE_TYPE) {
		admin_range.address = range.address + SMALL_METADATA_START;
		admin_range.size = SMALL_METADATA_SIZE;
		recorder(task, context, MALLOC_ADMIN_REGION_RANGE_TYPE, &admin_range, 1);
	    }
	    if (type_mask & (MALLOC_PTR_REGION_RANGE_TYPE | MALLOC_ADMIN_REGION_RANGE_TYPE)) {
		ptr_range.address = range.address;
		ptr_range.size = NUM_SMALL_BLOCKS * SMALL_QUANTUM;
		recorder(task, context, MALLOC_PTR_REGION_RANGE_TYPE, &ptr_range, 1);
	    }
	    if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
		void 		*mag_last_free;
		vm_address_t	mag_last_free_ptr = 0;
		msize_t		mag_last_free_msize = 0;

		err = reader(task, range.address, range.size, (void **)&mapped_region);
		if (err) 
		    return err;
		    
		mag_index_t mag_index = MAGAZINE_INDEX_FOR_SMALL_REGION(mapped_region);
		magazine_t *small_mag_ptr = small_mag_base + mag_index;

		if (DEPOT_MAGAZINE_INDEX != mag_index) {
		    mag_last_free = small_mag_ptr->mag_last_free;
		    if (mag_last_free) {
			mag_last_free_ptr = (uintptr_t) mag_last_free & ~(SMALL_QUANTUM - 1);
			mag_last_free_msize = (uintptr_t) mag_last_free & (SMALL_QUANTUM - 1);
		    }
		} else {
		    for (mag_index = 0; mag_index < szone->num_small_magazines; mag_index++) {
			if ((void *)range.address == (small_mag_base + mag_index)->mag_last_free_rgn) {
			    mag_last_free = (small_mag_base + mag_index)->mag_last_free;
			    if (mag_last_free) {
				mag_last_free_ptr = (uintptr_t) mag_last_free & ~(SMALL_QUANTUM - 1);
				mag_last_free_msize = (uintptr_t) mag_last_free & (SMALL_QUANTUM - 1);
			    }
			}
		    }
		}

		block_header = (msize_t *)(mapped_region + SMALL_METADATA_START + sizeof(region_trailer_t));
		block_index = 0;
		block_limit = NUM_SMALL_BLOCKS;
		if (region == small_mag_ptr->mag_last_region) {
		    block_index += SMALL_MSIZE_FOR_BYTES(small_mag_ptr->mag_bytes_free_at_start);
		    block_limit -= SMALL_MSIZE_FOR_BYTES(small_mag_ptr->mag_bytes_free_at_end);
		}
		while (block_index < block_limit) {
		    msize_and_free = block_header[block_index];
		    msize = msize_and_free & ~ SMALL_IS_FREE;
		    if (! (msize_and_free & SMALL_IS_FREE) &&
			range.address + SMALL_BYTES_FOR_MSIZE(block_index) != mag_last_free_ptr) {
			// Block in use
			buffer[count].address = range.address + SMALL_BYTES_FOR_MSIZE(block_index);
			buffer[count].size = SMALL_BYTES_FOR_MSIZE(msize);
			count++;
			if (count >= MAX_RECORDER_BUFFER) {
			    recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE, buffer, count);
			    count = 0;
			}
		    }

		    if (!msize)
			return KERN_FAILURE; // Somethings amiss. Avoid looping at this block_index.

		    block_index += msize;
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

static void *
small_malloc_from_free_list(szone_t *szone, magazine_t *small_mag_ptr, mag_index_t mag_index, msize_t msize)
{
    free_list_t		*ptr;
    msize_t		this_msize;
    grain_t		slot = (msize <= szone->num_small_slots) ? msize - 1 : szone->num_small_slots - 1;
    free_list_t		**free_list = small_mag_ptr->mag_free_list;
    free_list_t		**the_slot = free_list + slot;
    free_list_t		*next;
    free_list_t		**limit;
    unsigned		bitmap;
    msize_t		leftover_msize;
    free_list_t		*leftover_ptr;

    // Assumes we've locked the region
    CHECK_MAGAZINE_PTR_LOCKED(szone, small_mag_ptr, __PRETTY_FUNCTION__);

    // Look for an exact match by checking the freelist for this msize.
    // 
    ptr = *the_slot;
    if (ptr) {
        next = free_list_unchecksum_ptr(szone, &ptr->next);
        if (next) {
            next->previous = ptr->previous;
        } else {
            BITMAPN_CLR(small_mag_ptr->mag_bitmap, slot);
        }
        *the_slot = next;
        this_msize = msize;
        goto return_small_alloc;
    }

    // Mask off the bits representing slots holding free blocks smaller than
    // the size we need.  If there are no larger free blocks, try allocating
    // from the free space at the end of the small region.
    if (szone->is_largemem) {
	// BITMAPN_CTZ implementation
	unsigned idx = slot >> 5;
	bitmap = 0;
	unsigned mask = ~ ((1 << (slot & 31)) - 1);
	for ( ; idx < SMALL_BITMAP_WORDS; ++idx ) {
	    bitmap = small_mag_ptr->mag_bitmap[idx] & mask;
	    if (bitmap != 0)
		break;
	    mask = ~0U;
	}
	// Check for fallthrough: No bits set in bitmap
	if ((bitmap == 0) && (idx == SMALL_BITMAP_WORDS))
            goto try_small_from_end;

	// Start looking at the first set bit, plus 32 bits for every word of
	// zeroes or entries that were too small.
	slot = BITMAP32_CTZ((&bitmap)) + (idx * 32);
    } else {
 	bitmap = small_mag_ptr->mag_bitmap[0] & ~ ((1 << slot) - 1);
	if (!bitmap) 
	    goto try_small_from_end;

	slot = BITMAP32_CTZ((&bitmap));
    }
    // FIXME: Explain use of - 1 here, last slot has special meaning
    limit = free_list + szone->num_small_slots - 1;
    free_list += slot;

    if (free_list < limit) {
        ptr = *free_list;
        if (ptr) {

            next = free_list_unchecksum_ptr(szone, &ptr->next);
            *free_list = next;
            if (next) {
                next->previous = ptr->previous;
            } else {
                BITMAPN_CLR(small_mag_ptr->mag_bitmap, slot);
            }
            this_msize = SMALL_PTR_SIZE(ptr);
            goto add_leftover_and_proceed;
        }
#if DEBUG_MALLOC
	malloc_printf("in small_malloc_from_free_list(), mag_bitmap out of sync, slot=%d\n",slot);
#endif
    }

    // We are now looking at the last slot, which contains blocks equal to, or
    // due to coalescing of free blocks, larger than (num_small_slots - 1) * (small quantum size).
    // If the last freelist is not empty, and the head contains a block that is
    // larger than our request, then the remainder is put back on the free list.
    //
    ptr = *limit;
    if (ptr) {
        this_msize = SMALL_PTR_SIZE(ptr);
        next = free_list_unchecksum_ptr(szone, &ptr->next);
        if (this_msize - msize >= szone->num_small_slots) {
            // the leftover will go back to the free list, so we optimize by
            // modifying the free list rather than a pop and push of the head
            leftover_msize = this_msize - msize;
            leftover_ptr = (free_list_t *)((unsigned char *)ptr + SMALL_BYTES_FOR_MSIZE(msize));
            *limit = leftover_ptr;
            if (next) {
                next->previous.u = free_list_checksum_ptr(szone, leftover_ptr);
            }
            leftover_ptr->previous = ptr->previous;
            leftover_ptr->next = ptr->next;
	    small_meta_header_set_is_free(SMALL_META_HEADER_FOR_PTR(leftover_ptr),
					  SMALL_META_INDEX_FOR_PTR(leftover_ptr), leftover_msize);
	    // Store msize at the end of the block denoted by "leftover_ptr" (i.e. at a negative offset from follower)
	    SMALL_PREVIOUS_MSIZE(FOLLOWING_SMALL_PTR(leftover_ptr, leftover_msize)) = leftover_msize; // Access is safe
#if DEBUG_MALLOC
            if (LOG(szone,ptr)) {
            malloc_printf("in small_malloc_from_free_list(), last slot ptr=%p, msize=%d this_msize=%d\n", ptr, msize, this_msize);
            }
#endif
            this_msize = msize;
            goto return_small_alloc;
        }
        if (next) {
            next->previous = ptr->previous;
        }
        *limit = next;
        goto add_leftover_and_proceed;
    }
    
try_small_from_end:
    // Let's see if we can use small_mag_ptr->mag_bytes_free_at_end
    if (small_mag_ptr->mag_bytes_free_at_end >= SMALL_BYTES_FOR_MSIZE(msize)) {
        ptr = (free_list_t *)(SMALL_REGION_END(small_mag_ptr->mag_last_region) -
			      small_mag_ptr->mag_bytes_free_at_end);
        small_mag_ptr->mag_bytes_free_at_end -= SMALL_BYTES_FOR_MSIZE(msize);
        if (small_mag_ptr->mag_bytes_free_at_end) {
            // let's mark this block as in use to serve as boundary
	    small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(ptr),
					 SMALL_META_INDEX_FOR_PTR((unsigned char *)ptr + SMALL_BYTES_FOR_MSIZE(msize)),
					 SMALL_MSIZE_FOR_BYTES(small_mag_ptr->mag_bytes_free_at_end));
        }
        this_msize = msize;
        goto return_small_alloc;
    }
#if ASLR_INTERNAL
    // Try from start if nothing left at end
    if (small_mag_ptr->mag_bytes_free_at_start >= SMALL_BYTES_FOR_MSIZE(msize)) {
        ptr = (free_list_t *)(SMALL_REGION_ADDRESS(small_mag_ptr->mag_last_region) +
			      small_mag_ptr->mag_bytes_free_at_start - SMALL_BYTES_FOR_MSIZE(msize));
        small_mag_ptr->mag_bytes_free_at_start -= SMALL_BYTES_FOR_MSIZE(msize);
        if (small_mag_ptr->mag_bytes_free_at_start) {
            // let's mark this block as in use to serve as boundary
	    small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(ptr), 0, SMALL_MSIZE_FOR_BYTES(small_mag_ptr->mag_bytes_free_at_start));
        }
        this_msize = msize;
        goto return_small_alloc;
    }
#endif
    return NULL;
    
add_leftover_and_proceed:
    if (this_msize > msize) {
        leftover_msize = this_msize - msize;
        leftover_ptr = (free_list_t *)((unsigned char *)ptr + SMALL_BYTES_FOR_MSIZE(msize));
#if DEBUG_MALLOC
        if (LOG(szone,ptr)) {
            malloc_printf("in small_malloc_from_free_list(), adding leftover ptr=%p, this_msize=%d\n", ptr, this_msize);
        }
#endif
        small_free_list_add_ptr(szone, small_mag_ptr, leftover_ptr, leftover_msize);
        this_msize = msize;
    }
    
return_small_alloc:
    small_mag_ptr->mag_num_objects++;
    small_mag_ptr->mag_num_bytes_in_objects += SMALL_BYTES_FOR_MSIZE(this_msize);
    
    // Update this region's bytes in use count
    region_trailer_t *node = REGION_TRAILER_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr));
    size_t bytes_used = node->bytes_used + SMALL_BYTES_FOR_MSIZE(this_msize);
    node->bytes_used = bytes_used;

    // Emptiness discriminant
    if (bytes_used < DENSITY_THRESHOLD(SMALL_REGION_PAYLOAD_BYTES)) {
        /* After this allocation the region is still sparse, so it must have been even more so before
	   the allocation. That implies the region is already correctly marked. Do nothing. */
    } else {
	/* Region has crossed threshold from sparsity to density. Mark in not "suitable" on the
	   recirculation candidates list. */
	node->recirc_suitable = FALSE;
   }
#if DEBUG_MALLOC
    if (LOG(szone,ptr)) {
	malloc_printf("in small_malloc_from_free_list(), ptr=%p, this_msize=%d, msize=%d\n", ptr, this_msize, msize);
    }
#endif
    small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(ptr), SMALL_META_INDEX_FOR_PTR(ptr), this_msize);
    return ptr;
}
#undef DENSITY_THRESHOLD
#undef K

static INLINE void *
small_malloc_should_clear(szone_t *szone, msize_t msize, boolean_t cleared_requested)
{
    void	*ptr;
    mag_index_t	mag_index = mag_get_thread_index(szone);
    magazine_t	*small_mag_ptr = &(szone->small_magazines[mag_index]);
    
    SZONE_MAGAZINE_PTR_LOCK(szone, small_mag_ptr);
    
#if SMALL_CACHE
    ptr = (void *)small_mag_ptr->mag_last_free;
    
    if ((((uintptr_t)ptr) & (SMALL_QUANTUM - 1)) == msize) {
	// we have a winner
	small_mag_ptr->mag_last_free = NULL;
	small_mag_ptr->mag_last_free_rgn = NULL;
	SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	CHECK(szone, __PRETTY_FUNCTION__);
	ptr = (void *)((uintptr_t)ptr & ~ (SMALL_QUANTUM - 1));
	if (cleared_requested) {
	    memset(ptr, 0, SMALL_BYTES_FOR_MSIZE(msize));
	}
	return ptr;
    }
#endif /* SMALL_CACHE */

    while(1) {
	ptr = small_malloc_from_free_list(szone, small_mag_ptr, mag_index, msize);
	if (ptr) {
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	    CHECK(szone, __PRETTY_FUNCTION__);
	    if (cleared_requested) {
		memset(ptr, 0, SMALL_BYTES_FOR_MSIZE(msize));
	    }
	    return ptr;
	}

    if (small_get_region_from_depot(szone, small_mag_ptr, mag_index, msize)) {
	    ptr = small_malloc_from_free_list(szone, small_mag_ptr, mag_index, msize);
	    if (ptr) {
		SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
		CHECK(szone, __PRETTY_FUNCTION__);
		if (cleared_requested) {
		    memset(ptr, 0, SMALL_BYTES_FOR_MSIZE(msize));
		}
		return ptr;
	    }
	}

	// The magazine is exhausted. A new region (heap) must be allocated to satisfy this call to malloc().
	// The allocation, an mmap() system call, will be performed outside the magazine spin locks by the first
	// thread that suffers the exhaustion. That thread sets "alloc_underway" and enters a critical section.
	// Threads arriving here later are excluded from the critical section, yield the CPU, and then retry the
	// allocation. After some time the magazine is resupplied, the original thread leaves with its allocation,
	// and retry-ing threads succeed in the code just above. 
	if (!small_mag_ptr->alloc_underway) {
	    void *fresh_region;

	    // time to create a new region (do this outside the magazine lock)
	    small_mag_ptr->alloc_underway = TRUE;
	    OSMemoryBarrier();
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	    fresh_region = allocate_pages_securely(szone, SMALL_REGION_SIZE, SMALL_BLOCKS_ALIGN, VM_MEMORY_MALLOC_SMALL);    
	    SZONE_MAGAZINE_PTR_LOCK(szone, small_mag_ptr);
	    
	    MAGMALLOC_ALLOCREGION((void *)szone, (int)mag_index, fresh_region, SMALL_REGION_SIZE); // DTrace USDT Probe

	    if (!fresh_region) { // out of memory!
		small_mag_ptr->alloc_underway = FALSE;
		OSMemoryBarrier();
		SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
		return NULL;
	    }
		
	    ptr = small_malloc_from_region_no_lock(szone, small_mag_ptr, mag_index, msize, fresh_region);
	    
	    // we don't clear because this freshly allocated space is pristine
	    small_mag_ptr->alloc_underway = FALSE;
	    OSMemoryBarrier();
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	    CHECK(szone, __PRETTY_FUNCTION__);
	    return ptr;
	} else {
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	    pthread_yield_np();
	    SZONE_MAGAZINE_PTR_LOCK(szone, small_mag_ptr);
	}
    }
    /* NOTREACHED */
}

static NOINLINE void
free_small_botch(szone_t *szone, free_list_t *ptr)
{
    mag_index_t	mag_index = MAGAZINE_INDEX_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr));
    magazine_t	*small_mag_ptr = &(szone->small_magazines[mag_index]);
    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
    szone_error(szone, 1, "double free", ptr, NULL);
}

static INLINE void
free_small(szone_t *szone, void *ptr, region_t small_region, size_t known_size)
{
    msize_t	msize;
    mag_index_t	mag_index = MAGAZINE_INDEX_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr));
    magazine_t	*small_mag_ptr = &(szone->small_magazines[mag_index]);
    
    // ptr is known to be in small_region
    if (known_size) {
	msize = SMALL_MSIZE_FOR_BYTES(known_size + SMALL_QUANTUM - 1);
    } else {
	msize = SMALL_PTR_SIZE(ptr);
	if (SMALL_PTR_IS_FREE(ptr)) {
	    free_small_botch(szone, ptr);
	    return;
	}
    }
    
    SZONE_MAGAZINE_PTR_LOCK(szone, small_mag_ptr);

#if SMALL_CACHE
    // Depot does not participate in SMALL_CACHE since it can't be directly malloc()'d
    if (DEPOT_MAGAZINE_INDEX != mag_index) {
	
	void *ptr2 = small_mag_ptr->mag_last_free; // Might be NULL
	region_t rgn2 = small_mag_ptr->mag_last_free_rgn;
	
	/* check that we don't already have this pointer in the cache */
	if (ptr == (void *)((uintptr_t)ptr2 & ~ (SMALL_QUANTUM - 1))) {
	    free_small_botch(szone, ptr);
	    return;
	}

	if ((szone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE) && msize)
	    memset(ptr, 0x55, SMALL_BYTES_FOR_MSIZE(msize));

	small_mag_ptr->mag_last_free = (void *)(((uintptr_t)ptr) | msize);
	small_mag_ptr->mag_last_free_rgn = small_region;

	if (!ptr2) {
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	    CHECK(szone, __PRETTY_FUNCTION__);
	    return;
	}

	msize = (uintptr_t)ptr2 & (SMALL_QUANTUM - 1);
	ptr = (void *)(((uintptr_t)ptr2) & ~(SMALL_QUANTUM - 1));
	small_region = rgn2;
    }
#endif /* SMALL_CACHE */

    // Now in the time it took to acquire the lock, the region may have migrated
    // from one magazine to another. I.e. trailer->mag_index is volatile.
    // In which case the magazine lock we obtained (namely magazines[mag_index].mag_lock) 
    // is stale. If so, keep on tryin' ...
    region_trailer_t *trailer = REGION_TRAILER_FOR_SMALL_REGION(small_region);
    mag_index_t refreshed_index;
    
    while (mag_index != (refreshed_index = trailer->mag_index)) { // Note assignment
    
	SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	
	mag_index = refreshed_index;
	small_mag_ptr = &(szone->small_magazines[mag_index]);
	SZONE_MAGAZINE_PTR_LOCK(szone, small_mag_ptr);
    }
    			    
    if (small_free_no_lock(szone, small_mag_ptr, mag_index, small_region, ptr, msize))
    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	
    CHECK(szone, __PRETTY_FUNCTION__);
}

static void
print_small_free_list(szone_t *szone)
{
    free_list_t		*ptr;
    _SIMPLE_STRING	b = _simple_salloc();
    mag_index_t mag_index;
    
    if (b) {
	_simple_sappend(b, "small free sizes:\n");
	for (mag_index = -1; mag_index < szone->num_small_magazines; mag_index++) {
	    grain_t	slot = 0;
	    _simple_sprintf(b,"\tMagazine %d: ", mag_index);
	    while (slot < szone->num_small_slots) {
		ptr = szone->small_magazines[mag_index].mag_free_list[slot];
		if (ptr) {
		    _simple_sprintf(b, "%s%y[%d]; ", (slot == szone->num_small_slots-1) ? ">=" : "", 
				(slot + 1) * SMALL_QUANTUM, free_list_count(szone, ptr));
		}
		slot++;
	    }
	    _simple_sappend(b,"\n");
	}
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%s\n", _simple_string(b));
	_simple_sfree(b);
    }
}

static void
print_small_region(szone_t *szone, boolean_t verbose, region_t region, size_t bytes_at_start, size_t bytes_at_end)
{
    unsigned		counts[1024];
    unsigned		in_use = 0;
    uintptr_t		start = (uintptr_t)SMALL_REGION_ADDRESS(region);
    uintptr_t		current = start + bytes_at_start;
    uintptr_t		limit = (uintptr_t)SMALL_REGION_END(region) - bytes_at_end;
    msize_t		msize_and_free;
    msize_t		msize;
    unsigned		ci;
    _SIMPLE_STRING	b;
    uintptr_t		pgTot = 0;

    if (region == HASHRING_REGION_DEALLOCATED) {
	if ((b = _simple_salloc()) != NULL) {
	    _simple_sprintf(b, "Small region [unknown address] was returned to the OS\n");
	    _malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%s\n", _simple_string(b));
	    _simple_sfree(b);
	}
        return;
    }

    memset(counts, 0, sizeof(counts));
    while (current < limit) {
	msize_and_free = *SMALL_METADATA_FOR_PTR(current);
	msize = msize_and_free & ~ SMALL_IS_FREE;
	if (!msize) {
	    malloc_printf("*** error with %p: msize=%d\n", (void *)current, (unsigned)msize);
	    break;
	}
	if (!(msize_and_free & SMALL_IS_FREE)) {
	    // block in use
	    if (msize < 1024)
		counts[msize]++;
	    in_use++;
	} else {
	    uintptr_t pgLo = round_page(current + sizeof(free_list_t) + sizeof(msize_t));
	    uintptr_t pgHi = trunc_page(current + SMALL_BYTES_FOR_MSIZE(msize) - sizeof(msize_t));
	    
	    if (pgLo < pgHi) {
		pgTot += (pgHi - pgLo);
	    }
	}
	current += SMALL_BYTES_FOR_MSIZE(msize);
    }
    if ((b = _simple_salloc()) != NULL) {
	_simple_sprintf(b, "Small region [%p-%p, %y] \t", (void *)start, SMALL_REGION_END(region), (int)SMALL_REGION_SIZE);
	_simple_sprintf(b, "Magazine=%d \t", MAGAZINE_INDEX_FOR_SMALL_REGION(region));
	_simple_sprintf(b, "Allocations in use=%d \t Bytes in use=%ly \t", in_use, BYTES_USED_FOR_SMALL_REGION(region));
	if (bytes_at_end || bytes_at_start)
	    _simple_sprintf(b, "Untouched=%ly ", bytes_at_end + bytes_at_start);
	if (DEPOT_MAGAZINE_INDEX == MAGAZINE_INDEX_FOR_SMALL_REGION(region)) {
	    _simple_sprintf(b, "Advised MADV_FREE=%ly", pgTot);
	} else {
	    _simple_sprintf(b, "Fragments subject to reclamation=%ly", pgTot);
	}
	if (verbose && in_use) {
	    _simple_sappend(b, "\n\tSizes in use: "); 
	    for (ci = 0; ci < 1024; ci++)
		if (counts[ci])
		    _simple_sprintf(b, "%d[%d] ", SMALL_BYTES_FOR_MSIZE(ci), counts[ci]);
	}
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%s\n", _simple_string(b));
	_simple_sfree(b);
    }
}

static boolean_t
small_free_list_check(szone_t *szone, grain_t slot)
{
    mag_index_t mag_index;

    for (mag_index = -1; mag_index < szone->num_small_magazines; mag_index++) {
	magazine_t	*small_mag_ptr = &(szone->small_magazines[mag_index]);
	SZONE_MAGAZINE_PTR_LOCK(szone, small_mag_ptr);

	unsigned	count = 0;
	free_list_t	*ptr = szone->small_magazines[mag_index].mag_free_list[slot];
	msize_t		msize_and_free;
	free_list_t	*previous = NULL;
      
	while (ptr) {
            msize_and_free = *SMALL_METADATA_FOR_PTR(ptr);
            if (!(msize_and_free & SMALL_IS_FREE)) {
		malloc_printf("*** in-use ptr in free list slot=%d count=%d ptr=%p\n", slot, count, ptr);
		SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
		return 0;
            }
            if (((uintptr_t)ptr) & (SMALL_QUANTUM - 1)) {
		malloc_printf("*** unaligned ptr in free list slot=%d  count=%d ptr=%p\n", slot, count, ptr);
		SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
		return 0;
            }
            if (!small_region_for_ptr_no_lock(szone, ptr)) {
		malloc_printf("*** ptr not in szone slot=%d  count=%d ptr=%p\n", slot, count, ptr);
		SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
		return 0;
            }
            if (free_list_unchecksum_ptr(szone, &ptr->previous) != previous) {
		malloc_printf("*** previous incorrectly set slot=%d  count=%d ptr=%p\n", slot, count, ptr);
		SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
		return 0;
            }
            previous = ptr;
            ptr = free_list_unchecksum_ptr(szone, &ptr->next);
            count++;
	}
    
	SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
    }
    return 1;
}

/*******************************************************************************
 * Large allocator implementation
 ******************************************************************************/
#pragma mark large allocator

#if DEBUG_MALLOC

static void
large_debug_print(szone_t *szone)
{
    unsigned		index;
    large_entry_t	*range;
    _SIMPLE_STRING	b = _simple_salloc();

    if (b) {
	for (index = 0, range = szone->large_entries; index < szone->num_large_entries; index++, range++)
	    if (range->address)
		_simple_sprintf(b, "%d: %p(%y);  ", index, range->address, range->size);

	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%s\n", _simple_string(b));
	_simple_sfree(b);
    }
}
#endif

/*
 * Scan the hash ring looking for an entry for the given pointer.
 */
static large_entry_t *
large_entry_for_pointer_no_lock(szone_t *szone, const void *ptr)
{
    // result only valid with lock held
    unsigned		num_large_entries = szone->num_large_entries;
    unsigned		hash_index;
    unsigned		index;
    large_entry_t	*range;

    if (!num_large_entries)
	return NULL;
	
    hash_index = ((uintptr_t)ptr >> vm_page_shift) % num_large_entries;
    index = hash_index;
    
    do {
	range = szone->large_entries + index;
	if (range->address == (vm_address_t)ptr)
	    return range;
	if (0 == range->address)
	    return NULL; // end of chain
	index++;
	if (index == num_large_entries)
	    index = 0;
    } while (index != hash_index);
    
    return NULL;
}

static void
large_entry_insert_no_lock(szone_t *szone, large_entry_t range)
{
    unsigned		num_large_entries = szone->num_large_entries;
    unsigned		hash_index = (((uintptr_t)(range.address)) >> vm_page_shift) % num_large_entries;
    unsigned		index = hash_index;
    large_entry_t	*entry;

    // assert(szone->num_large_objects_in_use < szone->num_large_entries); /* must be called with room to spare */
    
    do {
	entry = szone->large_entries + index;
	if (0 == entry->address) {
	    *entry = range;
	    return; // end of chain
	}
	index++;
	if (index == num_large_entries)
	    index = 0;
    } while (index != hash_index);
    
    // assert(0); /* must not fallthrough! */
}

// FIXME: can't we simply swap the (now empty) entry with the last entry on the collision chain for this hash slot?
static INLINE void
large_entries_rehash_after_entry_no_lock(szone_t *szone, large_entry_t *entry)
{
    unsigned		num_large_entries = szone->num_large_entries;
    unsigned		hash_index = entry - szone->large_entries;
    unsigned		index = hash_index;
    large_entry_t	range;

    // assert(entry->address == 0) /* caller must have cleared *entry */
    
    do {
	index++;
	if (index == num_large_entries)
	    index = 0;
	range = szone->large_entries[index];
	if (0 == range.address)
	    return;
	szone->large_entries[index].address = (vm_address_t)0;
	szone->large_entries[index].size = 0;
	szone->large_entries[index].did_madvise_reusable = FALSE;
	large_entry_insert_no_lock(szone, range); // this will reinsert in the
						  // proper place
    } while (index != hash_index);
    
    // assert(0); /* since entry->address == 0, must not fallthrough! */
}

// FIXME: num should probably be a size_t, since you can theoretically allocate
// more than 2^32-1 large_threshold objects in 64 bit.
static INLINE large_entry_t *
large_entries_alloc_no_lock(szone_t *szone, unsigned num)
{
    size_t	size = num * sizeof(large_entry_t);
    
    // Note that we allocate memory (via a system call) under a spin lock
    // That is certainly evil, however it's very rare in the lifetime of a process
    // The alternative would slow down the normal case
    return allocate_pages(szone, round_page(size), 0, 0, VM_MEMORY_MALLOC_LARGE);
}

static void
large_entries_free_no_lock(szone_t *szone, large_entry_t *entries, unsigned num, vm_range_t *range_to_deallocate)
{
    size_t	size = num * sizeof(large_entry_t);

    range_to_deallocate->address = (vm_address_t)entries;
    range_to_deallocate->size = round_page(size);
}

static large_entry_t *
large_entries_grow_no_lock(szone_t *szone, vm_range_t *range_to_deallocate)
{
    // sets range_to_deallocate
    unsigned		old_num_entries = szone->num_large_entries;
    large_entry_t	*old_entries = szone->large_entries;
    // always an odd number for good hashing
    unsigned		new_num_entries = (old_num_entries) ? old_num_entries * 2 + 1 : 
							      ((vm_page_size / sizeof(large_entry_t)) - 1);
    large_entry_t	*new_entries = large_entries_alloc_no_lock(szone, new_num_entries);
    unsigned		index = old_num_entries;
    large_entry_t	oldRange;

    // if the allocation of new entries failed, bail
    if (new_entries == NULL)
	return NULL;

    szone->num_large_entries = new_num_entries;
    szone->large_entries = new_entries;

    /* rehash entries into the new list */
    while (index--) {
	oldRange = old_entries[index];
	if (oldRange.address) {
	    large_entry_insert_no_lock(szone, oldRange);
	}
    }
    
    if (old_entries) {
	large_entries_free_no_lock(szone, old_entries, old_num_entries, range_to_deallocate);
    } else {
	range_to_deallocate->address = (vm_address_t)0;
	range_to_deallocate->size = 0;
    }
    
    return new_entries;
}

// frees the specific entry in the size table
// returns a range to truly deallocate
static vm_range_t
large_entry_free_no_lock(szone_t *szone, large_entry_t *entry)
{
    vm_range_t		range;
    
    range.address = entry->address;
    range.size = entry->size;
    
    if (szone->debug_flags & SCALABLE_MALLOC_ADD_GUARD_PAGES) {
	protect((void *)range.address, range.size, PROT_READ | PROT_WRITE, szone->debug_flags);
	range.address -= vm_page_size;
	range.size += 2 * vm_page_size;
    }
    
    entry->address = 0;
    entry->size = 0;
    entry->did_madvise_reusable = FALSE;
    large_entries_rehash_after_entry_no_lock(szone, entry);
    
#if DEBUG_MALLOC
    if (large_entry_for_pointer_no_lock(szone, (void *)range.address)) {
	malloc_printf("*** freed entry %p still in use; num_large_entries=%d\n",
	  range.address, szone->num_large_entries);
	large_debug_print(szone);
	szone_sleep();
    }
#endif
    return range;
}

static NOINLINE kern_return_t
large_in_use_enumerator(task_t task, void *context, unsigned type_mask, vm_address_t large_entries_address,
			unsigned num_entries, memory_reader_t reader, vm_range_recorder_t recorder)
{
    unsigned		index = 0;
    vm_range_t		buffer[MAX_RECORDER_BUFFER];
    unsigned		count = 0;
    large_entry_t	*entries;
    kern_return_t	err;
    vm_range_t		range;
    large_entry_t	entry;

    err = reader(task, large_entries_address, sizeof(large_entry_t) * num_entries, (void **)&entries);
    if (err)
	return err;
	
    index = num_entries;
    if (type_mask & MALLOC_ADMIN_REGION_RANGE_TYPE) {
	range.address = large_entries_address;
	range.size = round_page(num_entries * sizeof(large_entry_t));
	recorder(task, context, MALLOC_ADMIN_REGION_RANGE_TYPE, &range, 1);
    }
    if (type_mask & (MALLOC_PTR_IN_USE_RANGE_TYPE | MALLOC_PTR_REGION_RANGE_TYPE)) {
	while (index--) {
	    entry = entries[index];
	    if (entry.address) {
		range.address = entry.address;
		range.size = entry.size;
		buffer[count++] = range;
		if (count >= MAX_RECORDER_BUFFER) {
		    recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE | MALLOC_PTR_REGION_RANGE_TYPE,
			     buffer, count);
		    count = 0;
		}
	    }
	}
    }
    if (count) {
	recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE | MALLOC_PTR_REGION_RANGE_TYPE, 
		 buffer, count);
    }
    return 0;
}

static void *
large_malloc(szone_t *szone, size_t num_pages, unsigned char alignment,
	boolean_t cleared_requested)
{
    void		*addr;
    vm_range_t		range_to_deallocate;
    size_t		size;
    large_entry_t	large_entry;
    
    if (!num_pages)
	num_pages = 1; // minimal allocation size for this szone
    size = (size_t)num_pages << vm_page_shift;
    range_to_deallocate.size = 0;
    range_to_deallocate.address = 0;
    
#if LARGE_CACHE
    if (size < LARGE_CACHE_SIZE_ENTRY_LIMIT) { // Look for a large_entry_t on the death-row cache?    
    SZONE_LOCK(szone);

    int i, best = -1, idx = szone->large_entry_cache_newest, stop_idx = szone->large_entry_cache_oldest;
    size_t best_size = SIZE_T_MAX;
    
    while (1) { // Scan large_entry_cache for best fit, starting with most recent entry
	size_t this_size = szone->large_entry_cache[idx].size;
	addr = (void *)szone->large_entry_cache[idx].address;

	if (0 == alignment || 0 == (((uintptr_t) addr) & (((uintptr_t) 1 << alignment) - 1))) {
	    if (size == this_size) { // size match!
		best = idx;
		best_size = this_size;
		break;
	    }
	
	    if (size <= this_size && this_size < best_size) { // improved fit?
		best = idx;
		best_size = this_size;
	    }
	}
	
	if (idx == stop_idx) // exhausted live ring?
	    break;
	    
	if (idx)
	    idx--; // bump idx down
	else
	    idx = LARGE_ENTRY_CACHE_SIZE - 1; // wrap idx
    }
    
    if (best > -1 && (best_size - size) < size) { //limit fragmentation to 50%
	addr = (void *)szone->large_entry_cache[best].address;
	boolean_t was_madvised_reusable = szone->large_entry_cache[best].did_madvise_reusable;
	
	// Compact live ring to fill entry now vacated at large_entry_cache[best]
	// while preserving time-order
	if (szone->large_entry_cache_oldest < szone->large_entry_cache_newest) {
	
	    // Ring hasn't wrapped. Fill in from right.
	    for (i = best; i < szone->large_entry_cache_newest; ++i) 
		szone->large_entry_cache[i] = szone->large_entry_cache[i + 1];
		
	    szone->large_entry_cache_newest--; // Pull in right endpoint. 
	    
	} else if (szone->large_entry_cache_newest < szone->large_entry_cache_oldest) {
	
	    // Ring has wrapped. Arrange to fill in from the contiguous side.
	    if (best <= szone->large_entry_cache_newest) {
	        // Fill from right.
		for (i = best; i < szone->large_entry_cache_newest; ++i)
		    szone->large_entry_cache[i] = szone->large_entry_cache[i + 1];
		    
		if (0 < szone->large_entry_cache_newest)
		    szone->large_entry_cache_newest--;
		else
		    szone->large_entry_cache_newest = LARGE_ENTRY_CACHE_SIZE - 1;
	    } else {
	        // Fill from left.
	        for ( i = best; i > szone->large_entry_cache_oldest; --i)
		    szone->large_entry_cache[i] = szone->large_entry_cache[i - 1];
		    
		if (szone->large_entry_cache_oldest < LARGE_ENTRY_CACHE_SIZE - 1)
		    szone->large_entry_cache_oldest++;
		else
		    szone->large_entry_cache_oldest = 0;
	    }
	    
	} else {
	    // By trichotomy, large_entry_cache_newest == large_entry_cache_oldest.
	    // That implies best == large_entry_cache_newest == large_entry_cache_oldest
	    // and the ring is now empty.
	    szone->large_entry_cache[best].address = 0;
	    szone->large_entry_cache[best].size = 0;
	    szone->large_entry_cache[best].did_madvise_reusable = FALSE;
	}
	
	if ((szone->num_large_objects_in_use + 1) * 4 > szone->num_large_entries) {
	    // density of hash table too high; grow table
	    // we do that under lock to avoid a race
	    large_entry_t *entries = large_entries_grow_no_lock(szone, &range_to_deallocate);
	    if (entries == NULL) {
		SZONE_UNLOCK(szone);
		return NULL;
	    }
	}
	
	large_entry.address = (vm_address_t)addr;
	large_entry.size = best_size;
	large_entry.did_madvise_reusable = FALSE; 
	large_entry_insert_no_lock(szone, large_entry);

	szone->num_large_objects_in_use ++;
	szone->num_bytes_in_large_objects += best_size;
	if (!was_madvised_reusable)
	    szone->large_entry_cache_reserve_bytes -= best_size;

 	szone->large_entry_cache_bytes -= best_size;

 	if (szone->flotsam_enabled && szone->large_entry_cache_bytes < SZONE_FLOTSAM_THRESHOLD_LOW) {
 	    szone->flotsam_enabled = FALSE;
 	}

	SZONE_UNLOCK(szone);
	
	if (range_to_deallocate.size) {
	    // we deallocate outside the lock
	    deallocate_pages(szone, (void *)range_to_deallocate.address, range_to_deallocate.size, 0);
	}

	// Perform the madvise() outside the lock.
	// Typically the madvise() is successful and we'll quickly return from this routine.
	// In the unusual case of failure, reacquire the lock to unwind.
#if TARGET_OS_EMBEDDED
	// Ok to do this madvise on embedded because we won't call MADV_FREE_REUSABLE on a large
	// cache block twice without MADV_FREE_REUSE in between.
#endif
	if (was_madvised_reusable && -1 == madvise(addr, size, MADV_FREE_REUSE)) {
	    /* -1 return: VM map entry change makes this unfit for reuse. */
#if DEBUG_MADVISE
	    szone_error(szone, 0, "large_malloc madvise(..., MADV_FREE_REUSE) failed", 
	        addr, "length=%d\n", size);
#endif
	    
	    SZONE_LOCK(szone);
	    szone->num_large_objects_in_use--;
	    szone->num_bytes_in_large_objects -= large_entry.size;
    
	    // Re-acquire "entry" after interval just above where we let go the lock.
    	    large_entry_t *entry = large_entry_for_pointer_no_lock(szone, addr);
	    if (NULL == entry) {
		szone_error(szone, 1, "entry for pointer being discarded from death-row vanished", addr, NULL);
		SZONE_UNLOCK(szone);
	    } else {
	    
		    range_to_deallocate = large_entry_free_no_lock(szone, entry);
		    SZONE_UNLOCK(szone);
		    
		    if (range_to_deallocate.size) {
			// we deallocate outside the lock
			deallocate_pages(szone, (void *)range_to_deallocate.address, range_to_deallocate.size, 0);
		    }
	    }
	    /* Fall through to allocate_pages() afresh. */
	} else {
	    if (cleared_requested) {
		memset(addr, 0, size);
	    }
	
	    return addr;
	}
    } else {
	SZONE_UNLOCK(szone);
    }
    }

    range_to_deallocate.size = 0;
    range_to_deallocate.address = 0;
#endif /* LARGE_CACHE */

    addr = allocate_pages(szone, size, alignment, szone->debug_flags, VM_MEMORY_MALLOC_LARGE);
    if (addr == NULL) {
	return NULL;
    }

    SZONE_LOCK(szone);
    if ((szone->num_large_objects_in_use + 1) * 4 > szone->num_large_entries) {
	// density of hash table too high; grow table
	// we do that under lock to avoid a race
	large_entry_t *entries = large_entries_grow_no_lock(szone, &range_to_deallocate);
	if (entries == NULL) {
	    SZONE_UNLOCK(szone);
	    return NULL;
	}
    }

    large_entry.address = (vm_address_t)addr;
    large_entry.size = size;
    large_entry.did_madvise_reusable = FALSE;
    large_entry_insert_no_lock(szone, large_entry);

    szone->num_large_objects_in_use ++;
    szone->num_bytes_in_large_objects += size;
    SZONE_UNLOCK(szone);
    
    if (range_to_deallocate.size) {
	// we deallocate outside the lock
	deallocate_pages(szone, (void *)range_to_deallocate.address, range_to_deallocate.size, 0);
    }
    return addr;
}

static NOINLINE void
free_large(szone_t *szone, void *ptr)
{
    // We have established ptr is page-aligned and neither tiny nor small
    large_entry_t	*entry;
    vm_range_t		vm_range_to_deallocate;
    
    SZONE_LOCK(szone);
    entry = large_entry_for_pointer_no_lock(szone, ptr);
    if (entry) {
#if LARGE_CACHE
#ifndef MADV_CAN_REUSE
#define MADV_CAN_REUSE 9 /* per Francois, for testing until xnu is resubmitted to B&I */
#endif
	if (entry->size < LARGE_CACHE_SIZE_ENTRY_LIMIT && 
	    -1 != madvise((void *)(entry->address), entry->size, MADV_CAN_REUSE)) { // Put the large_entry_t on the death-row cache?    
	    int idx = szone->large_entry_cache_newest, stop_idx = szone->large_entry_cache_oldest;
	    large_entry_t this_entry = *entry; // Make a local copy, "entry" is volatile when lock is let go.
	    boolean_t reusable = TRUE;
	    boolean_t should_madvise = szone->large_entry_cache_reserve_bytes + this_entry.size > szone->large_entry_cache_reserve_limit;
    
	    // Already freed?
	    // [Note that repeated entries in death-row risk vending the same entry subsequently
	    // to two different malloc() calls. By checking here the (illegal) double free
	    // is accommodated, matching the behavior of the previous implementation.]
	    while (1) { // Scan large_entry_cache starting with most recent entry
		if (szone->large_entry_cache[idx].address == entry->address) {
		    szone_error(szone, 1, "pointer being freed already on death-row", ptr, NULL);
		    SZONE_UNLOCK(szone);
		    return;
		}
	
		if (idx == stop_idx) // exhausted live ring?
		    break;
	    
		if (idx)
		    idx--; // bump idx down
		else
		    idx = LARGE_ENTRY_CACHE_SIZE - 1; // wrap idx
	    }

	    SZONE_UNLOCK(szone);
	    
	    if (szone->debug_flags & SCALABLE_MALLOC_PURGEABLE) { // Are we a purgable zone?
		int state = VM_PURGABLE_NONVOLATILE; // restore to default condition

		if (KERN_SUCCESS != vm_purgable_control(mach_task_self(), this_entry.address, VM_PURGABLE_SET_STATE, &state)) {
		    malloc_printf("*** can't vm_purgable_control(..., VM_PURGABLE_SET_STATE) for large freed block at %p\n", 
		        this_entry.address);
		    reusable = FALSE;
		}
	    }

	    if (szone->large_legacy_reset_mprotect) { // Linked for Leopard?
		// Accomodate Leopard apps that (illegally) mprotect() their own guard pages on large malloc'd allocations
		int err = mprotect((void *)(this_entry.address), this_entry.size, PROT_READ | PROT_WRITE);
                if (err) {
                    malloc_printf("*** can't reset protection for large freed block at %p\n", this_entry.address);
		    reusable = FALSE;
                }
            }
	    
	    // madvise(..., MADV_REUSABLE) death-row arrivals if hoarding would exceed large_entry_cache_reserve_limit 
	    if (should_madvise) {
		// Issue madvise to avoid paging out the dirtied free()'d pages in "entry"
		MAGMALLOC_MADVFREEREGION((void *)szone, (void *)0, (void *)(this_entry.address), this_entry.size); // DTrace USDT Probe

#if TARGET_OS_EMBEDDED
		// Ok to do this madvise on embedded because we won't call MADV_FREE_REUSABLE on a large
		// cache block twice without MADV_FREE_REUSE in between.
#endif
		if (-1 == madvise((void *)(this_entry.address), this_entry.size, MADV_FREE_REUSABLE)) {
		    /* -1 return: VM map entry change makes this unfit for reuse. */
#if DEBUG_MADVISE
		    szone_error(szone, 0, "free_large madvise(..., MADV_FREE_REUSABLE) failed", 
		        (void *)this_entry.address, "length=%d\n", this_entry.size);
#endif
		    reusable = FALSE;
		}
	    }
	    
	    SZONE_LOCK(szone);

	    // Re-acquire "entry" after interval just above where we let go the lock.
    	    entry = large_entry_for_pointer_no_lock(szone, ptr);
	    if (NULL == entry) {
		szone_error(szone, 1, "entry for pointer being freed from death-row vanished", ptr, NULL);
		SZONE_UNLOCK(szone);
		return;
	    }

	    // Add "entry" to death-row ring
	    if (reusable) {
		int idx = szone->large_entry_cache_newest; // Most recently occupied
		vm_address_t addr;
		size_t adjsize;
		
		if (szone->large_entry_cache_newest == szone->large_entry_cache_oldest &&
		    0 == szone->large_entry_cache[idx].address) {
		    // Ring is empty, idx is good as it stands
		    addr = 0;
		    adjsize = 0;
		} else {
		    // Extend the queue to the "right" by bumping up large_entry_cache_newest
		    if (idx == LARGE_ENTRY_CACHE_SIZE - 1)
			idx = 0; // Wrap index
		    else
			idx++; // Bump index
			    
		    if (idx == szone->large_entry_cache_oldest) { // Fully occupied
			// Drop this entry from the cache and deallocate the VM
			addr = szone->large_entry_cache[idx].address;
			adjsize = szone->large_entry_cache[idx].size;
			szone->large_entry_cache_bytes -= adjsize;
			if (!szone->large_entry_cache[idx].did_madvise_reusable)
			    szone->large_entry_cache_reserve_bytes -= adjsize;
		    } else {
			// Using an unoccupied cache slot
			addr = 0;
			adjsize = 0;
		    }
		}

		if ((szone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE))
		    memset((void *)(entry->address), 0x55, entry->size);
			
		entry->did_madvise_reusable = should_madvise; // Was madvise()'d above?
		if (!should_madvise) // Entered on death-row without madvise() => up the hoard total
		    szone->large_entry_cache_reserve_bytes += entry->size;
		
 		szone->large_entry_cache_bytes += entry->size;

 		if (!szone->flotsam_enabled && szone->large_entry_cache_bytes > SZONE_FLOTSAM_THRESHOLD_HIGH) {
 		    szone->flotsam_enabled = TRUE;
 		}

		szone->large_entry_cache[idx] = *entry;
		szone->large_entry_cache_newest = idx;
		
		szone->num_large_objects_in_use--;
		szone->num_bytes_in_large_objects -= entry->size;
		
		(void)large_entry_free_no_lock(szone, entry);
		    
		if (0 == addr) {
		    SZONE_UNLOCK(szone);
		    return;
		}
		    
		// Fall through to drop large_entry_cache_oldest from the cache,
		// and then deallocate its pages.

		// Trim the queue on the "left" by bumping up large_entry_cache_oldest
		if (szone->large_entry_cache_oldest == LARGE_ENTRY_CACHE_SIZE - 1)
		    szone->large_entry_cache_oldest = 0;
		else
		    szone->large_entry_cache_oldest++;
		    
		// we deallocate_pages, including guard pages, outside the lock
		SZONE_UNLOCK(szone);
		deallocate_pages(szone, (void *)addr, (size_t)adjsize, 0);
		return;
	    } else {
		/* fall through to discard an allocation that is not reusable */
	    }
	}
#endif /* LARGE_CACHE */

	szone->num_large_objects_in_use--;
	szone->num_bytes_in_large_objects -= entry->size;
    
	vm_range_to_deallocate = large_entry_free_no_lock(szone, entry);
    } else {
#if DEBUG_MALLOC
	large_debug_print(szone);
#endif
	szone_error(szone, 1, "pointer being freed was not allocated", ptr, NULL);
	SZONE_UNLOCK(szone);
	return;
    }
    SZONE_UNLOCK(szone); // we release the lock asap
    CHECK(szone, __PRETTY_FUNCTION__);
    
    // we deallocate_pages, including guard pages, outside the lock
    if (vm_range_to_deallocate.address) {
#if DEBUG_MALLOC
	// FIXME: large_entry_for_pointer_no_lock() needs the lock held ...
	if (large_entry_for_pointer_no_lock(szone, (void *)vm_range_to_deallocate.address)) {
	    malloc_printf("*** invariant broken: %p still in use num_large_entries=%d\n", 
			  vm_range_to_deallocate.address, szone->num_large_entries);
	    large_debug_print(szone);
	    szone_sleep();
	}
#endif
	deallocate_pages(szone, (void *)vm_range_to_deallocate.address, (size_t)vm_range_to_deallocate.size, 0);
    }
}

static INLINE void *
large_try_shrink_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_good_size)
{
    size_t shrinkage = old_size - new_good_size;
    
    if (shrinkage) {
	SZONE_LOCK(szone);
	/* contract existing large entry */
	large_entry_t *large_entry = large_entry_for_pointer_no_lock(szone, ptr);
	if (!large_entry) {
	    szone_error(szone, 1, "large entry reallocated is not properly in table", ptr, NULL);
	    SZONE_UNLOCK(szone);
	    return ptr;
	}
	
	large_entry->address = (vm_address_t)ptr;
	large_entry->size = new_good_size;
	szone->num_bytes_in_large_objects -= shrinkage;
	SZONE_UNLOCK(szone); // we release the lock asap
	
	deallocate_pages(szone, (void *)((uintptr_t)ptr + new_good_size), shrinkage, 0);
    }
    return ptr;
}

static INLINE int
large_try_realloc_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_size)
{
    vm_address_t	addr = (vm_address_t)ptr + old_size;
    large_entry_t	*large_entry;
    kern_return_t	err;

    SZONE_LOCK(szone);
    large_entry = large_entry_for_pointer_no_lock(szone, (void *)addr);
    SZONE_UNLOCK(szone);
    
    if (large_entry) { // check if "addr = ptr + old_size" is already spoken for
	return 0; // large pointer already exists in table - extension is not going to work
    }
    
    new_size = round_page(new_size);
    /*
     * Ask for allocation at a specific address, and mark as realloc
     * to request coalescing with previous realloc'ed extensions.
     */
    err = vm_allocate(mach_task_self(), &addr, new_size - old_size, VM_MAKE_TAG(VM_MEMORY_REALLOC));
    if (err != KERN_SUCCESS) {
	return 0;
    }
    
    SZONE_LOCK(szone);
    /* extend existing large entry */
    large_entry = large_entry_for_pointer_no_lock(szone, ptr);
    if (!large_entry) {
	szone_error(szone, 1, "large entry reallocated is not properly in table", ptr, NULL);
	SZONE_UNLOCK(szone);
	return 0; // Bail, leaking "addr"
    }

    large_entry->address = (vm_address_t)ptr;
    large_entry->size = new_size;
    szone->num_bytes_in_large_objects += new_size - old_size;
    SZONE_UNLOCK(szone); // we release the lock asap
    
    return 1;
}

/*********************	Zone call backs	************************/
/*
 * Mark these NOINLINE to avoid bloating the purgeable zone call backs
 */
static NOINLINE void
szone_free(szone_t *szone, void *ptr)
{
    region_t	tiny_region;
    region_t	small_region;

#if DEBUG_MALLOC
    if (LOG(szone, ptr))
	malloc_printf("in szone_free with %p\n", ptr);
#endif
    if (!ptr)
	return;
    /*
     * Try to free to a tiny region.
     */
    if ((uintptr_t)ptr & (TINY_QUANTUM - 1)) {
	szone_error(szone, 1, "Non-aligned pointer being freed", ptr, NULL);
	return;
    }
    if ((tiny_region = tiny_region_for_ptr_no_lock(szone, ptr)) != NULL) {
	if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS) {
	    szone_error(szone, 1, "Pointer to metadata being freed", ptr, NULL);
	    return;
	}
	free_tiny(szone, ptr, tiny_region, 0);
	return;
    }

    /*
     * Try to free to a small region.
     */
    if ((uintptr_t)ptr & (SMALL_QUANTUM - 1)) {
	szone_error(szone, 1, "Non-aligned pointer being freed (2)", ptr, NULL);
	return;
    }
    if ((small_region = small_region_for_ptr_no_lock(szone, ptr)) != NULL) {
	if (SMALL_META_INDEX_FOR_PTR(ptr) >= NUM_SMALL_BLOCKS) {
	    szone_error(szone, 1, "Pointer to metadata being freed (2)", ptr, NULL);
	    return;
	}
	free_small(szone, ptr, small_region, 0);
	return;
    }

    /* check that it's a legal large allocation */
    if ((uintptr_t)ptr & (vm_page_size - 1)) {
	szone_error(szone, 1, "non-page-aligned, non-allocated pointer being freed", ptr, NULL);
	return;
    }
    free_large(szone, ptr);
}

static NOINLINE void
szone_free_definite_size(szone_t *szone, void *ptr, size_t size)
{
#if DEBUG_MALLOC
    if (LOG(szone, ptr))
	malloc_printf("in szone_free_definite_size with %p\n", ptr);
	
    if (0 == size) {
	szone_error(szone, 1, "pointer of size zero being freed", ptr, NULL);
	return;
    }
    
#endif
    if (!ptr)
	return;
	
    /*
     * Try to free to a tiny region.
     */
    if ((uintptr_t)ptr & (TINY_QUANTUM - 1)) {
	szone_error(szone, 1, "Non-aligned pointer being freed", ptr, NULL);
	return;
    }
    if (size <= (NUM_TINY_SLOTS - 1)*TINY_QUANTUM) {
	if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS) {
	    szone_error(szone, 1, "Pointer to metadata being freed", ptr, NULL);
	    return;
	}
	free_tiny(szone, ptr, TINY_REGION_FOR_PTR(ptr), size);
	return;
    }

    /*
     * Try to free to a small region.
     */
    if ((uintptr_t)ptr & (SMALL_QUANTUM - 1)) {
	szone_error(szone, 1, "Non-aligned pointer being freed (2)", ptr, NULL);
	return;
    }
    if (size <= szone->large_threshold) {
	if (SMALL_META_INDEX_FOR_PTR(ptr) >= NUM_SMALL_BLOCKS) {
	    szone_error(szone, 1, "Pointer to metadata being freed (2)", ptr, NULL);
	    return;
	}
	free_small(szone, ptr, SMALL_REGION_FOR_PTR(ptr), size);
	return;
    }

    /* check that it's a legal large allocation */
    if ((uintptr_t)ptr & (vm_page_size - 1)) {
	szone_error(szone, 1, "non-page-aligned, non-allocated pointer being freed", ptr, NULL);
	return;
    }
    free_large(szone, ptr);
}

static NOINLINE void *
szone_malloc_should_clear(szone_t *szone, size_t size, boolean_t cleared_requested)
{
    void	*ptr;
    msize_t	msize;

    if (size <= (NUM_TINY_SLOTS - 1)*TINY_QUANTUM) {
	// think tiny
	msize = TINY_MSIZE_FOR_BYTES(size + TINY_QUANTUM - 1);
	if (!msize)
	    msize = 1;
	ptr = tiny_malloc_should_clear(szone, msize, cleared_requested);
    } else if (size <= szone->large_threshold) {
	// think small
	msize = SMALL_MSIZE_FOR_BYTES(size + SMALL_QUANTUM - 1);
	if (!msize) 
	    msize = 1;
	ptr = small_malloc_should_clear(szone, msize, cleared_requested);
    } else {
	// large
	size_t num_pages = round_page(size) >> vm_page_shift;
	if (num_pages == 0)	/* Overflowed */
	    ptr = 0;
	else
	    ptr = large_malloc(szone, num_pages, 0, cleared_requested);
    }
#if DEBUG_MALLOC
    if (LOG(szone, ptr))
	malloc_printf("szone_malloc returned %p\n", ptr);
#endif
    /*
     * If requested, scribble on allocated memory.
     */
    if ((szone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE) && ptr && !cleared_requested && size)
	memset(ptr, 0xaa, size);

    return ptr;
}

static NOINLINE void *
szone_malloc(szone_t *szone, size_t size) {
    return szone_malloc_should_clear(szone, size, 0);
}

static NOINLINE void *
szone_calloc(szone_t *szone, size_t num_items, size_t size)
{
    size_t total_bytes = num_items * size;
    
    // Check for overflow of integer multiplication
    if (num_items > 1) {
#if __LP64__ /* size_t is uint64_t */
	if ((num_items | size) & 0xffffffff00000000ul) {
	    // num_items or size equals or exceeds sqrt(2^64) == 2^32, appeal to wider arithmetic
	    __uint128_t product = ((__uint128_t)num_items) * ((__uint128_t)size);
	    if ((uint64_t)(product >> 64)) // compiles to test on upper register of register pair
	        return NULL;
	}
#else /* size_t is uint32_t */
	if ((num_items | size) & 0xffff0000ul) {
	    // num_items or size equals or exceeds sqrt(2^32) == 2^16, appeal to wider arithmetic
	    uint64_t product = ((uint64_t)num_items) * ((uint64_t)size);
	    if ((uint32_t)(product >> 32)) // compiles to test on upper register of register pair
	        return NULL;
	}
#endif
    }
    
    return szone_malloc_should_clear(szone, total_bytes, 1);
}

static NOINLINE void *
szone_valloc(szone_t *szone, size_t size)
{
    void	*ptr;
    
    if (size <= szone->large_threshold) {
	ptr = szone_memalign(szone, vm_page_size, size);
    } else {
	size_t	num_pages;

	num_pages = round_page(size) >> vm_page_shift;
	ptr = large_malloc(szone, num_pages, 0, 0);
    }
    
#if DEBUG_MALLOC
    if (LOG(szone, ptr))
	malloc_printf("szone_valloc returned %p\n", ptr);
#endif
    return ptr;
}

/* Isolate PIC-base load (for __is_threaded) here. */
static NOINLINE size_t
szone_size_try_large(szone_t *szone, const void *ptr)
{
    size_t		size = 0;
    large_entry_t	*entry;

    SZONE_LOCK(szone);
    entry = large_entry_for_pointer_no_lock(szone, ptr);
    if (entry) {
	size = entry->size;
    }
    SZONE_UNLOCK(szone); 
#if DEBUG_MALLOC
    if (LOG(szone, ptr)) {
	malloc_printf("szone_size for %p returned %d\n", ptr, (unsigned)size);
    }
#endif
    return size;
}

static NOINLINE size_t
szone_size(szone_t *szone, const void *ptr)
{
    boolean_t		is_free;
    msize_t		msize, msize_and_free;
    
    if (!ptr)
	return 0;
#if DEBUG_MALLOC
    if (LOG(szone, ptr)) {
	malloc_printf("in szone_size for %p (szone=%p)\n", ptr, szone);
    }
#endif

    /*
     * Look for it in a tiny region.
     */
    if ((uintptr_t)ptr & (TINY_QUANTUM - 1))
	return 0;
    if (tiny_region_for_ptr_no_lock(szone, ptr)) {
	if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS)
	    return 0;
	msize = get_tiny_meta_header(ptr, &is_free);
	if (is_free)
	    return 0;
#if TINY_CACHE
	{
	    mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr));
	    if (DEPOT_MAGAZINE_INDEX != mag_index) {
		magazine_t	*tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);

		if (msize < TINY_QUANTUM && ptr == (void *)((uintptr_t)(tiny_mag_ptr->mag_last_free) & ~ (TINY_QUANTUM - 1)))
		    return 0;
	    } else {
		for (mag_index = 0; mag_index < szone->num_tiny_magazines; mag_index++) {
		    magazine_t	*tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);

		    if (msize < TINY_QUANTUM && ptr == (void *)((uintptr_t)(tiny_mag_ptr->mag_last_free) & ~ (TINY_QUANTUM - 1)))
			return 0;
		}
	    }
	}
#endif
	return TINY_BYTES_FOR_MSIZE(msize);
    }
    
    /*
     * Look for it in a small region.
     */
    if ((uintptr_t)ptr & (SMALL_QUANTUM - 1))
	return 0;
    if (small_region_for_ptr_no_lock(szone, ptr)) {
	if (SMALL_META_INDEX_FOR_PTR(ptr) >= NUM_SMALL_BLOCKS)
	    return 0;
	msize_and_free = *SMALL_METADATA_FOR_PTR(ptr);
	if (msize_and_free & SMALL_IS_FREE)
	    return 0;
#if SMALL_CACHE
	{
	    mag_index_t	mag_index = MAGAZINE_INDEX_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr));
	    if (DEPOT_MAGAZINE_INDEX != mag_index) {
		magazine_t	*small_mag_ptr = &(szone->small_magazines[mag_index]);

		if (ptr == (void *)((uintptr_t)(small_mag_ptr->mag_last_free) & ~ (SMALL_QUANTUM - 1)))
		    return 0;
	    } else {
		for (mag_index = 0; mag_index < szone->num_small_magazines; mag_index++) {
		    magazine_t	*small_mag_ptr = &(szone->small_magazines[mag_index]);

		    if (ptr == (void *)((uintptr_t)(small_mag_ptr->mag_last_free) & ~ (SMALL_QUANTUM - 1)))
			return 0;
		}
	    }
	}
#endif
	return SMALL_BYTES_FOR_MSIZE(msize_and_free);
    }

    /*
     * If not page-aligned, it cannot have come from a large allocation.
     */
    if ((uintptr_t)ptr & (vm_page_size - 1))
	return 0;

    /*
     * Look for it in a large entry.
     */
    return szone_size_try_large(szone, ptr);
}

static NOINLINE void *
szone_realloc(szone_t *szone, void *ptr, size_t new_size)
{
    size_t	old_size, new_good_size, valid_size;
    void	*new_ptr;
    
#if DEBUG_MALLOC
    if (LOG(szone, ptr)) {
	malloc_printf("in szone_realloc for %p, %d\n", ptr, (unsigned)new_size);
    }
#endif
    if (NULL == ptr) {
	// If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
	return szone_malloc(szone, new_size);
    } else if (0 == new_size) {
	// If size is 0 and ptr is not a null pointer, the object pointed to is freed.
	szone_free(szone, ptr);
	// If size is 0, either a null pointer or a unique pointer that can be successfully passed 
	// to free() shall be returned.
	return szone_malloc(szone, 1);
    }

    old_size = szone_size(szone, ptr);
    if (!old_size) {
	szone_error(szone, 1, "pointer being reallocated was not allocated", ptr, NULL);
	return NULL;
    }

    new_good_size = szone_good_size(szone, new_size);
    if (new_good_size == old_size) { // Existing allocation is best fit evar?
	return ptr;
    }

    /*
     * If the new size suits the tiny allocator and the pointer being resized
     * belongs to a tiny region, try to reallocate in-place.
     */
    if (new_good_size <= (NUM_TINY_SLOTS - 1) * TINY_QUANTUM) {
	if (old_size <= (NUM_TINY_SLOTS - 1) * TINY_QUANTUM) {
	    if (new_good_size <= (old_size >> 1)) {
	        /*
	         * Serious shrinkage (more than half). free() the excess.
		 */
		return tiny_try_shrink_in_place(szone, ptr, old_size, new_good_size);
	    } else if (new_good_size <= old_size) {
		/* 
		 * new_good_size smaller than old_size but not by much (less than half).
		 * Avoid thrashing at the expense of some wasted storage.
		 */
		return ptr;
	    } else if (tiny_try_realloc_in_place(szone, ptr, old_size, new_good_size)) { // try to grow the allocation
		return ptr;
	    }
	}

    /*
     * Else if the new size suits the small allocator and the pointer being resized
     * belongs to a small region, and we're not protecting the small allocations
     * try to reallocate in-place.
     */
    } else if (new_good_size <= szone->large_threshold) {
	if ((NUM_TINY_SLOTS - 1) * TINY_QUANTUM < old_size && old_size <= szone->large_threshold) {
	    if (new_good_size <= (old_size >> 1)) {
		return small_try_shrink_in_place(szone, ptr, old_size, new_good_size);
	    } else if (new_good_size <= old_size) {
		return ptr;
	    } else if (small_try_realloc_in_place(szone, ptr, old_size, new_good_size)) {
		return ptr;
	    }
	}
    /*
     * Else if the allocation's a large allocation, try to reallocate in-place there.
     */
    } else if (!(szone->debug_flags & SCALABLE_MALLOC_PURGEABLE) && // purgeable needs fresh allocation
		(old_size > szone->large_threshold) &&
		(new_good_size > szone->large_threshold)) {
	if (new_good_size <= (old_size >> 1)) {
	    return large_try_shrink_in_place(szone, ptr, old_size, new_good_size);
	} else if (new_good_size <= old_size) {
	    return ptr;
	} else if (large_try_realloc_in_place(szone, ptr, old_size, new_good_size)) {
	    return ptr;
	}
    }

    /*
     * Can't reallocate in place for whatever reason; allocate a new buffer and copy.
     */
    if (new_good_size <= (old_size >> 1)) {
	/* Serious shrinkage (more than half). FALL THROUGH to alloc/copy/free. */
    } else if (new_good_size <= old_size) { 
	return ptr;
    }

    new_ptr = szone_malloc(szone, new_size);
    if (new_ptr == NULL)
	return NULL;

    /*
     * If the allocation's large enough, try to copy using VM.  If that fails, or
     * if it's too small, just copy by hand.
     */
    valid_size = MIN(old_size, new_size);
    if ((valid_size < szone->vm_copy_threshold) ||
	vm_copy(mach_task_self(), (vm_address_t)ptr, valid_size, (vm_address_t)new_ptr))
	memcpy(new_ptr, ptr, valid_size);
    szone_free(szone, ptr);
    
#if DEBUG_MALLOC
    if (LOG(szone, ptr)) {
	malloc_printf("szone_realloc returned %p for %d\n", new_ptr, (unsigned)new_size);
    }
#endif
    return new_ptr;
}

static NOINLINE void *
szone_memalign(szone_t *szone, size_t alignment, size_t size)
{
    if ((size + alignment) < size) // size_t arithmetic wrapped!
        return NULL;

    // alignment is gauranteed a power of 2 at least as large as sizeof(void *), hence non-zero.
    // Since size + alignment didn't wrap, 0 <= size + alignment - 1 < size + alignment
    size_t span = size + alignment - 1;
    
    if (alignment <= TINY_QUANTUM) {
	return szone_malloc(szone, size); // Trivially satisfied by tiny, small, or large
	
    } else if (span <= (NUM_TINY_SLOTS - 1)*TINY_QUANTUM) {
	msize_t mspan = TINY_MSIZE_FOR_BYTES(span + TINY_QUANTUM - 1);
	void *p = szone_malloc(szone, span); // avoids inlining tiny_malloc_should_clear(szone, mspan, 0);
	
	if (NULL == p)
	    return NULL;

	size_t offset = ((uintptr_t) p) & (alignment - 1); // p % alignment
	size_t pad = (0 == offset) ? 0 : alignment - offset; // p + pad achieves desired alignment
	
	msize_t msize = TINY_MSIZE_FOR_BYTES(size + TINY_QUANTUM - 1);
	msize_t mpad = TINY_MSIZE_FOR_BYTES(pad + TINY_QUANTUM - 1);
	msize_t mwaste = mspan - msize - mpad; // excess blocks

	if (mpad > 0) {
	    void *q = (void *)(((uintptr_t) p) + pad);
	    
	    // Mark q as a block header and in-use, thus creating two blocks.
	    magazine_t	*tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->tiny_magazines,
				    REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(p)),
				    MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(p)));    
	    set_tiny_meta_header_in_use(q, msize);
	    tiny_mag_ptr->mag_num_objects++;
	    
	    // set_tiny_meta_header_in_use() "reaffirms" the block_header on the *following* block, so
	    // now set its in_use bit as well. But only if its within the original allocation made above.
	    if (mwaste > 0)
		BITARRAY_SET(TINY_INUSE_FOR_HEADER(TINY_BLOCK_HEADER_FOR_PTR(q)), TINY_INDEX_FOR_PTR(q) + msize);
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	    
	    // Give up mpad blocks beginning at p to the tiny free list
	    // region_t r = TINY_REGION_FOR_PTR(p);
	    szone_free(szone, p); // avoids inlining free_tiny(szone, p, &r);
	    
	    p = q; // advance p to the desired alignment
	}
	
	if (mwaste > 0) {
	    void *q = (void *)(((uintptr_t) p) + TINY_BYTES_FOR_MSIZE(msize));
	    // Mark q as block header and in-use, thus creating two blocks.
	    magazine_t	*tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->tiny_magazines,
				    REGION_TRAILER_FOR_TINY_REGION(TINY_REGION_FOR_PTR(p)),
				    MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(p)));    
	    set_tiny_meta_header_in_use(q, mwaste);
	    tiny_mag_ptr->mag_num_objects++;
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	    
	    // Give up mwaste blocks beginning at q to the tiny free list
	    // region_t r = TINY_REGION_FOR_PTR(q);
	    szone_free(szone, q); // avoids inlining free_tiny(szone, q, &r);
	}
	
	return p; // p has the desired size and alignment, and can later be free()'d
	
    } else if ((NUM_TINY_SLOTS - 1)*TINY_QUANTUM < size && alignment <= SMALL_QUANTUM) {
	return szone_malloc(szone, size); // Trivially satisfied by small or large
	
    } else if (span <= szone->large_threshold) {

	if (size <= (NUM_TINY_SLOTS - 1)*TINY_QUANTUM) {
	    size = (NUM_TINY_SLOTS - 1)*TINY_QUANTUM + TINY_QUANTUM; // ensure block allocated by small does not have a tiny-possible size
	    span = size + alignment - 1;
	}

	msize_t mspan = SMALL_MSIZE_FOR_BYTES(span + SMALL_QUANTUM - 1);
	void *p = szone_malloc(szone, span); // avoid inlining small_malloc_should_clear(szone, mspan, 0);

	if (NULL == p)
	    return NULL;

	size_t offset = ((uintptr_t) p) & (alignment - 1); // p % alignment
	size_t pad = (0 == offset) ? 0 : alignment - offset; // p + pad achieves desired alignment
	
	msize_t msize = SMALL_MSIZE_FOR_BYTES(size + SMALL_QUANTUM - 1);
	msize_t mpad = SMALL_MSIZE_FOR_BYTES(pad + SMALL_QUANTUM - 1);
	msize_t mwaste = mspan - msize - mpad; // excess blocks
	
	if (mpad > 0) {
	    void *q = (void *)(((uintptr_t) p) + pad);
	    
	    // Mark q as block header and in-use, thus creating two blocks.
	    magazine_t	*small_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->small_magazines,
				    REGION_TRAILER_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(p)),
				    MAGAZINE_INDEX_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(p)));    
	    small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(p), SMALL_META_INDEX_FOR_PTR(p), mpad);
	    small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(q), SMALL_META_INDEX_FOR_PTR(q), msize + mwaste);
	    small_mag_ptr->mag_num_objects++;
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	    
	    // Give up mpad blocks beginning at p to the small free list
	    // region_t r = SMALL_REGION_FOR_PTR(p);
	    szone_free(szone, p); // avoid inlining free_small(szone, p, &r);
	    
	    p = q; // advance p to the desired alignment
	}
	if (mwaste > 0) {
	    void *q = (void *)(((uintptr_t) p) + SMALL_BYTES_FOR_MSIZE(msize));
	    // Mark q as block header and in-use, thus creating two blocks.
	    magazine_t	*small_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->small_magazines,
				    REGION_TRAILER_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(p)),
				    MAGAZINE_INDEX_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(p)));    
	    small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(p), SMALL_META_INDEX_FOR_PTR(p), msize);
	    small_meta_header_set_in_use(SMALL_META_HEADER_FOR_PTR(q), SMALL_META_INDEX_FOR_PTR(q), mwaste);
	    small_mag_ptr->mag_num_objects++;
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	    
	    // Give up mwaste blocks beginning at q to the small free list
	    // region_t r = SMALL_REGION_FOR_PTR(q);
	    szone_free(szone, q); // avoid inlining free_small(szone, q, &r);
	}

	return p; // p has the desired size and alignment, and can later be free()'d
	
    } else if (szone->large_threshold < size && alignment <= vm_page_size) {
	return szone_malloc(szone, size); // Trivially satisfied by large
	
    } else {
	// ensure block allocated by large does not have a small-possible size
	size_t num_pages = round_page(MAX(szone->large_threshold + 1, size)) >> vm_page_shift;
	void *p;
	
	if (num_pages == 0)	/* Overflowed */
	    p = NULL;
	else
	    p = large_malloc(szone, num_pages, MAX(vm_page_shift, __builtin_ctz(alignment)), 0);
	    
	return p;
    }
    /* NOTREACHED */
}

// given a size, returns the number of pointers allocated capable of holding
// that size, up to the limit specified by the 'count' argument.  These pointers
// are stored in the 'results' array, which must be allocated by the caller.
// may return zero, since this function is only a best attempt at allocating
// the pointers.  clients should be prepared to call malloc for any additional
// blocks they need.
static NOINLINE unsigned
szone_batch_malloc(szone_t *szone, size_t size, void **results, unsigned count)
{
    msize_t	msize = TINY_MSIZE_FOR_BYTES(size + TINY_QUANTUM - 1);
    unsigned	found = 0;
    mag_index_t	mag_index = mag_get_thread_index(szone);
    magazine_t	*tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);

    // only bother implementing this for tiny
    if (size > (NUM_TINY_SLOTS - 1)*TINY_QUANTUM)
        return 0;
    // make sure to return objects at least one quantum in size
    if (!msize)
        msize = 1;

    CHECK(szone, __PRETTY_FUNCTION__);

    // We must lock the zone now, since tiny_malloc_from_free_list assumes that 
    // the caller has done so.
    SZONE_MAGAZINE_PTR_LOCK(szone, tiny_mag_ptr);

    // with the zone locked, allocate objects from the free list until all
    // sufficiently large objects have been exhausted, or we have met our quota
    // of objects to allocate.
    while (found < count) {
	void *ptr = tiny_malloc_from_free_list(szone, tiny_mag_ptr, mag_index, msize);
	if (!ptr)
	    break;

	*results++ = ptr;
	found++;
    }
    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
    return found;
}

/* Try caching the tiny_region and checking if the next ptr hits there. */
static NOINLINE void
szone_batch_free(szone_t *szone, void **to_be_freed, unsigned count)
{
    unsigned	cc = 0;
    void	*ptr;
    region_t	tiny_region = NULL;
    boolean_t	is_free;
    msize_t	msize;
    magazine_t	*tiny_mag_ptr = NULL;
    mag_index_t mag_index = -1;
    
    // frees all the pointers in to_be_freed
    // note that to_be_freed may be overwritten during the process
    if (!count)
	return;
	
    CHECK(szone, __PRETTY_FUNCTION__);
    while (cc < count) {
	ptr = to_be_freed[cc];
	if (ptr) {
	    if (NULL == tiny_region || tiny_region != TINY_REGION_FOR_PTR(ptr)) { // region same as last iteration?
	        if (tiny_mag_ptr) { // non-NULL iff magazine lock taken
		    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		    tiny_mag_ptr = NULL;
		}
		    
		tiny_region = tiny_region_for_ptr_no_lock(szone, ptr);
		
		if (tiny_region) {
		    tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->tiny_magazines,
						REGION_TRAILER_FOR_TINY_REGION(tiny_region),
						MAGAZINE_INDEX_FOR_TINY_REGION(tiny_region));
		    mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(tiny_region);
		}		
	    }
	    if (tiny_region) {
		// this is a tiny pointer
		if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS)
		    break; // pointer to metadata; let the standard free deal with it
		msize = get_tiny_meta_header(ptr, &is_free);
		if (is_free)
		    break; // a double free; let the standard free deal with it
		    
		if (!tiny_free_no_lock(szone, tiny_mag_ptr, mag_index, tiny_region, ptr, msize)) {
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
	SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	tiny_mag_ptr = NULL;
    }

    CHECK(szone, __PRETTY_FUNCTION__);
    while (count--) {
	ptr = to_be_freed[count];
	if (ptr)
	    szone_free(szone, ptr);
    }
}

// FIXME: Suppose one of the locks is held?
static void
szone_destroy(szone_t *szone)
{
    size_t		index;
    large_entry_t	*large;
    vm_range_t		range_to_deallocate;

#if LARGE_CACHE
    SZONE_LOCK(szone);

     /* disable any memory pressure responder */
     szone->flotsam_enabled = FALSE;

    // stack allocated copy of the death-row cache
    int idx = szone->large_entry_cache_oldest, idx_max = szone->large_entry_cache_newest;
    large_entry_t local_entry_cache[LARGE_ENTRY_CACHE_SIZE];

    memcpy((void *)local_entry_cache, (void *)szone->large_entry_cache, sizeof(local_entry_cache));
 	
    szone->large_entry_cache_oldest = szone->large_entry_cache_newest = 0;
    szone->large_entry_cache[0].address = 0x0;
    szone->large_entry_cache[0].size = 0;
    szone->large_entry_cache_bytes = 0;
    szone->large_entry_cache_reserve_bytes = 0;

    SZONE_UNLOCK(szone);
 
    // deallocate the death-row cache outside the zone lock
    while (idx != idx_max) {
	deallocate_pages(szone, (void *) local_entry_cache[idx].address, local_entry_cache[idx].size, 0);
	if (++idx == LARGE_ENTRY_CACHE_SIZE) idx = 0;
    }
    if (0 != local_entry_cache[idx].address && 0 != local_entry_cache[idx].size) {
	deallocate_pages(szone, (void *) local_entry_cache[idx].address, local_entry_cache[idx].size, 0);
    }
#endif

    /* destroy large entries */
    index = szone->num_large_entries;
    while (index--) {
	large = szone->large_entries + index;
	if (large->address) {
	    // we deallocate_pages, including guard pages
	    deallocate_pages(szone, (void *)(large->address), large->size, szone->debug_flags);
	}
    }
    large_entries_free_no_lock(szone, szone->large_entries, szone->num_large_entries, &range_to_deallocate);
    if (range_to_deallocate.size)
	deallocate_pages(szone, (void *)range_to_deallocate.address, (size_t)range_to_deallocate.size, 0);

    /* destroy tiny regions */
    for (index = 0; index < szone->tiny_region_generation->num_regions_allocated; ++index)
      if ((HASHRING_OPEN_ENTRY != szone->tiny_region_generation->hashed_regions[index]) && 
          (HASHRING_REGION_DEALLOCATED != szone->tiny_region_generation->hashed_regions[index]))
        deallocate_pages(szone, szone->tiny_region_generation->hashed_regions[index], TINY_REGION_SIZE, 0);

    /* destroy small regions */
    for (index = 0; index < szone->small_region_generation->num_regions_allocated; ++index)
      if ((HASHRING_OPEN_ENTRY != szone->small_region_generation->hashed_regions[index]) && 
          (HASHRING_REGION_DEALLOCATED != szone->small_region_generation->hashed_regions[index]))
        deallocate_pages(szone, szone->small_region_generation->hashed_regions[index], SMALL_REGION_SIZE, 0);

    /* destroy region hash rings, if any */
    if (szone->tiny_region_generation->hashed_regions != szone->initial_tiny_regions) {
      size_t size = round_page(szone->tiny_region_generation->num_regions_allocated * sizeof(region_t));
      deallocate_pages(szone, szone->tiny_region_generation->hashed_regions, size, 0);
    }
    if (szone->small_region_generation->hashed_regions != szone->initial_small_regions) {
      size_t size = round_page(szone->small_region_generation->num_regions_allocated * sizeof(region_t));
      deallocate_pages(szone, szone->small_region_generation->hashed_regions, size, 0);
    }
    
    /* Now destroy the separate szone region */
    if (szone->cpu_id_key != (pthread_key_t) -1)
	(void)pthread_key_delete(szone->cpu_id_key);
    deallocate_pages(szone, (void *)&(szone->tiny_magazines[-1]), TINY_MAGAZINE_PAGED_SIZE, SCALABLE_MALLOC_ADD_GUARD_PAGES);
    deallocate_pages(szone, (void *)&(szone->small_magazines[-1]), SMALL_MAGAZINE_PAGED_SIZE, SCALABLE_MALLOC_ADD_GUARD_PAGES);
    deallocate_pages(szone, (void *)szone, SZONE_PAGED_SIZE, 0);
}

static NOINLINE size_t
szone_good_size(szone_t *szone, size_t size)
{
    msize_t msize;

    // Find a good size for this tiny allocation.
    if (size <= (NUM_TINY_SLOTS - 1) * TINY_QUANTUM) {
        msize = TINY_MSIZE_FOR_BYTES(size + TINY_QUANTUM - 1);
        if (!msize)
            msize = 1;
        return TINY_BYTES_FOR_MSIZE(msize);
    }
    
    // Find a good size for this small allocation.
    if (size <= szone->large_threshold) {
        msize = SMALL_MSIZE_FOR_BYTES(size + SMALL_QUANTUM - 1);
        if (!msize)
            msize = 1;
        return SMALL_BYTES_FOR_MSIZE(msize);
    }
    
    // Check for integer overflow on the size, since unlike the two cases above,
    // there is no upper bound on allocation size at this point.
    if (size > round_page(size))
        return (size_t)(-1LL);

#if DEBUG_MALLOC
    // It is not acceptable to see a size of zero here, since that means we
    // failed to catch a request for zero bytes in the tiny check, or the size
    // overflowed to zero during some arithmetic.
    if (size == 0)
        malloc_printf("szone_good_size() invariant broken %y\n", size);
#endif
    return round_page(size);
}

unsigned szone_check_counter = 0;
unsigned szone_check_start = 0;
unsigned szone_check_modulo = 1;

static NOINLINE boolean_t
szone_check_all(szone_t *szone, const char *function)
{
    size_t index;

    /* check tiny regions - chould check region count */
    for (index = 0; index < szone->tiny_region_generation->num_regions_allocated; ++index) {
	region_t tiny = szone->tiny_region_generation->hashed_regions[index];
      
	if (HASHRING_REGION_DEALLOCATED == tiny)
	    continue;
      
	if (tiny) {
	    magazine_t *tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->tiny_magazines,
		REGION_TRAILER_FOR_TINY_REGION(tiny), MAGAZINE_INDEX_FOR_TINY_REGION(tiny));

	    if (!tiny_check_region(szone, tiny)) {
		SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		szone->debug_flags &= ~ CHECK_REGIONS;
		szone_error(szone, 1, "check: tiny region incorrect", NULL,
			    "*** tiny region %ld incorrect szone_check_all(%s) counter=%d\n",
			    index, function, szone_check_counter);
		return 0;
	    }
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	}
    }
    /* check tiny free lists */
    for (index = 0; index < NUM_TINY_SLOTS; ++index) {
	if (!tiny_free_list_check(szone, index)) {
            szone->debug_flags &= ~ CHECK_REGIONS;
            szone_error(szone, 1, "check: tiny free list incorrect", NULL,
			"*** tiny free list incorrect (slot=%ld) szone_check_all(%s) counter=%d\n",
			index, function, szone_check_counter);
	    return 0;
	}
    }
 
    /* check small regions - could check region count */
    for (index = 0; index < szone->small_region_generation->num_regions_allocated; ++index) {
	region_t small = szone->small_region_generation->hashed_regions[index];
      
	if (HASHRING_REGION_DEALLOCATED == small)
	    continue;
      
	if (small) {
	    magazine_t *small_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->small_magazines,
		REGION_TRAILER_FOR_SMALL_REGION(small), MAGAZINE_INDEX_FOR_SMALL_REGION(small));
		
	    if (!small_check_region(szone, small)) {
		SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
		szone->debug_flags &= ~ CHECK_REGIONS;
		szone_error(szone, 1, "check: small region incorrect", NULL,
			    "*** small region %ld incorrect szone_check_all(%s) counter=%d\n",
			    index, function, szone_check_counter);
		return 0;
	    }
	    SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
	}
    }
    /* check small free lists */
    for (index = 0; index < szone->num_small_slots; ++index) {
	if (!small_free_list_check(szone, index)) {
            szone->debug_flags &= ~ CHECK_REGIONS;
            szone_error(szone, 1, "check: small free list incorrect", NULL,
			"*** small free list incorrect (slot=%ld) szone_check_all(%s) counter=%d\n",
			index, function, szone_check_counter);
            return 0;
	}
    }

    return 1;
}

static boolean_t
szone_check(szone_t *szone)
{
    if ((++szone_check_counter % 10000) == 0)
	_malloc_printf(ASL_LEVEL_NOTICE, "at szone_check counter=%d\n", szone_check_counter);
	
    if (szone_check_counter < szone_check_start)
	return 1;
	
    if (szone_check_counter % szone_check_modulo)
	return 1;
	
    return szone_check_all(szone, "");
}

static kern_return_t
szone_ptr_in_use_enumerator(task_t task, void *context, unsigned type_mask, vm_address_t zone_address, 
			    memory_reader_t reader, vm_range_recorder_t recorder)
{
    szone_t		*szone;
    kern_return_t	err;
    
    if (!reader) reader = _szone_default_reader;
    
    err = reader(task, zone_address, sizeof(szone_t), (void **)&szone);
    if (err) return err;
    
    err = tiny_in_use_enumerator(task, context, type_mask, szone, reader, recorder);
    if (err) return err;
    
    err = small_in_use_enumerator(task, context, type_mask, szone, reader, recorder);
    if (err) return err;
    
    err = large_in_use_enumerator(task, context, type_mask,
      (vm_address_t)szone->large_entries, szone->num_large_entries, reader, recorder);
    return err;
}

// Following method is deprecated:  use scalable_zone_statistics instead
void
scalable_zone_info(malloc_zone_t *zone, unsigned *info_to_fill, unsigned count)
{
    szone_t	*szone = (void *)zone;
    unsigned	info[13];

    // We do not lock to facilitate debug
    
    size_t	s = 0;
    unsigned	t = 0;
    size_t	u = 0;
    mag_index_t mag_index;
    
    for (mag_index = -1; mag_index < szone->num_tiny_magazines; mag_index++) {
	s += szone->tiny_magazines[mag_index].mag_bytes_free_at_start;
	s += szone->tiny_magazines[mag_index].mag_bytes_free_at_end;
	t += szone->tiny_magazines[mag_index].mag_num_objects;
	u += szone->tiny_magazines[mag_index].mag_num_bytes_in_objects;
    }
    
    info[4] = t;
    info[5] = u;
    
    for (t = 0, u = 0, mag_index = -1; mag_index < szone->num_small_magazines; mag_index++) {
	s += szone->small_magazines[mag_index].mag_bytes_free_at_start;
	s += szone->small_magazines[mag_index].mag_bytes_free_at_end;
	t += szone->small_magazines[mag_index].mag_num_objects;
	u += szone->small_magazines[mag_index].mag_num_bytes_in_objects;
    }
    
    info[6] = t;
    info[7] = u;
    
    info[8] = szone->num_large_objects_in_use;
    info[9] = szone->num_bytes_in_large_objects;
    
    info[10] = 0; // DEPRECATED szone->num_huge_entries;
    info[11] = 0; // DEPRECATED szone->num_bytes_in_huge_objects;
    
    info[12] = szone->debug_flags;
    
    info[0] = info[4] + info[6] + info[8] + info[10];
    info[1] = info[5] + info[7] + info[9] + info[11];
    
    info[3] = (szone->num_tiny_regions - szone->num_tiny_regions_dealloc) * TINY_REGION_SIZE + 
	      (szone->num_small_regions - szone->num_small_regions_dealloc) * SMALL_REGION_SIZE + info[9] + info[11];
    
    info[2] = info[3] - s;
    memcpy(info_to_fill, info, sizeof(unsigned)*count);
}

// FIXME: consistent picture requires locking!
static NOINLINE void
szone_print(szone_t *szone, boolean_t verbose)
{
    unsigned	info[13];
    size_t	index;
    region_t	region;
  
    scalable_zone_info((void *)szone, info, 13);
    _malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
                 "Scalable zone %p: inUse=%u(%y) touched=%y allocated=%y flags=%d\n",
                 szone, info[0], info[1], info[2], info[3], info[12]);
    _malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
                 "\ttiny=%u(%y) small=%u(%y) large=%u(%y) huge=%u(%y)\n",
                 info[4], info[5], info[6], info[7], info[8], info[9], info[10], info[11]);
    // tiny
    _malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
                 "%lu tiny regions:\n", szone->num_tiny_regions);
    if (szone->num_tiny_regions_dealloc)
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
	    "[%lu tiny regions have been vm_deallocate'd]\n", szone->num_tiny_regions_dealloc);
    for (index = 0; index < szone->tiny_region_generation->num_regions_allocated; ++index) {
	region = szone->tiny_region_generation->hashed_regions[index];
	if (HASHRING_OPEN_ENTRY != region && HASHRING_REGION_DEALLOCATED != region) {
	    mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(region);
	    print_tiny_region(verbose, region,
				(region == szone->tiny_magazines[mag_index].mag_last_region) ? 
				szone->tiny_magazines[mag_index].mag_bytes_free_at_start : 0,
				(region == szone->tiny_magazines[mag_index].mag_last_region) ? 
						szone->tiny_magazines[mag_index].mag_bytes_free_at_end : 0);
	}
    }
    if (verbose)
	print_tiny_free_list(szone);
    // small
    _malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
                 "%lu small regions:\n", szone->num_small_regions);
    if (szone->num_small_regions_dealloc)
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
	    "[%lu small regions have been vm_deallocate'd]\n", szone->num_small_regions_dealloc);
    for (index = 0; index < szone->small_region_generation->num_regions_allocated; ++index) {
	region = szone->small_region_generation->hashed_regions[index];
	if (HASHRING_OPEN_ENTRY != region && HASHRING_REGION_DEALLOCATED != region) {
	    mag_index_t mag_index = MAGAZINE_INDEX_FOR_SMALL_REGION(region);
	    print_small_region(szone, verbose, region,
				(region == szone->small_magazines[mag_index].mag_last_region) ? 
				szone->small_magazines[mag_index].mag_bytes_free_at_start : 0,
				(region == szone->small_magazines[mag_index].mag_last_region) ? 
				szone->small_magazines[mag_index].mag_bytes_free_at_end : 0);
	}
    }
    if (verbose)
	print_small_free_list(szone);
}

static void
szone_log(malloc_zone_t *zone, void *log_address)
{
    szone_t	*szone = (szone_t *)zone;

    szone->log_address = log_address;
}

static void
szone_force_lock(szone_t *szone)
{
    mag_index_t i;
    
    for (i = 0; i < szone->num_tiny_magazines; ++i) {
	SZONE_MAGAZINE_PTR_LOCK(szone, (&(szone->tiny_magazines[i])));
    }
    SZONE_MAGAZINE_PTR_LOCK(szone, (&(szone->tiny_magazines[DEPOT_MAGAZINE_INDEX])));
    
    for (i = 0; i < szone->num_small_magazines; ++i) {
	SZONE_MAGAZINE_PTR_LOCK(szone, (&(szone->small_magazines[i])));
    }
    SZONE_MAGAZINE_PTR_LOCK(szone, (&(szone->small_magazines[DEPOT_MAGAZINE_INDEX])));
    
    SZONE_LOCK(szone);
}

static void
szone_force_unlock(szone_t *szone)
{
    mag_index_t i;
    
    SZONE_UNLOCK(szone);
    
    for (i = -1; i < szone->num_small_magazines; ++i) {
	SZONE_MAGAZINE_PTR_UNLOCK(szone, (&(szone->small_magazines[i])));
    }

    for (i = -1; i < szone->num_tiny_magazines; ++i) {
	SZONE_MAGAZINE_PTR_UNLOCK(szone, (&(szone->tiny_magazines[i])));
    }
}

static boolean_t
szone_locked(szone_t *szone) 
{
  mag_index_t i;
  int tookLock;

  tookLock = SZONE_TRY_LOCK(szone);
  if (tookLock == 0)
    return 1;
  SZONE_UNLOCK(szone);
  
  for (i = -1; i < szone->num_small_magazines; ++i) {
    tookLock = SZONE_MAGAZINE_PTR_TRY_LOCK(szone, (&(szone->small_magazines[i])));
    if (tookLock == 0)
      return 1;
    SZONE_MAGAZINE_PTR_UNLOCK(szone, (&(szone->small_magazines[i])));
  }
  
  for (i = -1; i < szone->num_tiny_magazines; ++i) {
    tookLock = SZONE_MAGAZINE_PTR_TRY_LOCK(szone, (&(szone->tiny_magazines[i])));
    if (tookLock == 0)
      return 1;
    SZONE_MAGAZINE_PTR_UNLOCK(szone, (&(szone->tiny_magazines[i])));
  }
  return 0;
}  

static size_t
szone_pressure_relief(szone_t *szone, size_t goal)
{
#if LARGE_CACHE
    if (!szone->flotsam_enabled)
	    return 0;

     SZONE_LOCK(szone);

    // stack allocated copy of the death-row cache
    int idx = szone->large_entry_cache_oldest, idx_max = szone->large_entry_cache_newest;
    large_entry_t local_entry_cache[LARGE_ENTRY_CACHE_SIZE];

    memcpy((void *)local_entry_cache, (void *)szone->large_entry_cache, sizeof(local_entry_cache));
 	
    szone->large_entry_cache_oldest = szone->large_entry_cache_newest = 0;
    szone->large_entry_cache[0].address = 0x0;
    szone->large_entry_cache[0].size = 0;
    szone->large_entry_cache_bytes = 0;
    szone->large_entry_cache_reserve_bytes = 0;

    szone->flotsam_enabled = FALSE;

    SZONE_UNLOCK(szone);
 
    // deallocate the death-row cache outside the zone lock
    size_t total = 0;
    while (idx != idx_max) {
	deallocate_pages(szone, (void *) local_entry_cache[idx].address, local_entry_cache[idx].size, 0);
	total += local_entry_cache[idx].size;
	if (++idx == LARGE_ENTRY_CACHE_SIZE) idx = 0;
    }
    if (0 != local_entry_cache[idx].address && 0 != local_entry_cache[idx].size) {
	deallocate_pages(szone, (void *) local_entry_cache[idx].address, local_entry_cache[idx].size, 0);
	total += local_entry_cache[idx].size;
    }
    MAGMALLOC_PRESSURERELIEF((void *)szone, goal, total); // DTrace USDT Probe
    return total;
#else
    return 0;
#endif
}

boolean_t
scalable_zone_statistics(malloc_zone_t *zone, malloc_statistics_t *stats, unsigned subzone)
{
    szone_t *szone = (szone_t *)zone;
    
    switch (subzone) {
	case 0:
	{	
	    size_t	s = 0;
	    unsigned	t = 0;
	    size_t	u = 0;
	    mag_index_t mag_index;
	    
	    for (mag_index = -1; mag_index < szone->num_tiny_magazines; mag_index++) {
		s += szone->tiny_magazines[mag_index].mag_bytes_free_at_start;
		s += szone->tiny_magazines[mag_index].mag_bytes_free_at_end;
		t += szone->tiny_magazines[mag_index].mag_num_objects;
		u += szone->tiny_magazines[mag_index].mag_num_bytes_in_objects;
	    }
    
	    stats->blocks_in_use = t;
	    stats->size_in_use = u;
	    stats->size_allocated = (szone->num_tiny_regions - szone->num_tiny_regions_dealloc) * TINY_REGION_SIZE;
	    stats->max_size_in_use = stats->size_allocated - s;
	    return 1;
	}
	case 1:	
	{
	    size_t	s = 0;
	    unsigned	t = 0;
	    size_t	u = 0;
	    mag_index_t mag_index;
	    
	    for (mag_index = -1; mag_index < szone->num_small_magazines; mag_index++) {
		s += szone->small_magazines[mag_index].mag_bytes_free_at_start;
		s += szone->small_magazines[mag_index].mag_bytes_free_at_end;
		t += szone->small_magazines[mag_index].mag_num_objects;
		u += szone->small_magazines[mag_index].mag_num_bytes_in_objects;
	    }
    
	    stats->blocks_in_use = t;
	    stats->size_in_use = u;
	    stats->size_allocated = (szone->num_small_regions - szone->num_small_regions_dealloc) * SMALL_REGION_SIZE;
	    stats->max_size_in_use = stats->size_allocated - s;
	    return 1;
	}
	case 2:
	    stats->blocks_in_use = szone->num_large_objects_in_use;
	    stats->size_in_use = szone->num_bytes_in_large_objects;
	    stats->max_size_in_use = stats->size_allocated = stats->size_in_use;
	    return 1;
	case 3:
	    stats->blocks_in_use = 0; // DEPRECATED szone->num_huge_entries;
	    stats->size_in_use = 0; // DEPRECATED szone->num_bytes_in_huge_objects;
	    stats->max_size_in_use = stats->size_allocated = 0;
	    return 1;
    }
    return 0;
}

static void
szone_statistics(szone_t *szone, malloc_statistics_t *stats)
{
    size_t	large;
    
    size_t	s = 0;
    unsigned	t = 0;
    size_t	u = 0;
    mag_index_t mag_index;
    
    for (mag_index = -1; mag_index < szone->num_tiny_magazines; mag_index++) {
	s += szone->tiny_magazines[mag_index].mag_bytes_free_at_start;
	s += szone->tiny_magazines[mag_index].mag_bytes_free_at_end;
	t += szone->tiny_magazines[mag_index].mag_num_objects;
	u += szone->tiny_magazines[mag_index].mag_num_bytes_in_objects;
    }
    
    for (mag_index = -1; mag_index < szone->num_small_magazines; mag_index++) {
	s += szone->small_magazines[mag_index].mag_bytes_free_at_start;
	s += szone->small_magazines[mag_index].mag_bytes_free_at_end;
	t += szone->small_magazines[mag_index].mag_num_objects;
	u += szone->small_magazines[mag_index].mag_num_bytes_in_objects;
    }
    
    large = szone->num_bytes_in_large_objects + 0; // DEPRECATED szone->num_bytes_in_huge_objects;
    
    stats->blocks_in_use = t + szone->num_large_objects_in_use + 0; // DEPRECATED szone->num_huge_entries;
    stats->size_in_use = u + large;
    stats->max_size_in_use = stats->size_allocated = 
	(szone->num_tiny_regions - szone->num_tiny_regions_dealloc) * TINY_REGION_SIZE +
	(szone->num_small_regions - szone->num_small_regions_dealloc) * SMALL_REGION_SIZE + large; 
    // Now we account for the untouched areas
    stats->max_size_in_use -= s;
}

static void *
legacy_zeroing_large_malloc(szone_t *szone, size_t size) {
    if (size > LARGE_THRESHOLD) // Leopard and earlier returned a ZFOD range, so ...
	return szone_calloc(szone, 1, size); // Clear to zero always, ham-handedly touching in each page
    else
	return szone_malloc(szone, size);
}

static void *
legacy_zeroing_large_valloc(szone_t *szone, size_t size) {
    void *p = szone_valloc(szone, size);
    
    // Leopard and earlier returned a ZFOD range, so ...
    memset(p, 0, size); // Clear to zero always, ham-handedly touching in each page
    return p;
}

void zeroify_scalable_zone(malloc_zone_t *zone)
{
    szone_t	*szone = (szone_t *)zone;
    
    if (szone) {
	mprotect(szone, sizeof(szone->basic_zone), PROT_READ | PROT_WRITE);
	szone->basic_zone.malloc = (void *)legacy_zeroing_large_malloc;
	szone->basic_zone.valloc = (void *)legacy_zeroing_large_valloc;
	mprotect(szone, sizeof(szone->basic_zone), PROT_READ);
    }
}

static const struct malloc_introspection_t szone_introspect = {
    (void *)szone_ptr_in_use_enumerator,
    (void *)szone_good_size,
    (void *)szone_check,
    (void *)szone_print,
    szone_log,
    (void *)szone_force_lock,
    (void *)szone_force_unlock,
    (void *)szone_statistics,
    (void *)szone_locked,
    NULL, NULL, NULL, NULL, /* Zone enumeration version 7 and forward. */
}; // marked as const to spare the DATA section

malloc_zone_t *
create_scalable_zone(size_t initial_size, unsigned debug_flags)
{
    szone_t	*szone;
    uint64_t	hw_memsize = 0;

    /*
     * Sanity-check our build-time assumptions about the size of a page.
     * Since we have sized various things assuming the default page size,
     * attempting to determine it dynamically is not useful.
     */
    if ((vm_page_size != _vm_page_size) || (vm_page_shift != _vm_page_shift)) {
	malloc_printf("*** FATAL ERROR - machine page size does not match our assumptions.\n");
	exit(-1);
    }

#if defined(__i386__) || defined(__x86_64__)
    if (_COMM_PAGE_VERSION_REQD > (*((short *) _COMM_PAGE_VERSION))) { // _COMM_PAGE_CPU_NUMBER must be present at runtime
	malloc_printf("*** ERROR - comm page version mismatch.\n");
	exit(-1);
    }
#endif
    
    /* get memory for the zone. */
    szone = allocate_pages(NULL, SZONE_PAGED_SIZE, 0, 0, VM_MEMORY_MALLOC);
    if (!szone)
        return NULL;

    /* set up the szone structure */
#if 0
#warning CHECK_REGIONS enabled
    debug_flags |= CHECK_REGIONS;
#endif
#if 0
#warning LOG enabled
    szone->log_address = ~0;
#endif
    szone->trg[0].nextgen = &(szone->trg[1]);
    szone->trg[1].nextgen = &(szone->trg[0]);
    szone->tiny_region_generation = &(szone->trg[0]);
    
    szone->tiny_region_generation->hashed_regions = szone->initial_tiny_regions;
    szone->tiny_region_generation->num_regions_allocated = INITIAL_NUM_REGIONS;
    szone->tiny_region_generation->num_regions_allocated_shift = INITIAL_NUM_REGIONS_SHIFT;
    
    szone->srg[0].nextgen = &(szone->srg[1]);
    szone->srg[1].nextgen = &(szone->srg[0]);
    szone->small_region_generation = &(szone->srg[0]);
    
    szone->small_region_generation->hashed_regions = szone->initial_small_regions;
    szone->small_region_generation->num_regions_allocated = INITIAL_NUM_REGIONS;
    szone->small_region_generation->num_regions_allocated_shift = INITIAL_NUM_REGIONS_SHIFT;
    
    
    /*
     * Initialize variables that size the free list for SMALL allocations based
     * upon the amount of memory in the system.  Switch to a larger number of
     * free list entries at 1GB.
     */
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__)
    if ((hw_memsize = *(uint64_t *)(uintptr_t)_COMM_PAGE_MEMORY_SIZE) >= (1ULL << 30))
#else
    size_t	uint64_t_size = sizeof(hw_memsize);

    if (0 == sysctlbyname("hw.memsize", &hw_memsize, &uint64_t_size, 0, 0) &&
	hw_memsize >= (1ULL << 30))
#endif
    {
	szone->is_largemem = 1;
	szone->num_small_slots = NUM_SMALL_SLOTS_LARGEMEM;
	szone->large_threshold = LARGE_THRESHOLD_LARGEMEM;
	szone->vm_copy_threshold = VM_COPY_THRESHOLD_LARGEMEM;
    } else {
	szone->is_largemem = 0;
	szone->num_small_slots = NUM_SMALL_SLOTS;
	szone->large_threshold = LARGE_THRESHOLD;
	szone->vm_copy_threshold = VM_COPY_THRESHOLD;
    }
#if LARGE_CACHE
    szone->large_entry_cache_reserve_limit = 
        hw_memsize >> 10; // madvise(..., MADV_REUSABLE) death-row arrivals above this threshold [~0.1%]

    /* <rdar://problem/6610904> Reset protection when returning a previous large allocation? */
    int32_t libSystemVersion  = NSVersionOfLinkTimeLibrary("System");
    if ((-1 != libSystemVersion) && ((libSystemVersion >> 16) < 112) /* CFSystemVersionSnowLeopard */)
	szone->large_legacy_reset_mprotect = TRUE;
    else
	szone->large_legacy_reset_mprotect = FALSE;
#endif
    
    // Initialize the security token.
    szone->cookie = (uintptr_t)malloc_entropy[0];
    
    // Prepare ASLR
#if __i386__ || __LP64__ || TARGET_OS_EMBEDDED
#if __i386__
    uintptr_t stackbase = 0x8fe00000;
    int entropic_bits = 3;
#elif __LP64__
    uintptr_t stackbase = USRSTACK64;
    int entropic_bits = 16;
#else
    uintptr_t stackbase = USRSTACK;
    int entropic_bits = 3;
#endif
    if (0 != _dyld_get_image_slide((const struct mach_header*)_NSGetMachExecuteHeader())) {
	if (0 == entropic_address) {
	    uintptr_t t = stackbase - MAXSSIZ - ((uintptr_t) (malloc_entropy[1] & ((1 << entropic_bits) - 1)) << SMALL_BLOCKS_ALIGN);
	    (void)__sync_bool_compare_and_swap(&entropic_limit, 0, t); // Just one initialization please
	    (void)__sync_bool_compare_and_swap(&entropic_address, 0, t - ENTROPIC_KABILLION); // Just one initialization please
	}
	debug_flags &= ~DISABLE_ASLR;
    } else {
	debug_flags |= DISABLE_ASLR;
    }

#else
    debug_flags |= DISABLE_ASLR;
#endif
    
    szone->basic_zone.version = 8;
    szone->basic_zone.size = (void *)szone_size;
    szone->basic_zone.malloc = (void *)szone_malloc;
    szone->basic_zone.calloc = (void *)szone_calloc;
    szone->basic_zone.valloc = (void *)szone_valloc;
    szone->basic_zone.free = (void *)szone_free;
    szone->basic_zone.realloc = (void *)szone_realloc;
    szone->basic_zone.destroy = (void *)szone_destroy;
    szone->basic_zone.batch_malloc = (void *)szone_batch_malloc;
    szone->basic_zone.batch_free = (void *)szone_batch_free;
    szone->basic_zone.introspect = (struct malloc_introspection_t *)&szone_introspect;
    szone->basic_zone.memalign = (void *)szone_memalign;
    szone->basic_zone.free_definite_size = (void *)szone_free_definite_size;
    szone->basic_zone.pressure_relief = (void *)szone_pressure_relief;

    szone->basic_zone.reserved1 = 0; /* Set to zero once and for all as required by CFAllocator. */
    szone->basic_zone.reserved2 = 0; /* Set to zero once and for all as required by CFAllocator. */
    mprotect(szone, sizeof(szone->basic_zone), PROT_READ); /* Prevent overwriting the function pointers in basic_zone. */

    szone->debug_flags = debug_flags;
    LOCK_INIT(szone->large_szone_lock);
    
#if defined(__ppc__) || defined(__ppc64__)
    /*
     * In the interest of compatibility for PPC applications executing via Rosetta,
     * arrange to zero-fill allocations as occurred by side effect in Leopard and earlier.
     */
    zeroify_scalable_zone((malloc_zone_t *)szone);
#endif

#if defined(__i386__) || defined(__x86_64__)
    szone->cpu_id_key = (pthread_key_t) -1; // Unused. _COMM_PAGE_CPU_NUMBER preferred.
#else
    int		err;
    if ((err = pthread_key_create(&(szone->cpu_id_key), NULL))) {
 	malloc_printf("*** ERROR -pthread_key_create failure err=%d.\n", err);
	szone->cpu_id_key = (pthread_key_t) -1;
    }
#endif

    // Query the number of configured processors.
    // Uniprocessor case gets just one tiny and one small magazine (whose index is zero). This gives
    // the same behavior as the original scalable malloc. MP gets per-CPU magazines
    // that scale (way) better.
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__)
    int nproc = *(uint8_t *)(uintptr_t)_COMM_PAGE_NCPUS;
#else
    int nproc = sysconf(_SC_NPROCESSORS_CONF);
#endif
    szone->num_tiny_magazines = (nproc > 1) ? MIN(nproc, TINY_MAX_MAGAZINES) : 1;

    // FIXME vm_allocate() based on number of configured CPUs
    magazine_t *tiny_magazines = allocate_pages(NULL, TINY_MAGAZINE_PAGED_SIZE, 0, 
						    SCALABLE_MALLOC_ADD_GUARD_PAGES, VM_MEMORY_MALLOC);
    if (NULL == tiny_magazines)
	return NULL;

    szone->tiny_magazines = &(tiny_magazines[1]); // szone->tiny_magazines[-1] is the Depot

    // The magazines are indexed in [0 .. (num_tiny_magazines - 1)]
    // Find the smallest power of 2 that exceeds (num_tiny_magazines - 1)
    szone->num_tiny_magazines_mask_shift = 0;
    int i = 1;
     while( i <= (szone->num_tiny_magazines - 1) ) {
        szone->num_tiny_magazines_mask_shift++;
	i <<= 1;
    }
    
    // Now if i <= TINY_MAX_MAGAZINES we'll never access tiny_magazines[] out of bounds.
    if (i > TINY_MAX_MAGAZINES) {
 	malloc_printf("*** FATAL ERROR - magazine mask exceeds allocated magazines.\n");
	exit(-1);
    }
    
    // Reduce i by 1 to obtain a mask covering [0 .. (num_tiny_magazines - 1)]
    szone->num_tiny_magazines_mask = i - 1; // A mask used for hashing to a magazine index (and a safety aid)
#if TARGET_OS_EMBEDDED
    szone->last_tiny_advise = 0;
#endif
    
    // Init the tiny_magazine locks
    LOCK_INIT(szone->tiny_regions_lock);
    LOCK_INIT(szone->tiny_magazines[DEPOT_MAGAZINE_INDEX].magazine_lock);
    for (i = 0; i < szone->num_tiny_magazines; ++i) {
	LOCK_INIT(szone->tiny_magazines[i].magazine_lock);
    }

    szone->num_small_magazines = (nproc > 1) ? MIN(nproc, SMALL_MAX_MAGAZINES) : 1;

    // FIXME vm_allocate() based on number of configured CPUs
    magazine_t *small_magazines = allocate_pages(NULL, SMALL_MAGAZINE_PAGED_SIZE, 0, 
						    SCALABLE_MALLOC_ADD_GUARD_PAGES, VM_MEMORY_MALLOC);
    if (NULL == small_magazines)
	return NULL;

    szone->small_magazines = &(small_magazines[1]); // szone->small_magazines[-1] is the Depot
    
    // The magazines are indexed in [0 .. (num_small_magazines - 1)]
    // Find the smallest power of 2 that exceeds (num_small_magazines - 1)
    szone->num_small_magazines_mask_shift = 0;
     while( i <= (szone->num_small_magazines - 1) ) {
        szone->num_small_magazines_mask_shift++;
	i <<= 1;
    }
    
    // Now if i <= SMALL_MAX_MAGAZINES we'll never access small_magazines[] out of bounds.
    if (i > SMALL_MAX_MAGAZINES) {
 	malloc_printf("*** FATAL ERROR - magazine mask exceeds allocated magazines.\n");
	exit(-1);
    }
    
    // Reduce i by 1 to obtain a mask covering [0 .. (num_small_magazines - 1)]
    szone->num_small_magazines_mask = i - 1; // A mask used for hashing to a magazine index (and a safety aid)
#if TARGET_OS_EMBEDDED
    szone->last_small_advise = 0;
#endif
    
    // Init the small_magazine locks
    LOCK_INIT(szone->small_regions_lock);
    LOCK_INIT(szone->small_magazines[DEPOT_MAGAZINE_INDEX].magazine_lock);
    for (i = 0; i < szone->num_small_magazines; ++i) {
	LOCK_INIT(szone->small_magazines[i].magazine_lock);
    }

    CHECK(szone, __PRETTY_FUNCTION__);
    return (malloc_zone_t *)szone;
}

//
// purgeable zones have their own "large" allocation pool, but share "tiny" and "large" 
// heaps with a helper_zone identified in the call to create_purgeable_zone()
//
static size_t
purgeable_size(szone_t *szone, const void *ptr)
{
    // Only claim our large allocations, leave the shared tiny/small for the helper zone to claim.
    return szone_size_try_large(szone, ptr);
}

static void *
purgeable_malloc(szone_t *szone, size_t size) {
    if (size <= szone->large_threshold)
	return szone_malloc(szone->helper_zone, size);
    else
	return szone_malloc(szone, size);
}

static void *
purgeable_calloc(szone_t *szone, size_t num_items, size_t size)
{
    size_t total_bytes = num_items * size;
    
    // Check for overflow of integer multiplication
    if (num_items > 1) {
#if __LP64__ /* size_t is uint64_t */
	if ((num_items | size) & 0xffffffff00000000ul) {
	    // num_items or size equals or exceeds sqrt(2^64) == 2^32, appeal to wider arithmetic
		__uint128_t product = ((__uint128_t)num_items) * ((__uint128_t)size);
	    if ((uint64_t)(product >> 64)) // compiles to test on upper register of register pair
	        return NULL;
	}
#else /* size_t is uint32_t */
	if ((num_items | size) & 0xffff0000ul) {
	    // num_items or size equals or exceeds sqrt(2^32) == 2^16, appeal to wider arithmetic
		uint64_t product = ((uint64_t)num_items) * ((uint64_t)size);
	    if ((uint32_t)(product >> 32)) // compiles to test on upper register of register pair
	        return NULL;
	}
#endif
    }
    
    if (total_bytes <= szone->large_threshold)
	return szone_calloc(szone->helper_zone, 1, total_bytes);
    else
	return szone_calloc(szone, 1, total_bytes);
}

static void *
purgeable_valloc(szone_t *szone, size_t size)
{
    if (size <= szone->large_threshold)
	return szone_valloc(szone->helper_zone, size);
    else
	return szone_valloc(szone, size);
}

static void
purgeable_free(szone_t *szone, void *ptr)
{
    large_entry_t	*entry;

    SZONE_LOCK(szone);
    entry = large_entry_for_pointer_no_lock(szone, ptr);
    SZONE_UNLOCK(szone); 
    if (entry) {
	return free_large(szone, ptr);
    } else {
	return szone_free(szone->helper_zone, ptr);
    }
}
    
static void
purgeable_free_definite_size(szone_t *szone, void *ptr, size_t size)
{
    if (size <= szone->large_threshold)
	return szone_free_definite_size(szone->helper_zone, ptr, size);
    else
	return szone_free_definite_size(szone, ptr, size);
}

static void *
purgeable_realloc(szone_t *szone, void *ptr, size_t new_size)
{
    size_t old_size;

    if (NULL == ptr) {
	// If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
	return purgeable_malloc(szone, new_size);
    } else if (0 == new_size) {
	// If size is 0 and ptr is not a null pointer, the object pointed to is freed.
	purgeable_free(szone, ptr);
	// If size is 0, either a null pointer or a unique pointer that can be successfully passed 
	// to free() shall be returned.
	return purgeable_malloc(szone, 1);
    }

    old_size = purgeable_size(szone, ptr); // Now ptr can be safely size()'d
    if (!old_size) 
	old_size = szone_size(szone->helper_zone, ptr);

    if (!old_size) {
	szone_error(szone, 1, "pointer being reallocated was not allocated", ptr, NULL);
	return NULL;
    }

    // Distinguish 4 cases: {oldsize, newsize} x { <= , > large_threshold }
    // and deal with the allocation crossing from the purgeable zone to the helper zone and vice versa. 
    if (old_size <= szone->large_threshold) {
	if (new_size <= szone->large_threshold)
	    return szone_realloc(szone->helper_zone, ptr, new_size);
	else {
	    // allocation crosses from helper to purgeable zone
	    void * new_ptr = purgeable_malloc(szone, new_size);
	    if (new_ptr) {
		memcpy(new_ptr, ptr, old_size);
		szone_free_definite_size(szone->helper_zone, ptr, old_size);
	    }
	    return new_ptr; // in state VM_PURGABLE_NONVOLATILE
	}
    } else {
	if (new_size <= szone->large_threshold) {
	    // allocation crosses from purgeable to helper zone
	    void * new_ptr = szone_malloc(szone->helper_zone, new_size);
	    if (new_ptr) {
		memcpy(new_ptr, ptr, new_size);
		purgeable_free_definite_size(szone, ptr, old_size);
	    }
	    return new_ptr;
	} else {
	    void * new_ptr = purgeable_malloc(szone, new_size);
	    if (new_ptr) {
		memcpy(new_ptr, ptr, MIN(old_size, new_size));
		purgeable_free_definite_size(szone, ptr, old_size);
	    }
	    return new_ptr; // in state VM_PURGABLE_NONVOLATILE
	}
    }
    /* NOTREACHED */
}

static void
purgeable_destroy(szone_t *szone)
{
    /* destroy large entries */
    size_t index = szone->num_large_entries;
    large_entry_t	*large;
    vm_range_t		range_to_deallocate;

    while (index--) {
	large = szone->large_entries + index;
	if (large->address) {
	    // we deallocate_pages, including guard pages
	    deallocate_pages(szone, (void *)(large->address), large->size, szone->debug_flags);
	}
    }
    large_entries_free_no_lock(szone, szone->large_entries, szone->num_large_entries, &range_to_deallocate);
    if (range_to_deallocate.size)
	deallocate_pages(szone, (void *)range_to_deallocate.address, (size_t)range_to_deallocate.size, 0);
    
    /* Now destroy the separate szone region */
    deallocate_pages(szone, (void *)szone, SZONE_PAGED_SIZE, 0);
}

static unsigned
purgeable_batch_malloc(szone_t *szone, size_t size, void **results, unsigned count)
{
    return szone_batch_malloc(szone->helper_zone, size, results, count);
}

static void
purgeable_batch_free(szone_t *szone, void **to_be_freed, unsigned count)
{
    return szone_batch_free(szone->helper_zone, to_be_freed, count);
}

static void *
purgeable_memalign(szone_t *szone, size_t alignment, size_t size)
{
    if (size <= szone->large_threshold)
	return szone_memalign(szone->helper_zone, alignment, size);
    else
	return szone_memalign(szone, alignment, size);
}

static kern_return_t
purgeable_ptr_in_use_enumerator(task_t task, void *context, unsigned type_mask, vm_address_t zone_address, 
			    memory_reader_t reader, vm_range_recorder_t recorder)
{
    szone_t		*szone;
    kern_return_t	err;
    
    if (!reader) reader = _szone_default_reader;
    
    err = reader(task, zone_address, sizeof(szone_t), (void **)&szone);
    if (err) return err;

    err = large_in_use_enumerator(task, context, type_mask, 
      (vm_address_t)szone->large_entries, szone->num_large_entries, reader, recorder);
    return err;
}

static size_t
purgeable_good_size(szone_t *szone, size_t size)
{
    if (size <= szone->large_threshold)
	return szone_good_size(szone->helper_zone, size);
    else
	return szone_good_size(szone, size);
}

static boolean_t
purgeable_check(szone_t *szone)
{
    return 1;
}

static void
purgeable_print(szone_t *szone, boolean_t verbose)
{
    _malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
                 "Scalable zone %p: inUse=%u(%y) flags=%d\n",
                 szone, szone->num_large_objects_in_use, szone->num_bytes_in_large_objects, szone->debug_flags);
}

static void
purgeable_log(malloc_zone_t *zone, void *log_address)
{
    szone_t	*szone = (szone_t *)zone;

    szone->log_address = log_address;
}

static void
purgeable_force_lock(szone_t *szone)
{    
    SZONE_LOCK(szone);
}

static void
purgeable_force_unlock(szone_t *szone)
{    
    SZONE_UNLOCK(szone);
}

static void
purgeable_statistics(szone_t *szone, malloc_statistics_t *stats)
{
    stats->blocks_in_use = szone->num_large_objects_in_use;
    stats->size_in_use = stats->max_size_in_use = stats->size_allocated = szone->num_bytes_in_large_objects;
}

static boolean_t
purgeable_locked(szone_t *szone) 
{
  int tookLock;

  tookLock = SZONE_TRY_LOCK(szone);
  if (tookLock == 0)
    return 1;
  SZONE_UNLOCK(szone);
  return 0;
}

static size_t
purgeable_pressure_relief(szone_t *szone, size_t goal)
{
    return szone_pressure_relief(szone, goal) + szone_pressure_relief(szone->helper_zone, goal);
}

static const struct malloc_introspection_t purgeable_introspect = {
    (void *)purgeable_ptr_in_use_enumerator,
    (void *)purgeable_good_size,
    (void *)purgeable_check,
    (void *)purgeable_print,
    purgeable_log,
    (void *)purgeable_force_lock,
    (void *)purgeable_force_unlock,
    (void *)purgeable_statistics,
    (void *)purgeable_locked,
    NULL, NULL, NULL, NULL, /* Zone enumeration version 7 and forward. */
}; // marked as const to spare the DATA section

__private_extern__ malloc_zone_t *
create_purgeable_zone(size_t initial_size, malloc_zone_t *malloc_default_zone, unsigned debug_flags)
{
    szone_t	*szone;
    uint64_t	hw_memsize = 0;
        
    /* get memory for the zone. */
    szone = allocate_pages(NULL, SZONE_PAGED_SIZE, 0, 0, VM_MEMORY_MALLOC);
    if (!szone)
        return NULL;

    /* set up the szone structure */
#if 0
#warning LOG enabled
    szone->log_address = ~0;
#endif
    
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__)
    hw_memsize = *(uint64_t *)(uintptr_t)_COMM_PAGE_MEMORY_SIZE;
#else
    size_t	uint64_t_size = sizeof(hw_memsize);

    sysctlbyname("hw.memsize", &hw_memsize, &uint64_t_size, 0, 0);
#endif
	/* Purgeable zone does not participate in the adaptive "largemem" sizing. */
    szone->is_largemem = 0;
    szone->large_threshold = LARGE_THRESHOLD;
    szone->vm_copy_threshold = VM_COPY_THRESHOLD;

#if LARGE_CACHE
    szone->large_entry_cache_reserve_limit = 
        hw_memsize >> 10; // madvise(..., MADV_REUSABLE) death-row arrivals above this threshold [~0.1%]

    /* <rdar://problem/6610904> Reset protection when returning a previous large allocation? */
    int32_t libSystemVersion  = NSVersionOfLinkTimeLibrary("System");
    if ((-1 != libSystemVersion) && ((libSystemVersion >> 16) < 112) /* CFSystemVersionSnowLeopard */)
	szone->large_legacy_reset_mprotect = TRUE;
    else
	szone->large_legacy_reset_mprotect = FALSE;
#endif

    szone->basic_zone.version = 8;
    szone->basic_zone.size = (void *)purgeable_size;
    szone->basic_zone.malloc = (void *)purgeable_malloc;
    szone->basic_zone.calloc = (void *)purgeable_calloc;
    szone->basic_zone.valloc = (void *)purgeable_valloc;
    szone->basic_zone.free = (void *)purgeable_free;
    szone->basic_zone.realloc = (void *)purgeable_realloc;
    szone->basic_zone.destroy = (void *)purgeable_destroy;
    szone->basic_zone.batch_malloc = (void *)purgeable_batch_malloc;
    szone->basic_zone.batch_free = (void *)purgeable_batch_free;
    szone->basic_zone.introspect = (struct malloc_introspection_t *)&purgeable_introspect;
    szone->basic_zone.memalign = (void *)purgeable_memalign;
    szone->basic_zone.free_definite_size = (void *)purgeable_free_definite_size;
    szone->basic_zone.pressure_relief = (void *)purgeable_pressure_relief;

    szone->basic_zone.reserved1 = 0; /* Set to zero once and for all as required by CFAllocator. */
    szone->basic_zone.reserved2 = 0; /* Set to zero once and for all as required by CFAllocator. */
    mprotect(szone, sizeof(szone->basic_zone), PROT_READ); /* Prevent overwriting the function pointers in basic_zone. */
    
    szone->debug_flags = debug_flags | SCALABLE_MALLOC_PURGEABLE;
    
    /* Purgeable zone does not support SCALABLE_MALLOC_ADD_GUARD_PAGES. */
    if (szone->debug_flags & SCALABLE_MALLOC_ADD_GUARD_PAGES) {
	_malloc_printf(ASL_LEVEL_INFO, "purgeable zone does not support guard pages\n");
	szone->debug_flags &= ~SCALABLE_MALLOC_ADD_GUARD_PAGES;
    }

    LOCK_INIT(szone->large_szone_lock);

    szone->helper_zone = (struct szone_s *)malloc_default_zone;

    CHECK(szone, __PRETTY_FUNCTION__);
    return (malloc_zone_t *)szone;
}

/*
 * For use by CheckFix: create a new zone whose behavior is, apart from
 * the use of death-row and per-CPU magazines, that of Leopard.
 */
static NOINLINE void *
legacy_valloc(szone_t *szone, size_t size)
{
    void	*ptr;
    size_t num_pages;

    num_pages = round_page(size) >> vm_page_shift;
    ptr = large_malloc(szone, num_pages, 0, TRUE);
#if DEBUG_MALLOC
    if (LOG(szone, ptr))
    malloc_printf("legacy_valloc returned %p\n", ptr);
#endif
    return ptr;
}

__private_extern__ malloc_zone_t *
create_legacy_scalable_zone(size_t initial_size, unsigned debug_flags)
{
    malloc_zone_t *mzone = create_scalable_zone(initial_size, debug_flags);
    szone_t	*szone = (szone_t *)mzone;

    if (!szone)
        return NULL;

    szone->is_largemem = 0;
    szone->num_small_slots = NUM_SMALL_SLOTS;
    szone->large_threshold = LARGE_THRESHOLD;
    szone->vm_copy_threshold = VM_COPY_THRESHOLD;
    
    mprotect(szone, sizeof(szone->basic_zone), PROT_READ | PROT_WRITE);
    szone->basic_zone.valloc = (void *)legacy_valloc;
    szone->basic_zone.free_definite_size = NULL;
    mprotect(szone, sizeof(szone->basic_zone), PROT_READ);
    
    return mzone;
}

/********* Support code for emacs unexec ************/

/* History of freezedry version numbers:
 *
 * 1) Old malloc (before the scalable malloc implementation in this file
 *    existed).
 * 2) Original freezedrying code for scalable malloc.  This code was apparently
 *    based on the old freezedrying code and was fundamentally flawed in its
 *    assumption that tracking allocated memory regions was adequate to fake
 *    operations on freezedried memory.  This doesn't work, since scalable
 *    malloc does not store flags in front of large page-aligned allocations.
 * 3) Original szone-based freezedrying code.
 * 4) Fresher malloc with tiny zone
 * 5) 32/64bit compatible malloc
 * 6) Metadata within 1MB and 8MB region for tiny and small
 *
 * No version backward compatibility is provided, but the version number does
 * make it possible for malloc_jumpstart() to return an error if the application
 * was freezedried with an older version of malloc.
 */
#define MALLOC_FREEZEDRY_VERSION 6

typedef struct {
    unsigned	version;
    unsigned	nszones;
    szone_t	*szones;
} malloc_frozen;

static void *
frozen_malloc(szone_t *zone, size_t new_size)
{
    return malloc(new_size);
}

static void *
frozen_calloc(szone_t *zone, size_t num_items, size_t size)
{
    return calloc(num_items, size);
}

static void *
frozen_valloc(szone_t *zone, size_t new_size)
{
    return valloc(new_size);
}

static void *
frozen_realloc(szone_t *zone, void *ptr, size_t new_size)
{
    size_t	old_size = szone_size(zone, ptr);
    void	*new_ptr;
    
    if (new_size <= old_size) {
	return ptr;
    }
    new_ptr = malloc(new_size);
    if (old_size > 0) {
	memcpy(new_ptr, ptr, old_size);
    }
    return new_ptr;
}

static void
frozen_free(szone_t *zone, void *ptr)
{
}

static void
frozen_destroy(szone_t *zone)
{
}

/********* Pseudo-private API for emacs unexec ************/

/*
 * malloc_freezedry() records all of the szones in use, so that they can be
 * partially reconstituted by malloc_jumpstart().  Due to the differences
 * between reconstituted memory regions and those created by the szone code,
 * care is taken not to reallocate from the freezedried memory, except in the
 * case of a non-growing realloc().
 *
 * Due to the flexibility provided by the zone registration mechanism, it is
 * impossible to implement generic freezedrying for any zone type.  This code
 * only handles applications that use the szone allocator, so malloc_freezedry()
 * returns 0 (error) if any non-szone zones are encountered.
 */

uintptr_t
malloc_freezedry(void)
{
    extern unsigned malloc_num_zones;
    extern malloc_zone_t **malloc_zones;
    malloc_frozen *data;
    unsigned i;

    /* Allocate space in which to store the freezedry state. */
    data = (malloc_frozen *) malloc(sizeof(malloc_frozen));

    /* Set freezedry version number so that malloc_jumpstart() can check for compatibility. */
    data->version = MALLOC_FREEZEDRY_VERSION;

    /* Allocate the array of szone pointers. */
    data->nszones = malloc_num_zones;
    data->szones = (szone_t *) calloc(malloc_num_zones, sizeof(szone_t));

    /*
     * Fill in the array of szone structures.  They are copied rather than
     * referenced, since the originals are likely to be clobbered during malloc
     * initialization.
     */
    for (i = 0; i < malloc_num_zones; i++) {
	if (strcmp(malloc_zones[i]->zone_name, "DefaultMallocZone")) {
	    /* Unknown zone type. */
	    free(data->szones);
	    free(data);
	    return 0;
	}
	memcpy(&data->szones[i], malloc_zones[i], sizeof(szone_t));
    }

    return((uintptr_t)data);
}

int
malloc_jumpstart(uintptr_t cookie)
{
    malloc_frozen *data = (malloc_frozen *)cookie;
    unsigned i;

    if (data->version != MALLOC_FREEZEDRY_VERSION) {
	/* Unsupported freezedry version. */
	return 1;
    }

    for (i = 0; i < data->nszones; i++) {
	/* Set function pointers.  Even the functions that stay the same must be
	 * set, since there are no guarantees that they will be mapped to the
	 * same addresses. */
	data->szones[i].basic_zone.size = (void *) szone_size;
	data->szones[i].basic_zone.malloc = (void *) frozen_malloc;
	data->szones[i].basic_zone.calloc = (void *) frozen_calloc;
	data->szones[i].basic_zone.valloc = (void *) frozen_valloc;
	data->szones[i].basic_zone.free = (void *) frozen_free;
	data->szones[i].basic_zone.realloc = (void *) frozen_realloc;
	data->szones[i].basic_zone.destroy = (void *) frozen_destroy;
	data->szones[i].basic_zone.introspect = (struct malloc_introspection_t *)&szone_introspect;

	/* Register the freezedried zone. */
	malloc_zone_register(&data->szones[i].basic_zone);
    }

    return 0;
}
