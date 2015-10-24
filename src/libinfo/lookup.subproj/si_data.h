/*
 * Copyright (c) 2008 Apple Inc.  All rights reserved.
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

#ifndef __SI_DATA_H__
#define __SI_DATA_H__

#include <stdint.h>

typedef struct
{
	void *src;
	uint32_t type;
	int32_t refcount;
	uint64_t validation_a;
	uint64_t validation_b;
} si_item_t;

typedef struct
{
	int32_t refcount;
	uint32_t count;
	uint32_t curr;
	si_item_t **entry;
} si_list_t;

/* list construction - these do not retain items */
si_list_t *si_list_add(si_list_t *l, si_item_t *e);
si_list_t *si_list_concat(si_list_t *l, si_list_t *x);

si_list_t *si_list_retain(si_list_t *l);
void si_list_release(si_list_t *l);

si_item_t *si_list_next(si_list_t *list);
void si_list_reset(si_list_t *list);

si_item_t *si_item_retain(si_item_t *item);
void si_item_release(si_item_t *item);

#endif /* ! __SI_DATA_H__ */
