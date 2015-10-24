#ifndef __ASL_CORE_H__
#define __ASL_CORE_H__

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
 * @APPLE_LICENSE_HEADER_END@
 */

#include <stdlib.h>
#include <stdint.h>
#include <Availability.h>

typedef struct
{
	uint32_t encoding;
	size_t delta;
	size_t bufsize;
	size_t cursor;
	char *buf;
} asl_string_t;

#define ASL_STATUS_OK               0
#define ASL_STATUS_INVALID_ARG      1
#define ASL_STATUS_INVALID_STORE    2
#define ASL_STATUS_INVALID_STRING   3
#define ASL_STATUS_INVALID_ID       4
#define ASL_STATUS_INVALID_MESSAGE  5
#define ASL_STATUS_NOT_FOUND        6
#define ASL_STATUS_READ_FAILED      7
#define ASL_STATUS_WRITE_FAILED     8
#define ASL_STATUS_NO_MEMORY        9
#define ASL_STATUS_ACCESS_DENIED   10
#define ASL_STATUS_READ_ONLY       11
#define ASL_STATUS_WRITE_ONLY      12
#define ASL_STATUS_MATCH_FAILED    13
#define ASL_STATUS_NO_RECORDS      14
#define ASL_STATUS_FAILED        9999

#define ASL_REF_NULL 0xffffffffffffffffLL

#define ASL_MSG_FLAG_READ_UID_SET 0x0001
#define ASL_MSG_FLAG_READ_GID_SET 0x0002
#define ASL_MSG_FLAG_SEARCH_MATCH 0x8000
#define ASL_MSG_FLAG_SEARCH_CLEAR 0x7fff

#define ASL_QUERY_MATCH_SLOW     0x00000000
#define ASL_QUERY_MATCH_MSG_ID   0x00000001
#define ASL_QUERY_MATCH_TIME     0x00000002
#define ASL_QUERY_MATCH_NANO     0x00000004
#define ASL_QUERY_MATCH_LEVEL    0x00000008
#define ASL_QUERY_MATCH_PID      0x00000010
#define ASL_QUERY_MATCH_UID      0x00000020
#define ASL_QUERY_MATCH_GID      0x00000040
#define ASL_QUERY_MATCH_RUID     0x00000080
#define ASL_QUERY_MATCH_RGID     0x00000100
#define ASL_QUERY_MATCH_REF_PID  0x00000200
#define ASL_QUERY_MATCH_HOST     0x00000400
#define ASL_QUERY_MATCH_SENDER   0x00000800
#define ASL_QUERY_MATCH_FACILITY 0x00001000
#define ASL_QUERY_MATCH_MESSAGE  0x00002000
#define ASL_QUERY_MATCH_REF_PROC 0x00004000
#define ASL_QUERY_MATCH_SESSION  0x00008000
#define ASL_QUERY_MATCH_TRUE     0x80000000
#define ASL_QUERY_MATCH_FALSE    0x40000000
#define ASL_QUERY_MATCH_ERROR    0x20000000

#define ASL_ENCODE_NONE		0
#define ASL_ENCODE_SAFE		1
#define ASL_ENCODE_ASL		2
#define ASL_ENCODE_XML		3
#define ASL_ENCODE_MASK		0x0000000f
#define ASL_STRING_VM		0x80000000
#define ASL_STRING_LEN		0x40000000

#define ASL_STRING_MIG		0xc0000002

uint32_t asl_core_string_hash(const char *str, uint32_t len) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
const char *asl_core_error(uint32_t code) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_core_check_access(int32_t msgu, int32_t msgg, int32_t readu, int32_t readg, uint16_t flags) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint64_t asl_core_htonq(uint64_t n) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint64_t asl_core_ntohq(uint64_t n)__OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint64_t asl_core_new_msg_id(uint64_t start) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
char *asl_core_encode_buffer(const char *in, uint32_t len) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_5_0);
int32_t asl_core_decode_buffer(const char *in, char **buf, uint32_t *len) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_5_0);

asl_string_t *asl_string_new(uint32_t encoding) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
char *asl_string_free_return_bytes(asl_string_t *str) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
void asl_string_free(asl_string_t *str) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
char *asl_string_bytes(asl_string_t *str) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
size_t asl_string_length(asl_string_t *str) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
size_t asl_string_allocated_size(asl_string_t *str) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
asl_string_t *asl_string_append(asl_string_t *str, const char *app) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
asl_string_t *asl_string_append_asl_key(asl_string_t *str, const char *app) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
asl_string_t *asl_string_append_op(asl_string_t *str, uint32_t op) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
asl_string_t *asl_string_append_no_encoding(asl_string_t *str, const char *app) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
asl_string_t *asl_string_append_char_no_encoding(asl_string_t *str, const char c) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);
asl_string_t *asl_string_append_xml_tag(asl_string_t *str, const char *tag, const char *s) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_5_1);

#endif /* __ASL_CORE_H__ */
