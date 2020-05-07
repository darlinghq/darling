/*
 * Copyright (c) 2015-2016 Apple Inc. All rights reserved.
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

#ifndef log_encode_types_h
#define log_encode_types_h

/*
 * These are IPIs between xnu and libtrace, used to have common encoding
 * and implementation for kernel logging and user logging. They are subject
 * to change at any point.
 */

#include <os/base.h>
#include <os/log.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#pragma mark - buffer support structures, enums

OS_ENUM(os_log_value_type, uint8_t,
    OS_LOG_BUFFER_VALUE_TYPE_SCALAR = 0,
    OS_LOG_BUFFER_VALUE_TYPE_COUNT = 1,
    OS_LOG_BUFFER_VALUE_TYPE_STRING = 2,
#ifndef KERNEL
    OS_LOG_BUFFER_VALUE_TYPE_POINTER = 3,
    OS_LOG_BUFFER_VALUE_TYPE_OBJECT = 4,
#endif
    );

OS_ENUM(os_log_value_subtype, uint8_t,
    OS_LOG_BUFFER_VALUE_SUBTYPE_NONE = 0,
    OS_LOG_BUFFER_VALUE_SUBTYPE_INTEGER = 1,
#ifndef KERNEL
    OS_LOG_BUFFER_VALUE_SUBTYPE_FLOAT = 2,
#endif
    );

enum os_log_int_types_t {
	OST_CHAR      = -2,
	OST_SHORT     = -1,
	OST_INT       =  0,
	OST_LONG      =  1,
	OST_LONGLONG  =  2,
	OST_SIZE      =  3,
	OST_INTMAX    =  4,
	OST_PTRDIFF   =  5,
};

union os_log_format_types_u {
	uint16_t    u16;
	uint32_t    u32;
	uint64_t    u64;
	char        ch;
	short       s;
	int         i;
	void        *p;
	char        *pch;
#ifndef KERNEL
	wchar_t     wch;
	wchar_t     *pwch;
#endif
	size_t      z;
	intmax_t    im;
	ptrdiff_t   pd;
	long        l;
	long long   ll;
#ifndef KERNEL
	double      d;
	float       f;
	long double ld;
#endif
};

typedef struct os_log_format_value_s {
	union os_log_format_types_u type;
	os_log_value_type_t ctype;
	uint16_t size;
} *os_log_format_value_t;

#define OST_FORMAT_MAX_ARGS 48
#ifdef KERNEL
#define OST_FORMAT_MAX_STRING_SIZE 512
#else
#define OST_FORMAT_MAX_STRING_SIZE 1024
#endif

#define OST_FORMAT_NON_STATIC ~0

typedef struct os_log_buffer_value_s {
#define OS_LOG_CONTENT_FLAG_PRIVATE 0x1
	uint8_t flags : 4;
	os_log_value_type_t type : 4;
	uint8_t size;
	uint8_t value[];
} *os_log_buffer_value_t;

typedef struct os_log_buffer_s {
#define OS_LOG_BUFFER_HAS_PRIVATE 0x1
#define OS_LOG_BUFFER_HAS_NON_SCALAR 0x2
	uint8_t flags;
	uint8_t arg_cnt;
	uint8_t content[];
} *os_log_buffer_t;

typedef struct os_log_buffer_context_s {
	os_log_t log;
	os_log_buffer_t buffer;
	uint8_t *pubdata;
	uint8_t *privdata;

	// composed string
	char *comp;
	size_t comp_off;
	size_t comp_sz;

	// sizes and offsets
	uint16_t content_off; // offset into buffer->content
	uint16_t content_sz; // size not including the header
	uint16_t pubdata_off;
	uint16_t pubdata_sz;
	uint16_t privdata_off;
	uint16_t privdata_sz;

	uint8_t arg_idx;

	// if argument content was limited with %.* or %.#

#ifndef KERNEL
	const char *symptom_str;
	const void *symptom_ptr;
	uint16_t symptom_ptr_len;
	char *annotated;
#endif
	int arg_content_sz;
	bool need_size;
	bool shimmed;
} *os_log_buffer_context_t;

typedef struct os_log_arginfo_s {
	uint16_t offset;
	uint16_t length;
} *os_log_arginfo_t;

/* Clients of these interfaces/structures may be expected to provide implementations of the following functions */

#ifndef KERNEL
extern bool
_NSCF2data(const void *obj, char *string_value, size_t string_sz, bool *is_private);
#endif

extern bool
_os_log_string_is_public(const char *str);

#endif /* log_encode_types_h */
