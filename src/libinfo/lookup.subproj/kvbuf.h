/*
 * Copyright (c) 2006-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _KVBUF_H_
#define _KVBUF_H_

#include <stdint.h>
#include <sys/cdefs.h>
#include <malloc/malloc.h>

#define KVBUF_START_SIZE 128

/*
 * kvbuf_t is used to encode requests and replies.
 * It encodes a list of dictionaries.
 * First 4 bytes are number of dictionaries.
 * All numbers and lengths are uint32_t in network byte order.
 * Each dictionary is a list of (key, value list) pairs.
 * First 4 bytes in a dictionary are the number of keys.
 * Key is 4 bytes (length) followed by nul-terminated string.
 * Following the key is a value list.
 * First 4 bytes in a value list are the number of values.
 * Each value is 4 bytes (length) followed by nul-terminated string.
 */
typedef struct
{
	uint32_t datalen;
	char *databuf;
	uint32_t _size;
	uint32_t _dict;
	uint32_t _key;
	uint32_t _vlist;
	uint32_t _val;
} kvbuf_t;

typedef struct
{
	uint32_t kcount;
	const char **key;
	uint32_t *vcount;
	const char ***val;
} kvdict_t;

typedef struct
{
	uint32_t count;
	uint32_t curr;
	kvdict_t *dict;
	kvbuf_t *kv;
} kvarray_t;

__BEGIN_DECLS

/*
 * Utilities for creating KV buffers
 */
kvbuf_t *kvbuf_new(void);
kvbuf_t *kvbuf_new_zone(malloc_zone_t *zone);
kvbuf_t *kvbuf_init(char *buffer, uint32_t length);
kvbuf_t *kvbuf_init_zone(malloc_zone_t *zone, char *buffer, uint32_t length);

void kvbuf_add_dict(kvbuf_t *kv);
void kvbuf_add_key(kvbuf_t *kv, const char *key);
void kvbuf_add_val(kvbuf_t *kv, const char *val);
void kvbuf_add_val_len(kvbuf_t *kv, const char *val, uint32_t len);
uint32_t kvbuf_get_len(const char *p);

void kvbuf_make_purgeable(kvbuf_t *kv);
int kvbuf_make_nonpurgeable(kvbuf_t *kv);

void kvbuf_free(kvbuf_t *kv);

/* 
 * Utilities for getting data back from KV buffers
 * These are ugly, but reasonably efficient.
 * Libinfo routines decode the raw databuf in a single pass
 * i.e. not with these routines.
 */

kvarray_t *kvbuf_decode(kvbuf_t *kv);
void kvarray_free(kvarray_t *a);

/*
 * Utility to append a kvbuf to an existing kvbuf
 */
void kvbuf_append_kvbuf( kvbuf_t *kv, const kvbuf_t *kv2 );

/*
 * Call this to start walking through the kvbuf.
 * Returns the number of dictionaries.
 */
uint32_t kvbuf_reset(kvbuf_t *kv);

/*
 * Walk through dictionaries.
 * Returns the number of keys in the dictionary.
 */
uint32_t kvbuf_next_dict(kvbuf_t *kv);

/*
 * Walk through keys in a dictionary.
 * Returns the key.  Don't free it!
 * Sets the number of values for the key in the val_count output parameter.
 */
char *kvbuf_next_key(kvbuf_t *kv, uint32_t *val_count);

/*
 * Walk through values for a key.
 * Returns the value.  Don't free it!
 */
char *kvbuf_next_val(kvbuf_t *kv);

/*
 * Walk through values for a key, with a length returned
 * Returns the value.  Don't free it!
 */
char *kvbuf_next_val_len(kvbuf_t *kv, uint32_t *vl );

/*
 * kvbuf query support
 */
kvbuf_t *kvbuf_query(char *fmt, ...);
kvbuf_t *kvbuf_query_key_int(const char *key, int32_t i);
kvbuf_t *kvbuf_query_key_uint(const char *key, uint32_t u);
kvbuf_t *kvbuf_query_key_val(const char *key, const char *val);

__END_DECLS

#endif /* ! _KVBUF_H_ */
