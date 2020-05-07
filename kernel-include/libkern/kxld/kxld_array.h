/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#ifndef _KXLD_ARRAY_H_
#define _KXLD_ARRAY_H_

#include <sys/queue.h>
#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
#else
    #include "kxld_types.h"
#endif

/*******************************************************************************
* This is a resizeable array implementation designed primarily to maximize
* memory reuse.  The array should only be allocated once, but it can be
* initialized many times.  It persists its memory across initializations, and
* reallocates only if it needs to grow the internal array, such that memory
* allocation churn is eliminated.  Growth is accomodated by building a linked
* list of identically sized arrays.  These arrays can be consolidated into
* one large array in the init function.
*
* A technique commonly used in kxld is to make an array of objects that
* themselves contain kxld_arrays.  To minimize memory churn across links, only
* the individual objects contained in an array should be cleared at the end of
* each link, such that they are in a state ready for reinitialization with the
* memory they have already allocated.  The array that contains them should not
* be cleared.  After all links are complete, to ensure that all memory is
* properly freed, one should call kxld_array_get_slot to walk the entire
* allocated space of the array and clean up all potential instances contained
* therein.  Since this technique is somewhat fragile, there are certain
* requirements that must be met, and guarantees that the array implementation
* provides.
*
* Requirements:
*   - A newly allocated, uninitialized array object must be zeroed out before
*     it is initialized
*   - The objects stored in the array that will be reused must consider
*     being bzeroed a valid initial state.  Specifially, they must check that
*     pointers they contain are nonnull before they are freed or followed
*     at both construction and destruction time.
*
* Guarantees:
*   - The init function will always bzero newly allocated memory.  If memory
*     is added by resizing, it will bzero only the newly allocated portion.
*   - clear, deinit, and copy are the only functions that will change the
*     contents of initialized memory.
*   - The reset, clear, deinit functions will accept a NULL pointer to an array.
*******************************************************************************/

STAILQ_HEAD(kxld_array_head, kxld_array_pool);

struct kxld_array {
	struct kxld_array_head pools;
	size_t itemsize;        /* The size of the items that the array contains */
	size_t pool_capacity;   /* The size of each pool's internal buffer */
	u_int pool_maxitems;    /* The maximum number of items each pool can hold
	                         * given the current size of each pool's buffer.
	                         */
	u_int nitems;           /* The current number of items this array contains */
	u_int maxitems;         /* The maximum number of items this array can contain */
	u_int npools;           /* The number of pools in the pool list */
};

struct kxld_array_pool {
	STAILQ_ENTRY(kxld_array_pool) entries;
	u_char *buffer;         /* The internal memory buffer */
	u_int nitems;           /* The number of items the array contains */
};

typedef struct kxld_array KXLDArray;
typedef struct kxld_array_head KXLDArrayHead;
typedef struct kxld_array_pool KXLDArrayPool;

/*******************************************************************************
* Constructors and Destructors
*******************************************************************************/

/* Initializes the array's capacity to a minimum of nitems * itemsize */
kern_return_t kxld_array_init(KXLDArray *array, size_t itemsize, u_int nitems)
__attribute__((nonnull, visibility("hidden")));

/* Performs a deep copy of the array */
kern_return_t kxld_array_copy(KXLDArray *array, const KXLDArray *src)
__attribute__((nonnull, visibility("hidden")));

/* Sets the number of items in the array to 0 */
void kxld_array_reset(KXLDArray *array)
__attribute__((visibility("hidden")));

/* Zeroes out the array and sets nitems to 0 */
void kxld_array_clear(KXLDArray *array)
__attribute__((visibility("hidden")));

/* Frees the array's internal buffer */
void kxld_array_deinit(KXLDArray *array)
__attribute__((visibility("hidden")));

/*******************************************************************************
* Accessors
*******************************************************************************/

/* Returns the item at the specified index, or NULL if idx > nitems */
void *kxld_array_get_item(const KXLDArray *array, u_int idx)
__attribute__((pure, nonnull, visibility("hidden")));

/* Returns the item at the specified index, or NULL if idx > maxitems */
void *kxld_array_get_slot(const KXLDArray *array, u_int idx)
__attribute__((pure, nonnull, visibility("hidden")));

/* Returns the index of a specified item in the array */
kern_return_t kxld_array_get_index(const KXLDArray *array, const void *item,
    u_int *idx)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Modifiers
*******************************************************************************/

/* Grows the array to contain a minimum of nitems.  If extra memory is needed,
 * it will allocate a pool and add it to the list of pools maintained by this
 * array.
 */
kern_return_t kxld_array_resize(KXLDArray *array, u_int nitems)
__attribute__((nonnull, visibility("hidden")));

/* Removes an element from the array.  This is only supported for arrays with
 * a single pool.
 */
kern_return_t kxld_array_remove(KXLDArray *array, u_int idx)
__attribute__((nonnull, visibility("hidden")));

#endif /* _KXLD_ARRAY_H_ */
