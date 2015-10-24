/*
 * Copyright (c) 2007-2011 Apple Inc. All rights reserved.
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

#ifndef __ASL_FILE_H__
#define __ASL_FILE_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <asl.h>
#include <Availability.h>

#define DB_HEADER_LEN 80
#define DB_HEADER_COOKIE_OFFSET 0
#define DB_HEADER_VERS_OFFSET 12
#define DB_HEADER_FIRST_OFFSET 16
#define DB_HEADER_TIME_OFFSET 24
#define DB_HEADER_CSIZE_OFFSET 32
#define DB_HEADER_LAST_OFFSET 36

/*
 * Magic Cookie for database files.
 * MAXIMUM 12 CHARS! (DB_HEADER_VERS_OFFSET)
 */
#define ASL_DB_COOKIE "ASL DB"
#define ASL_DB_COOKIE_LEN 6
#define DB_VERSION 2
#define DB_VERSION_LEGACY_1 1

#define ASL_FILE_FLAG_READ_ONLY			0x00000001
#define ASL_FILE_FLAG_UNLIMITED_CACHE	0x00000002
#define ASL_FILE_FLAG_PRESERVE_MSG_ID	0x00000004
#define ASL_FILE_FLAG_LEGACY_STORE		0x00000008

#define ASL_FILE_TYPE_MSG 0
#define ASL_FILE_TYPE_STR 1

#define ASL_FILE_POSITION_FIRST 0
#define ASL_FILE_POSITION_PREVIOUS 1
#define ASL_FILE_POSITION_NEXT 2
#define ASL_FILE_POSITION_LAST 3

/* NB CACHE_SIZE must be > 1 */
#define CACHE_SIZE 256

/* Size of the fixed-length part of a MSG record */
#define MSG_RECORD_FIXED_LENGTH 122

/*
 * The first record (header) in the database has the format:
 *
 * | 12     | 4    | 8      | 8    | 4                 | 8    | 36   | (80 bytes)
 * | Cookie | Vers | First  | Time | String cache size | Last | Zero |
 * 
 * MSG records have the format:
 *
 * | 2  | 4   | 8    | 8  | 8    | 4    | 2     | 2     | 4   | 4   | 4   | 4    | 4    | 4      | 4
 * | 00 | Len | Next | ID | Time | Nano | Level | Flags | PID | UID | GID | RUID | RGID | RefPID | KV count ...
 *
 * | 8    | 8      | 8        | 8       | 8       | 8       | 8    | 8    |     | 8
 * | Host | Sender | Facility | Message | RefProc | Session | Key0 | Val0 | ... | Previous |
 * 
 * STR records have the format:
 *
 * | 2  | 4   | Len      | (Len + 6 bytes)
 * | 01 | Len | Data+NUL | 
 * 
 */

typedef struct file_string_s
{
	uint64_t where;
	uint32_t hash;
	struct file_string_s *next;
	char str[];
} file_string_t;

typedef struct
{
	uint32_t flags;
	uint32_t version;
	uint32_t string_count;
	file_string_t *string_list;
	uint64_t first;
	uint64_t last;
	uint64_t prev;
	uint64_t cursor;
	uint64_t cursor_xid;
	uint64_t dob;
	size_t file_size;
	FILE *store;
	void *legacy;
	char *scratch;
} asl_file_t;

typedef struct asl_file_list_s
{
	asl_file_t *file;
	struct asl_file_list_s *next;
} asl_file_list_t;

asl_file_list_t *asl_file_list_add(asl_file_list_t *list, asl_file_t *f) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
void asl_file_list_close(asl_file_list_t *list) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_file_open_write(const char *path, mode_t mode, uid_t uid, gid_t gid, asl_file_t **s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_file_close(asl_file_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_file_save(asl_file_t *s, aslmsg msg, uint64_t *mid) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_file_open_read(const char *path, asl_file_t **s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_file_fetch(asl_file_t *s, uint64_t mid, aslmsg *msg) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_file_read_set_position(asl_file_t *s, uint32_t pos) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_file_fetch_next(asl_file_t *s, aslmsg *msg) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_file_fetch_previous(asl_file_t *s, aslmsg *msg) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_file_match(asl_file_t *s, aslresponse query, aslresponse *res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_file_list_match_timeout(asl_file_list_t *list, aslresponse query, aslresponse *res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction, uint32_t usec) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);
uint32_t asl_file_list_match(asl_file_list_t *list, aslresponse query, aslresponse *res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

void *asl_file_list_match_start(asl_file_list_t *list, uint64_t start_id, int32_t direction) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_file_list_match_next(void *token, aslresponse query, aslresponse *res, uint32_t count) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
void asl_file_list_match_end(void *token) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

size_t asl_file_size(asl_file_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint64_t asl_file_ctime(asl_file_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_file_compact(asl_file_t *s, const char *path, mode_t mode, uid_t uid, gid_t gid) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

#endif /* __ASL_FILE_H__ */
