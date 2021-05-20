#define PRIVATE 1
#include <sys/cdefs.h>

#include <darling/emulation/ext/for-xtrace.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/queue.h>

#include "malloc.h"
#include "lock.h"
#include <darling/emulation/simple.h>

#ifndef XTRACE_MALLOC_DEBUG
	#define XTRACE_MALLOC_DEBUG 0
#endif

#if XTRACE_MALLOC_DEBUG
	#define xtrace_malloc_debug(x, ...) xtrace_printf(x "\n", ## __VA_ARGS__)
	#undef XTRACE_INLINE
	#define XTRACE_INLINE
#else
	#define xtrace_malloc_debug(x, ...)
#endif

// we can't rely on libmalloc because:
// 1. we should be invisible to everyone but libkernel
// 2. libmalloc might need to `thread_switch(2)`, which will recurse back into xtrace, blowing up the stack
// so let's roll our own malloc! (using Linux's mmap)

// all blocks will be allocated in multiples of this size
#define BLOCK_SIZE_MULTIPLE (4096ULL)

#define LIST_PREV(elm, link) ({ \
		__typeof(elm) elm_cache = elm; \
		__typeof((elm_cache)->link)* as_link = __container_of(elm_cache->link.le_prev, __typeof((elm_cache)->link), le_next); \
		elm_cache = __container_of(as_link, __typeof(*elm_cache), link); \
		elm_cache; \
	})

typedef LIST_HEAD(xtrace_memory_fragment_head, xtrace_memory_fragment) xtrace_memory_fragment_head_t;
typedef LIST_ENTRY(xtrace_memory_fragment) xtrace_memory_fragment_entry_t;

//
// xtrace_memory_fragment
//

#define XTRACE_MEMORY_FRAGMENT_FLAG_IN_USE (1ULL << 63)
#define XTRACE_MEMORY_FRAGMENT_FLAG_IS_MMAP_BASE (1ULL << 62)
#define XTRACE_MEMORY_FRAGMENT_FLAG_BIT_COUNT (2)
#define XTRACE_MEMORY_FRAGMENT_SIZE_MASK (0xffffffffffffffffULL >> XTRACE_MEMORY_FRAGMENT_FLAG_BIT_COUNT)

typedef struct xtrace_memory_fragment* xtrace_memory_fragment_t;
struct xtrace_memory_fragment {
	uint64_t flags;
	xtrace_memory_fragment_entry_t block_link;
	xtrace_memory_fragment_entry_t free_link;
	char memory[];
};

XTRACE_INLINE
bool xtrace_memory_fragment_in_use(xtrace_memory_fragment_t fragment) {
	return fragment->flags & XTRACE_MEMORY_FRAGMENT_FLAG_IN_USE;
};

XTRACE_INLINE
void xtrace_memory_fragment_set_in_use(xtrace_memory_fragment_t fragment, bool in_use) {
	if (in_use) {
		fragment->flags |= XTRACE_MEMORY_FRAGMENT_FLAG_IN_USE;
	} else {
		fragment->flags &= ~XTRACE_MEMORY_FRAGMENT_FLAG_IN_USE;
	}
};

XTRACE_INLINE
bool xtrace_memory_fragment_is_mmap_base(xtrace_memory_fragment_t fragment) {
	return fragment->flags & XTRACE_MEMORY_FRAGMENT_FLAG_IS_MMAP_BASE;
};

XTRACE_INLINE
void xtrace_memory_fragment_set_is_mmap_base(xtrace_memory_fragment_t fragment, bool is_mmap_base) {
	if (is_mmap_base) {
		fragment->flags |= XTRACE_MEMORY_FRAGMENT_FLAG_IS_MMAP_BASE;
	} else {
		fragment->flags &= ~XTRACE_MEMORY_FRAGMENT_FLAG_IS_MMAP_BASE;
	}
};

XTRACE_INLINE
size_t xtrace_memory_fragment_size(xtrace_memory_fragment_t fragment) {
	return fragment->flags & XTRACE_MEMORY_FRAGMENT_SIZE_MASK;
};

XTRACE_INLINE
void xtrace_memory_fragment_set_size(xtrace_memory_fragment_t fragment, size_t new_size) {
	fragment->flags = (fragment->flags & ~XTRACE_MEMORY_FRAGMENT_SIZE_MASK) | (new_size & XTRACE_MEMORY_FRAGMENT_SIZE_MASK);
};

XTRACE_INLINE
void xtrace_memory_fragment_init(xtrace_memory_fragment_t fragment, size_t size) {
	xtrace_malloc_debug("initializing fragment of %llu bytes at %p", size, fragment);
	fragment->flags = size & XTRACE_MEMORY_FRAGMENT_SIZE_MASK;
	fragment->block_link.le_next = NULL;
	fragment->block_link.le_prev = NULL;
	fragment->free_link.le_next = NULL;
	fragment->free_link.le_prev = NULL;
};

//
// xtrace_memory_block
//

typedef struct xtrace_memory_block* xtrace_memory_block_t;
struct xtrace_memory_block {
	uint64_t size;
	xtrace_memory_fragment_head_t fragment_list;
	struct xtrace_memory_fragment fragment;
};

XTRACE_INLINE
bool xtrace_memory_block_in_use(xtrace_memory_block_t block) {
	// the block is in use if either:
	//   * the first fragment is in use, or
	//   * the block has been fragmented
	return xtrace_memory_fragment_in_use(&block->fragment) || (block->size != xtrace_memory_fragment_size(&block->fragment));
};

XTRACE_INLINE
void xtrace_memory_block_init(xtrace_memory_block_t block, size_t size) {
	block->size = size;
	LIST_INIT(&block->fragment_list);
	xtrace_memory_fragment_init(&block->fragment, block->size);
	xtrace_memory_fragment_set_is_mmap_base(&block->fragment, true);
	LIST_INSERT_HEAD(&block->fragment_list, &block->fragment, block_link);
};

//
// global variables
//

// list of available fragments
static xtrace_memory_fragment_head_t free_list = LIST_HEAD_INITIALIZER(free_list);
// lock for free_list
static xtrace_lock_t free_lock = XTRACE_LOCK_INITIALIZER;

//
// internal functions
//

static xtrace_memory_fragment_t allocate_fragment(size_t required_size);
static void release_fragment(xtrace_memory_fragment_t fragment);
static bool shrink_fragment(xtrace_memory_fragment_t fragment, size_t new_size);
static bool expand_fragment(xtrace_memory_fragment_t fragment, size_t new_size);

// borrowed from libgmalloc
XTRACE_INLINE
size_t round_up(size_t size, size_t increment) {
	if ((size & (increment - 1)) == 0) {
		return size;
	}
	return (size | (increment - 1)) + 1;
}

static xtrace_memory_fragment_t allocate_fragment(size_t required_size) {
	xtrace_memory_fragment_t viable_fragment = NULL;
	size_t viable_fragment_size = 0;
	xtrace_memory_fragment_t loop_var = NULL;
	xtrace_lock_lock(&free_lock);

	// look for the smallest free fragment
	LIST_FOREACH(loop_var, &free_list, free_link) {
		size_t loop_var_size = xtrace_memory_fragment_size(loop_var);

		// 1. always make sure the current fragment is large enough
		// 2. we'll prefer the current fragment over the previous candidate if either:
		//      * we don't currently have a candidate fragment, or
		//      * the current fragment is smaller than the previous candidate
		if (loop_var_size > required_size && (viable_fragment == NULL || loop_var_size < viable_fragment_size)) {
			viable_fragment = loop_var;
			viable_fragment_size = loop_var_size;
			xtrace_malloc_debug("found viable fragment with address %p and size %llu", viable_fragment->memory, viable_fragment_size);
		}
	}

	// if we didn't find a viable candidate...
	if (viable_fragment == NULL) {
		// ...allocate some memory
		size_t allocation_size = round_up(required_size, BLOCK_SIZE_MULTIPLE);
		xtrace_memory_block_t block = _mmap_for_xtrace(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

		xtrace_malloc_debug("mmap for %llu bytes returned %p", allocation_size, block);

		// check if return value is an error code
		// see similar check in libkernel's `mman.c`
		if ((unsigned long)block > (unsigned long)-4096) {
			// we failed to allocate the memory, we can't do anything else
			xtrace_malloc_debug("mmap result was failure");
			goto out;
		}

		// initialize it
		xtrace_memory_block_init(block, allocation_size - sizeof(struct xtrace_memory_block));

		// add it to the free list
		LIST_INSERT_HEAD(&free_list, &block->fragment, free_link);

		// and set `viable_fragment` so our code below can handle it the same way as the no-mmap case
		viable_fragment = &block->fragment;
	}

	// at this point, `viable_fragment` is guaranteed to have a value
	xtrace_malloc_debug("final fragment for allocation: %llu bytes at %p", xtrace_memory_fragment_size(viable_fragment), viable_fragment->memory);

	// shrink the fragment (if possible)
	shrink_fragment(viable_fragment, required_size);

	// set the in_use bit
	xtrace_memory_fragment_set_in_use(viable_fragment, true);

	// finally, remove the viable fragment from the free list
	LIST_REMOVE(viable_fragment, free_link);

out:
	xtrace_lock_unlock(&free_lock);

	return viable_fragment;
};

static void release_fragment(xtrace_memory_fragment_t fragment) {
	xtrace_lock_lock(&free_lock);

	xtrace_malloc_debug("releasing fragment of %llu bytes at %p", xtrace_memory_fragment_size(fragment), fragment->memory);

	// clear the in_use bit
	xtrace_memory_fragment_set_in_use(fragment, false);

	// add it to the free list
	LIST_INSERT_HEAD(&free_list, fragment, free_link);

	// try to defragment the block
	while (true) {
		xtrace_memory_fragment_t prev_frag = xtrace_memory_fragment_is_mmap_base(fragment) ? NULL : LIST_PREV(fragment, block_link);
		xtrace_memory_fragment_t next_frag = LIST_NEXT(fragment, block_link);
		bool defragged = false;

		if (prev_frag && !xtrace_memory_fragment_in_use(prev_frag)) {
			xtrace_malloc_debug("defragmenting with previous fragment (%llu bytes at %p)", xtrace_memory_fragment_size(prev_frag), prev_frag->memory);
			defragged = true;

			// remove the current fragment from the free list
			LIST_REMOVE(fragment, free_link);

			// remove the current fragment from the block list
			LIST_REMOVE(fragment, block_link);

			// expand the previous fragment
			xtrace_memory_fragment_set_size(prev_frag, xtrace_memory_fragment_size(prev_frag) + sizeof(struct xtrace_memory_fragment) + xtrace_memory_fragment_size(fragment));

			// `fragment` is now gone; set it to `prev_frag`
			fragment = prev_frag;
		}

		if (next_frag && !xtrace_memory_fragment_in_use(next_frag)) {
			xtrace_malloc_debug("defragmenting with next fragment (%llu bytes at %p)", xtrace_memory_fragment_size(fragment), fragment->memory);
			defragged = true;

			// remove the next fragment from the free list
			LIST_REMOVE(next_frag, free_link);

			// remove the next fragment from the block list
			LIST_REMOVE(next_frag, block_link);

			// expand the current fragment
			xtrace_memory_fragment_set_size(fragment, xtrace_memory_fragment_size(fragment) + sizeof(struct xtrace_memory_fragment) + xtrace_memory_fragment_size(next_frag));

			// `next_frag` is now gone
		}

		// if we didn't defragment anything on this loop, we can't defragment any more
		if (!defragged) {
			xtrace_malloc_debug("cannot defragment any further");
			break;
		} else {
			// otherwise, keep looping to try to defragment more
			xtrace_malloc_debug("doing another defragmentation iteration");
		}
	}

	xtrace_malloc_debug("final releasing fragment is %llu bytes at %p", xtrace_memory_fragment_size(fragment), fragment->memory);

	// if `fragment` is the mmap base fragment...
	if (xtrace_memory_fragment_is_mmap_base(fragment)) {
		xtrace_memory_block_t block = __container_of(fragment, struct xtrace_memory_block, fragment);
		xtrace_malloc_debug("releasing fragment is base fragment");

		// ...and we managed to defragment the entire block...
		if (!xtrace_memory_block_in_use(block)) {
			// ...unmap it!
			xtrace_malloc_debug("unmapping completely freed block (%llu bytes at %p)", block->size, block->fragment.memory);

			// start off by removing all traces of this block in the free list.
			// at this point, only the base fragment should be in the free list, so remove it
			LIST_REMOVE(fragment, free_link);

			// and now, just unmap it
			_munmap_for_xtrace(block, block->size + sizeof(struct xtrace_memory_block));
			// and we ignore errors
		}
	}

out:
	xtrace_lock_unlock(&free_lock);
};

// must be called with free_lock held
static bool shrink_fragment(xtrace_memory_fragment_t fragment, size_t new_size) {
	xtrace_malloc_debug("shrink to %llu bytes requested for fragment of %llu bytes at %p", new_size, xtrace_memory_fragment_size(fragment), fragment->memory);

	// calculate the remaining size if we were to shrink this fragment
	size_t remaining_size = xtrace_memory_fragment_size(fragment) - new_size;

	// if we would have enough space to create another fragment after shrinking this one...
	if (remaining_size > sizeof(struct xtrace_memory_fragment)) {
		xtrace_malloc_debug("fragment eligible; shrinking...");

		// ...shrink it...
		xtrace_memory_fragment_set_size(fragment, new_size);

		// ...and create the new fragment
		xtrace_memory_fragment_t new_fragment = (xtrace_memory_fragment_t)(fragment->memory + new_size);
		xtrace_memory_fragment_init(new_fragment, remaining_size - sizeof(struct xtrace_memory_fragment));

		// add it to the block list
		LIST_INSERT_AFTER(fragment, new_fragment, block_link);

		xtrace_malloc_debug("old fragment shrunk to %llu bytes at %p", xtrace_memory_fragment_size(fragment), fragment->memory);
		xtrace_malloc_debug("new fragment of %llu bytes created at %p", xtrace_memory_fragment_size(new_fragment), new_fragment->memory);

		// insert it into the free list
		LIST_INSERT_HEAD(&free_list, new_fragment, free_link);

		return true;
	} else {
		// otherwise, we don't want to shrink this one to avoid holes in our block
		xtrace_malloc_debug("fragment ineligible for shrinking (not enough space after it)");
	}

	return false;
};

// must be called with free_lock held
static bool expand_fragment(xtrace_memory_fragment_t fragment, size_t new_size) {
	xtrace_malloc_debug("expansion to %llu bytes requested for fragment of %llu bytes at %p", new_size, xtrace_memory_fragment_size(fragment), fragment->memory);

	size_t available_size = xtrace_memory_fragment_size(fragment);
	xtrace_memory_fragment_t loop_var = fragment;
	xtrace_memory_fragment_t end_frag = NULL;

	while ((end_frag = LIST_NEXT(loop_var, block_link)) != NULL) {
		// if the fragment is in use...
		if (xtrace_memory_fragment_in_use(end_frag)) {
			// ...we've reached a limit
			xtrace_malloc_debug("insufficient free fragments for expansion");
			break;
		}

		available_size += xtrace_memory_fragment_size(end_frag) + sizeof(struct xtrace_memory_fragment);
		loop_var = end_frag;

		// if we've got enough...
		if (available_size >= new_size) {
			xtrace_malloc_debug("found enough free fragments for expansion");
			// ...we're done; we don't need to keep looking for more
			break;
		}
	}

	if (end_frag && available_size >= new_size) {
		xtrace_memory_fragment_t terminating_value = LIST_NEXT(end_frag, block_link);

		while (LIST_NEXT(fragment, block_link) != terminating_value) {
			xtrace_memory_fragment_t dying_fragment = LIST_NEXT(fragment, block_link);

			xtrace_malloc_debug("absorbing fragment of %llu bytes at %p", xtrace_memory_fragment_size(dying_fragment), dying_fragment->memory);

			// remove the dying fragment from the free list
			LIST_REMOVE(dying_fragment, free_link);

			// remove the dying fragment from the block list
			LIST_REMOVE(dying_fragment, block_link);

			// expand the surviving fragment
			xtrace_memory_fragment_set_size(fragment, xtrace_memory_fragment_size(fragment) + sizeof(struct xtrace_memory_fragment) + xtrace_memory_fragment_size(dying_fragment));

			// `dying_fragment` is now gone
		}

		xtrace_malloc_debug("expanded fragment is %llu bytes at %p; trying to shrink...", xtrace_memory_fragment_size(fragment), fragment->memory);

		// try to shrink it so that we leave space for other possible fragments
		shrink_fragment(fragment, new_size);

		xtrace_malloc_debug("final fragment is %llu bytes at %p", xtrace_memory_fragment_size(fragment), fragment->memory);

		return true;
	}

	xtrace_malloc_debug("ineligible for expansion");

	return false;
};

//
// api functions
//

void* xtrace_malloc(size_t size) {
	if (size == 0) {
		return NULL;
	}
	xtrace_memory_fragment_t fragment = allocate_fragment(size);
	if (fragment == NULL) {
		return NULL;
	}
	return fragment->memory;
};

void xtrace_free(void* pointer) {
	if (pointer == NULL) {
		return;
	}
	xtrace_memory_fragment_t fragment = __container_of(pointer, struct xtrace_memory_fragment, memory);
	release_fragment(fragment);
};

void* xtrace_realloc(void* old_pointer, size_t new_size) {
	xtrace_memory_fragment_t fragment = __container_of(old_pointer, struct xtrace_memory_fragment, memory);
	size_t old_size = xtrace_memory_fragment_size(fragment);

	xtrace_lock_lock(&free_lock);

	// if we're shrinking, we can always do that
	if (new_size < old_size) {
		// we don't really care if it succeeds or not
		shrink_fragment(fragment, new_size);
		goto out;
	} else if (new_size == old_size) {
		// likewise, we can always keep it the same
		goto out;
	}

	// but otherwise, we're expanding

	// try to see if we can expand the fragment first
	if (expand_fragment(fragment, new_size)) {
		// if we expanded it successfully, we're done!
		goto out;
	} else {
		// otherwise, we need to allocate a new fragment
		xtrace_lock_unlock(&free_lock); // `allocate_fragment` takes the lock, so we need to drop it
		xtrace_memory_fragment_t new_fragment = allocate_fragment(new_size);
		if (new_fragment == NULL) {
			return NULL;
		}

		// copy the old contents to the new fragment
		memcpy(new_fragment->memory, fragment->memory, old_size);

		// and release the old fragment
		release_fragment(fragment);

		// assign the new pointer to the return variable and we're done!
		old_pointer = new_fragment->memory;
		goto out_unlocked;
	}

out:
	xtrace_lock_unlock(&free_lock);

out_unlocked:
	return old_pointer;
};
