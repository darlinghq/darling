/*
 * Copyright (c) 2003-2011 Apple Inc. All rights reserved.
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

#include <os/base.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/param.h>
#include <os/assumes.h>

#include "table.h"
#include "notify_internal.h"

#define TABLE_TOMBSTONE ((void *)~0)
#define TABLE_MINSHIFT  5
#define TABLE_MINSIZE   (1 << TABLE_MINSHIFT)

OS_ALWAYS_INLINE
static inline uint32_t
table_next(uint32_t i, uint32_t size)
{
    i++;
    return i >= size ? 0 : i;
}

OS_ALWAYS_INLINE
static inline uint32_t
table_prev(uint32_t i, uint32_t size)
{
    return (i ? i : size) - 1;
}

static inline bool
string_equals(const char *a, const char *b)
{
	return a == b || strcmp(a, b) == 0;
}

static uint32_t
string_hash(const char *key)
{
    uint32_t hash = 0;

    for (; *key; key++) {
        hash += (unsigned char)(*key);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

static inline bool
uint32_equals(uint32_t a, uint32_t b)
{
	return a == b;
}

static inline uint32_t
uint32_hash(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

static inline bool
uint64_equals(uint64_t a, uint64_t b)
{
	return a == b;
}

static inline uint32_t
uint64_hash(uint64_t key)
{
    return uint32_hash((uint32_t)key ^ (uint32_t)(key >> 32));
}

#define ns(n)         _nc_table##n
#define key_t         char *
#define ckey_t        const char *
#define key_hash      string_hash
#define key_equals    string_equals
#include "table.in.c"

#define ns(n)         _nc_table##n##_n
#define key_t         uint32_t
#define ckey_t        uint32_t
#define key_hash      uint32_hash
#define key_equals    uint32_equals
#include "table.in.c"

#define ns(n)         _nc_table##n##_64
#define key_t         uint64_t
#define ckey_t        uint64_t
#define key_hash      uint64_hash
#define key_equals    uint64_equals
#include "table.in.c"
