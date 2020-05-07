/*
 * Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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
#ifndef _KXLD_DICT_H_
#define _KXLD_DICT_H_

#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
#else
    #include "kxld_types.h"
#endif

#include "kxld_array.h"

/*******************************************************************************
* This is a dictionary implementation for hash tables with c-string keys.  It
* uses linear probing for collision resolution and supports hints for hash
* table size as well as automatic resizing.  All possible sizes for it are
* prime or 'pseudoprime' - good choices for number of buckets.
* NOTE: NULL is NOT a valid key or value!
*
* The dictionary also provides a basic iterator interface.  The iterator
* supports a basic walk through the dictionary in unsorted order.  If the
* dictionary is changed in any way while an iterator is being used, the
* iterator's behavior is undefined.
*******************************************************************************/

struct kxld_dict;
typedef struct kxld_dict KXLDDict;
typedef struct kxld_dict_iterator KXLDDictIterator;

typedef u_int (*kxld_dict_hash)(const KXLDDict *dict, const void *key);
typedef u_int (*kxld_dict_cmp)(const void *key1, const void *key2);

struct kxld_dict {
	KXLDArray buckets;      // The array of buckets
	KXLDArray resize_buckets; // A helper array for resizing
	kxld_dict_hash hash;    // Hash function
	kxld_dict_cmp cmp;      // Comparison function
	u_int num_entries;      // Num entries in the dictionary
	u_int resize_threshold; // Num entries we must reach to cause a resize
};

struct kxld_dict_iterator {
	u_int idx;
	const KXLDDict *dict;
};

/*******************************************************************************
* Constructors and Destructors
*******************************************************************************/

/* Initializes a new dictionary object.
 * num_entries is a hint to the maximum number of entries that will be inserted
 */
kern_return_t kxld_dict_init(KXLDDict *dict, kxld_dict_hash hash,
    kxld_dict_cmp cmp, u_int num_entries)
__attribute__((nonnull, visibility("hidden")));

/* Initializes a new dictionary iterator */
void kxld_dict_iterator_init(KXLDDictIterator *iter, const KXLDDict *dict)
__attribute__((nonnull, visibility("hidden")));

/* Removes all entries from the dictionary.  The dictionary must be
 * reinitialized before it can be used again.
 */
void kxld_dict_clear(KXLDDict *dict)
__attribute__((nonnull, visibility("hidden")));

/* Destroys a dictionary and all of its entries */
void kxld_dict_deinit(KXLDDict *dict)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Accessors
*******************************************************************************/

/* Returns the number of entries in the dictionary */
u_int kxld_dict_get_num_entries(const KXLDDict *dict)
__attribute__((pure, nonnull, visibility("hidden")));

/* Finds a key-value pair and assigns the value to the 'value' pointer, or NULL
 * when not found.
 */
void * kxld_dict_find(const KXLDDict *dict, const void *key)
__attribute__((pure, nonnull, visibility("hidden")));

/*******************************************************************************
* Modifiers
*******************************************************************************/

/* Inserts a key-value pair, and will overwrite the value for a key if that key
 * is already in the table.
 */
kern_return_t kxld_dict_insert(KXLDDict *dict, const void *key, void *value)
__attribute__((nonnull, visibility("hidden")));

/* Removes a key-value pair and assigns the value to the 'value' pointer.
 * 'value' pointer will be set to NULL if value to be removed is not found.
 * 'value pointer may be NULL if removed value is not needed.
 */
void kxld_dict_remove(KXLDDict *dict, const void *key, void **value)
__attribute__((nonnull(1, 2), visibility("hidden")));

/* Gets the next item in the dictionary */
void kxld_dict_iterator_get_next(KXLDDictIterator *iter, const void **key,
    void **value)
__attribute__((nonnull, visibility("hidden")));

/* Resets the iterator to the first item in the dictionary */
void kxld_dict_iterator_reset(KXLDDictIterator *iter)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Helpers
*******************************************************************************/

u_int kxld_dict_string_hash(const KXLDDict *dict, const void *key)
__attribute__((pure, nonnull, visibility("hidden")));
u_int kxld_dict_uint32_hash(const KXLDDict *dict, const void *key)
__attribute__((pure, nonnull, visibility("hidden")));
u_int kxld_dict_kxldaddr_hash(const KXLDDict *dict, const void *key)
__attribute__((pure, nonnull, visibility("hidden")));

u_int kxld_dict_string_cmp(const void *key1, const void *key2)
__attribute__((pure, visibility("hidden")));
u_int kxld_dict_uint32_cmp(const void *key1, const void *key2)
__attribute__((pure, visibility("hidden")));
u_int kxld_dict_kxldaddr_cmp(const void *key1, const void *key2)
__attribute__((pure, visibility("hidden")));

#endif /* _KXLD_DICT_H_ */
