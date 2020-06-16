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

#ifndef _NOTIFY_TABLE_H_
#define _NOTIFY_TABLE_H_

#include <os/base.h>
#include <stdint.h>

#define _nc_table(key_t, _ns) \
	struct _nc_table##_ns { \
		uint32_t     count; \
		uint32_t     tombstones; \
		uint32_t     size; \
		uint16_t     grow_shift; \
		uint16_t     key_offset; \
		key_t      **keys; \
	}

typedef _nc_table(char *, ) table_t;
typedef _nc_table(uint32_t, _n) table_n_t;
typedef _nc_table(uint64_t, _64) table_64_t;

__BEGIN_DECLS

extern void _nc_table_init(table_t *t, size_t key_offset);
extern void _nc_table_init_n(table_n_t *t, size_t key_offset);
extern void _nc_table_init_64(table_64_t *t, size_t key_offset);

extern void _nc_table_insert(table_t *t, char **key);
extern void _nc_table_insert_n(table_n_t *t, uint32_t *key);
extern void _nc_table_insert_64(table_64_t *t, uint64_t *key);

extern void *_nc_table_find(table_t *t, const char *key);
extern void *_nc_table_find_n(table_n_t *t, uint32_t key);
extern void *_nc_table_find_64(table_64_t *t, uint64_t key);

extern void _nc_table_delete(table_t *t, const char *key);
extern void _nc_table_delete_n(table_n_t *t, uint32_t key);
extern void _nc_table_delete_64(table_64_t *t, uint64_t key);

extern void _nc_table_foreach(table_t *t, OS_NOESCAPE bool (^)(void *));
extern void _nc_table_foreach_n(table_n_t *t, OS_NOESCAPE bool (^)(void *));
extern void _nc_table_foreach_64(table_64_t *t, OS_NOESCAPE bool (^)(void *));

__END_DECLS

#endif /* _NOTIFY_TABLE_H_ */
