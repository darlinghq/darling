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

#ifndef __ASL_STORE_H__
#define __ASL_STORE_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <asl.h>
#include "asl_file.h"
#include <Availability.h>

#define PATH_ASL_STORE "/var/log/asl"
#define PATH_ASL_ARCHIVE "/var/log/asl.archive"
#define FILE_ASL_STORE_DATA "StoreData"
#define FILE_ASL_STORE_SWEEP_SEMAPHORE "SweepStore"

#define FILE_CACHE_SIZE 64
#define FILE_CACHE_TTL 300

typedef struct
{
	time_t ts;
	uid_t u;
	gid_t g;
	time_t bb;
	char *path;
	asl_file_t *f;
} asl_cached_file_t;

typedef struct
{
	char *base_dir;
	FILE *storedata;
	uint64_t next_id;
	asl_cached_file_t file_cache[FILE_CACHE_SIZE];
	void *work;
	time_t start_today;
	time_t start_tomorrow;
	time_t last_write;
	size_t max_file_size;
} asl_store_t;

uint32_t asl_store_open_write(const char *basedir, asl_store_t **s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_open_read(const char *basedir, asl_store_t **s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_close(asl_store_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_statistics(asl_store_t *s, aslmsg *msg) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_store_save(asl_store_t *s, aslmsg msg) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_store_match(asl_store_t *s, aslresponse query, aslresponse *res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_match_timeout(asl_store_t *s, aslresponse query, aslresponse *res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction, uint32_t usec) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);

uint32_t asl_store_match_start(asl_store_t *s, uint64_t start_id, int32_t direction) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_match_next(asl_store_t *s, aslresponse query, aslresponse *res, uint32_t count) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_store_max_file_size(asl_store_t *s, size_t max) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_signal_sweep(asl_store_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_sweep_file_cache(asl_store_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);

uint32_t asl_store_open_aux(asl_store_t *s, aslmsg msg, int *fd, char **url) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);

#endif /* __ASL_STORE_H__ */
