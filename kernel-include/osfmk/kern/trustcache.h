/*
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
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

#ifndef _KERN_TRUSTCACHE_H_
#define _KERN_TRUSTCACHE_H_

#include <stdint.h>

#include <kern/cs_blobs.h>

#include <uuid/uuid.h>

#ifdef PLATFORM_BridgeOS
/* Version 0 trust caches: No defined sorting order (thus only suitable for small trust caches).
 * Used for loadable trust caches only, until phasing out support. */
typedef uint8_t trust_cache_hash0[CS_CDHASH_LEN];
struct trust_cache_module0 {
	uint32_t version;
	uuid_t uuid;
	uint32_t num_hashes;
	trust_cache_hash0 hashes[];
} __attribute__((__packed__));
#endif


/* Version 1 trust caches: Always sorted by cdhash, added hash type and flags field.
 * Suitable for all trust caches. */

struct trust_cache_entry1 {
	uint8_t cdhash[CS_CDHASH_LEN];
	uint8_t hash_type;
	uint8_t flags;
} __attribute__((__packed__));

struct trust_cache_module1 {
	uint32_t version;
	uuid_t uuid;
	uint32_t num_entries;
	struct trust_cache_entry1 entries[];
} __attribute__((__packed__));

// Trust Cache Entry Flags
#define CS_TRUST_CACHE_AMFID    0x1                     // valid cdhash for amfid

/* Trust Cache lookup functions return their result as a 32bit value
 * comprised of subfields, for straightforward passing through layers.
 *
 * Format:
 *
 * 0xXXCCBBAA
 *
 * AA:  0-7: lookup result
 *  bit  0: TC_LOOKUP_FOUND: set if any entry found
 *  bit  1: (obsolete) TC_LOOKUP_FALLBACK: set if found in legacy static trust cache
 *  bit  2-7: reserved
 * BB:  8-15: entry flags pass-through, see "Trust Cache Entry Flags" above
 * CC: 16-23: code directory hash type of entry, see CS_HASHTYPE_* in cs_blobs.h
 * XX: 24-31: reserved
 */

#define TC_LOOKUP_HASH_TYPE_SHIFT               16
#define TC_LOOKUP_HASH_TYPE_MASK                0xff0000L;
#define TC_LOOKUP_FLAGS_SHIFT                   8
#define TC_LOOKUP_FLAGS_MASK                    0xff00L
#define TC_LOOKUP_RESULT_SHIFT                  0
#define TC_LOOKUP_RESULT_MASK                   0xffL

#define TC_LOOKUP_FOUND         1

#ifdef XNU_KERNEL_PRIVATE

// Serialized Trust Caches

/* This is how iBoot delivers them to us. */
struct serialized_trust_caches {
	uint32_t num_caches;
	uint32_t offsets[0];
} __attribute__((__packed__));


void trust_cache_init(void);

uint32_t lookup_in_static_trust_cache(const uint8_t cdhash[CS_CDHASH_LEN]);

bool lookup_in_trust_cache_module(struct trust_cache_module1 const * const module,
    uint8_t const cdhash[CS_CDHASH_LEN],
    uint8_t       * const hash_type,
    uint8_t       * const flags);

#endif

#endif /* _KERN_TRUSTCACHE_H */
