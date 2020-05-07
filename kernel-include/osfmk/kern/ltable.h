/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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
#ifdef XNU_KERNEL_PRIVATE

#include <kern/kern_types.h>
#include <machine/locks.h>

#if CONFIG_LTABLE_DEBUG
#define ltdbg(fmt, ...) \
	printf("LT[%s]:  " fmt "\n", __func__, ## __VA_ARGS__)
#else
#define ltdbg(fmt, ...) do { } while (0)
#endif

#ifdef LTABLE_VERBOSE_DEBUG
#define ltdbg_v(fmt, ...) \
	printf("LT[v:%s]:  " fmt "\n", __func__, ## __VA_ARGS__)
#else
#define ltdbg_v(fmt, ...) do { } while (0)
#endif

#define ltinfo(fmt, ...) \
	printf("LT[%s]: " fmt "\n", __func__,  ## __VA_ARGS__)

#define lterr(fmt, ...) \
	printf("LT[%s] ERROR: " fmt "\n", __func__, ## __VA_ARGS__)



/* ----------------------------------------------------------------------
 *
 * Lockless Link Table Interface
 *
 * ---------------------------------------------------------------------- */

struct ltable_id {
	union {
		uint64_t id;
		struct {
			/*
			 * this bitfield is OK because we don't need to
			 * enforce a particular memory layout
			 */
			uint64_t idx:18, /* allows indexing up to 8MB of 32byte objects */
			    generation:46;
		};
	};
};

/* this _must_ match the idx bitfield definition in struct ltable_id */
#define LT_IDX_MAX  (0x3ffff)

extern vm_size_t     g_lt_max_tbl_size;


struct lt_elem {
	struct ltable_id lt_id;
	uint32_t lt_bits;
	uint32_t lt_next_idx;
};

/* reference count bits should _always_ be the low-order bits */
#define LT_BITS_REFCNT_MASK  (0x1FFFFFFFU)
#define LT_BITS_REFCNT_SHIFT (0)
#define LT_BITS_REFCNT       (LT_BITS_REFCNT_MASK << LT_BITS_REFCNT_SHIFT)

#define LT_BITS_TYPE_MASK    (0x3U)
#define LT_BITS_TYPE_SHIFT   (29)
#define LT_BITS_TYPE         (LT_BITS_TYPE_MASK << LT_BITS_TYPE_SHIFT)

#define LT_BITS_VALID_MASK   (0x1U)
#define LT_BITS_VALID_SHIFT  (31)
#define LT_BITS_VALID        (LT_BITS_VALID_MASK << LT_BITS_VALID_SHIFT)

#define lt_bits_refcnt(bits) \
	(((bits) >> LT_BITS_REFCNT_SHIFT) & LT_BITS_REFCNT_MASK)

#define lt_bits_type(bits) \
	(((bits) >> LT_BITS_TYPE_SHIFT) & LT_BITS_TYPE_MASK)

#define lt_bits_valid(bits) \
	((bits) & LT_BITS_VALID)

enum lt_elem_type {
	LT_FREE     = 0,
	LT_ELEM     = 1,
	LT_LINK     = 2,
	LT_RESERVED = 3,
};

struct link_table;
typedef void (*ltable_poison_func)(struct link_table *, struct lt_elem *);

/*
 * link_table structure
 *
 * A link table is a container for slabs of elements. Each slab is 'slab_sz'
 * bytes and contains 'slab_sz/elem_sz' elements (of 'elem_sz' bytes each).
 * These slabs allow the table to be broken up into potentially dis-contiguous
 * VA space. On 32-bit platforms with large amounts of physical RAM, this is
 * quite important. Keeping slabs like this slightly complicates retrieval of
 * table elements, but not by much.
 */
struct link_table {
	struct lt_elem **table;   /* an array of 'slabs' of elements */
	struct lt_elem **next_free_slab;
	struct ltable_id free_list __attribute__((aligned(8)));

	uint32_t         elem_sz;  /* size of a table element (bytes) */
	uint32_t         slab_shift;
	uint32_t         slab_msk;
	uint32_t         slab_elem;
	uint32_t         slab_sz;  /* size of a table 'slab' object (bytes) */

	uint32_t         nelem;
	uint32_t         used_elem;
	zone_t           slab_zone;

	ltable_poison_func poison;

	lck_mtx_t        lock;
	uint32_t         state;

#if CONFIG_LTABLE_STATS
	uint32_t         nslabs;

	uint64_t         nallocs;
	uint64_t         nreallocs;
	uint64_t         npreposts;
	int64_t          nreservations;
	uint64_t         nreserved_releases;
	uint64_t         nspins;

	uint64_t         max_used;
	uint64_t         avg_used;
	uint64_t         max_reservations;
	uint64_t         avg_reservations;
#endif
} __attribute__((aligned(8)));


/**
 * ltable_bootstrap: bootstrap a link table
 *
 * Called once at system boot
 */
extern void ltable_bootstrap(void);


/**
 * ltable_init: initialize a link table with given parameters
 *
 */
extern void ltable_init(struct link_table *table, const char *name,
    uint32_t max_tbl_elem, uint32_t elem_sz,
    ltable_poison_func poison);


/**
 * ltable_grow: grow a link table by adding another 'slab' of table elements
 *
 * Conditions:
 *	table mutex is unlocked
 *	calling thread can block
 */
extern void ltable_grow(struct link_table *table, uint32_t min_free);


/**
 * ltable_alloc_elem: allocate one or more elements from a given table
 *
 * The returned element(s) will be of type 'type', but will remain invalid.
 *
 * If the caller has disabled preemption, then this function may (rarely) spin
 * waiting either for another thread to either release 'nelem' table elements,
 * or grow the table.
 *
 * If the caller can block, then this function may (rarely) block while
 * the table grows to meet the demand for 'nelem' element(s).
 */
extern __attribute__((noinline))
struct lt_elem *ltable_alloc_elem(struct link_table *table, int type,
    int nelem, int nattempts);


#if DEVELOPMENT || DEBUG
/**
 * ltable_nelem: returns how many elements are used in this
 * table.
 */
extern
int ltable_nelem(struct link_table *table);
#endif

/**
 * ltable_realloc_elem: convert a reserved element to a particular type
 *
 * This funciton is used to convert reserved elements (not yet marked valid)
 * to the given 'type'. The generation of 'elem' is incremented, the element
 * is disconnected from any list to which it belongs, and its type is set to
 * 'type'.
 */
extern void ltable_realloc_elem(struct link_table *table,
    struct lt_elem *elem, int type);


/**
 * ltable_get_elem: get a reference to a table element identified by 'id'
 *
 * Returns a reference to the table element associated with the given 'id', or
 * NULL if the 'id' was invalid or does not exist in 'table'. The caller is
 * responsible to release the reference using ltable_put_elem().
 *
 * NOTE: if the table element pointed to by 'id' is marked as invalid,
 *       this function will return NULL.
 */
extern struct lt_elem *ltable_get_elem(struct link_table *table, uint64_t id);


/**
 * ltable_put_elem: release a reference to table element
 *
 * This function releases a reference taken on a table element via
 * ltable_get_elem(). This function will release the element back to 'table'
 * when the reference count goes to 0 AND the element has been marked as
 * invalid.
 */
extern void ltable_put_elem(struct link_table *table, struct lt_elem *elem);


/**
 * lt_elem_invalidate: mark 'elem' as invalid
 *
 * NOTE: this does _not_ get or put a reference on 'elem'
 */
extern void lt_elem_invalidate(struct lt_elem *elem);


/**
 * lt_elem_mkvalid: mark 'elem' as valid
 *
 * NOTE: this does _not_ get or put a reference on 'elem'
 */
extern void lt_elem_mkvalid(struct lt_elem *elem);


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 * API: lt_elem_list_*
 *
 * Reuse the free list linkage member, 'lt_next_idx' of a link table element
 * in a slightly more generic singly-linked list. All members of this list
 * have been allocated from a table, but have not been made valid.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/**
 * lt_elem_list_link: link a child onto a parent
 *
 * Note that if 'parent' is the head of a list, this function will follow that
 * list and attach 'child' to the end of it. In the simplest case, this
 * results in: parent->child
 * however this could also result in: parent->...->child
 */
extern int lt_elem_list_link(struct link_table *table,
    struct lt_elem *parent, struct lt_elem *child);


/**
 * lt_elem_list_first: obtain a pointer to the first element of a list.
 *
 * This function converts the head of a singly-linked list, 'id', into a real
 * lt_elem object and returns a pointer to the object.
 *
 * It does _not_ take an extra reference on the object: the list implicitly
 * holds that reference.
 */
extern struct lt_elem *lt_elem_list_first(struct link_table *table, uint64_t id);


/**
 * lt_elem_list_next: return the item subsequent to 'elem' in a list
 *
 * Note that this will return NULL if 'elem' is actually the end of the list.
 */
extern struct lt_elem *lt_elem_list_next(struct link_table *table,
    struct lt_elem *elem);


/**
 * lt_elem_list_break: break a list in two around 'elem'
 *
 * This function will reset the next_idx field of 'elem' (making it the end of
 * the list), and return the element subsequent to 'elem' in the list
 * (which could be NULL)
 */
extern struct lt_elem *lt_elem_list_break(struct link_table *table,
    struct lt_elem *elem);


/**
 * lt_elem_list_pop: pop an item off the head of a list
 *
 * The list head is pointed to by '*id', the element corresponding to '*id' is
 * returned by this function, and the new list head is returned in the in/out
 * parameter, '*id'.  The caller is responsible for the reference on the
 * returned object.  A realloc is done to reset the type of the object, but it
 * is still left invalid.
 */
extern struct lt_elem *lt_elem_list_pop(struct link_table *table,
    uint64_t *id, int type);


/**
 * lt_elem_list_release: free an entire list of reserved elements
 *
 * All elements in the list whose first member is 'head' will be released back
 * to 'table' as free elements. The 'type' parameter is used in development
 * kernels to assert that all elements on the list are of the given type.
 */
extern int lt_elem_list_release(struct link_table *table,
    struct lt_elem *head,
    int __assert_only type);

static inline int
lt_elem_list_release_id(struct link_table *table,
    uint64_t id, int type)
{
	return lt_elem_list_release(table, lt_elem_list_first(table, id), type);
}

#endif /* XNU_KERNEL_PRIVATE */
