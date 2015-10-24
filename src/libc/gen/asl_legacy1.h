#ifndef __ASL_LEGACY1_H__
#define __ASL_LEGACY1_H__

/*
 * Copyright (c) 2007-2011 Apple Inc.  All rights reserved.
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
 * @APPLE_LICENSE_HEADER_END@ */

/*
 * ASL Database VERSION 1 (LEGACY)
 *
 * Log messages are stored in 80 byte records of the form:
 *
 * | 1    | 4    | 8  | 4    | 4    | 8    | 8    | 8      | 8        | 4     | 4   | 4   | 4   | 8       | 2     | 1    | (80 bytes)
 * | Type | Next | ID | RUID | RGID | Time | Host | Sender | Facility | LEVEL | PID | UID | GID | Message | Flags | Zero |
 *
 * If there are no additional key/value pairs in the message, Next will be zero.  If there are additional 
 * key/value pairs in the database, Next is a record number for a record with the format:
 *
 * | 1    | 4    | 4      | 8    | 8    | 8    | 8    | 8    | 8    | 8    | 8    | 7    | (80 bytes)
 * | Type | Next | Count  | Key1 | Val1 | Key2 | Val2 | Key3 | Val3 | Key4 | Val4 | Zero | 
 *
 * Additional records will be chained using the Next field, with the count field left zero.
 *
 * Strings stored in records of the form:
 *
 * | 1    | 4    | 8  | 4        | 4    | 4      | 55     | (80 bytes)
 * | Type | Next | ID | Refcount | Hash | Length | String |
 * 
 * If the string is longer than 55 bytes, Next is a record number for a record with the format: 
 *
 * | 1    | 4    | 75     | (80 bytes)
 * | Type | Next | String |
 * 
 * The first record (header) in the database has the format:
 *
 * | 12     | 4    | 8      | 56   | (80 bytes)
 * | Cookie | Vers | Max ID | Zero |
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <asl.h>
#include <Availability.h>

typedef struct
{
	uint8_t type;
	uint32_t slot;
	uint64_t xid;
	uint32_t hash;
} asl_legacy1_slot_info_t;

typedef struct
{
	asl_legacy1_slot_info_t *slotlist;
	uint32_t slotlist_count;
	FILE *db;
} asl_legacy1_t;

uint32_t asl_legacy1_open(const char *path, asl_legacy1_t **s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_legacy1_close(asl_legacy1_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_legacy1_fetch(asl_legacy1_t *s, uint64_t msgid, aslmsg *msg) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_legacy1_match(asl_legacy1_t *s, aslresponse query, aslresponse *res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

#endif /*__ASL_LEGACY1_H__*/
