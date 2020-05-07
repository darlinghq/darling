/*
 * Copyright (c) 2016-2016 Apple Inc. All rights reserved.
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

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <kern/assert.h>
#include <machine/limits.h>
#include "lz4_assembly_select.h"
#include "lz4_constants.h"

#define memcpy __builtin_memcpy

#pragma mark - Building blocks

//  Represents a position in the input stream
typedef struct { uint32_t offset; uint32_t word; } lz4_hash_entry_t;
static const size_t lz4_hash_table_size = LZ4_COMPRESS_HASH_ENTRIES * sizeof(lz4_hash_entry_t);

// Worker function for lz4 encode.  Underlies both the buffer and stream encode operations.
// Performs lz4 encoding of up to 2gb of data, updates dst_ptr and src_ptr to point to the
// first byte of output and input that couldn't be completely processed, respectively.
//
// If skip_final_literals is 0, the entire src buffer is encoded, by emitting a final sequence of literals
// at the end of the compressed payload.
//
// If skip_final_literals is not 0, this final literal sequence is not emitted, and the src buffer is
// partially encoded (the length of this literal sequence varies).
extern void lz4_encode_2gb(uint8_t **dst_ptr, size_t dst_size,
    const uint8_t **src_ptr, const uint8_t *src_begin, size_t src_size,
    lz4_hash_entry_t hash_table[LZ4_COMPRESS_HASH_ENTRIES], int skip_final_literals);

extern int lz4_decode(uint8_t **dst_ptr, uint8_t *dst_begin, uint8_t *dst_end,
    const uint8_t **src_ptr, const uint8_t *src_end);

#if LZ4_ENABLE_ASSEMBLY_DECODE
extern int lz4_decode_asm(uint8_t **dst_ptr, uint8_t *dst_begin, uint8_t *dst_end,
    const uint8_t **src_ptr, const uint8_t *src_end);
#endif

#pragma mark - Buffer interfaces

static const size_t lz4_encode_scratch_size = lz4_hash_table_size;
static const size_t lz4_decode_scratch_size = 0;

#pragma mark - Buffer interfaces (LZ4 RAW)

size_t lz4raw_encode_buffer(uint8_t * __restrict dst_buffer, size_t dst_size,
    const uint8_t * __restrict src_buffer, size_t src_size,
    lz4_hash_entry_t hash_table[LZ4_COMPRESS_HASH_ENTRIES]);

size_t lz4raw_decode_buffer(uint8_t * __restrict dst_buffer, size_t dst_size,
    const uint8_t * __restrict src_buffer, size_t src_size,
    void * __restrict work __attribute__((unused)));

typedef __attribute__((__ext_vector_type__(8))) uint8_t vector_uchar8;
typedef __attribute__((__ext_vector_type__(16))) uint8_t vector_uchar16;
typedef __attribute__((__ext_vector_type__(32))) uint8_t vector_uchar32;
typedef __attribute__((__ext_vector_type__(64))) uint8_t vector_uchar64;
typedef __attribute__((__ext_vector_type__(16), __aligned__(1))) uint8_t packed_uchar16;
typedef __attribute__((__ext_vector_type__(32), __aligned__(1))) uint8_t packed_uchar32;
typedef __attribute__((__ext_vector_type__(64), __aligned__(1))) uint8_t packed_uchar64;

typedef __attribute__((__ext_vector_type__(4))) uint16_t vector_ushort4;
typedef __attribute__((__ext_vector_type__(4), __aligned__(2))) uint16_t packed_ushort4;

typedef __attribute__((__ext_vector_type__(2))) int32_t vector_int2;
typedef __attribute__((__ext_vector_type__(4))) int32_t vector_int4;
typedef __attribute__((__ext_vector_type__(8))) int32_t vector_int8;

typedef __attribute__((__ext_vector_type__(4))) uint32_t vector_uint4;

#define UTIL_FUNCTION static inline __attribute__((__always_inline__)) __attribute__((__overloadable__))

// Load N bytes from unaligned location PTR
UTIL_FUNCTION uint16_t
load2(const void * ptr)
{
	uint16_t data; memcpy(&data, ptr, sizeof data); return data;
}
UTIL_FUNCTION uint32_t
load4(const void * ptr)
{
	uint32_t data; memcpy(&data, ptr, sizeof data); return data;
}
UTIL_FUNCTION uint64_t
load8(const void * ptr)
{
	uint64_t data; memcpy(&data, ptr, sizeof data); return data;
}
UTIL_FUNCTION vector_uchar16
load16(const void * ptr)
{
	return (const vector_uchar16)*(const packed_uchar16 *)ptr;
}
UTIL_FUNCTION vector_uchar32
load32(const void * ptr)
{
	return (const vector_uchar32)*(const packed_uchar32 *)ptr;
}
UTIL_FUNCTION vector_uchar64
load64(const void * ptr)
{
	return (const vector_uchar64)*(const packed_uchar64 *)ptr;
}

// Store N bytes to unaligned location PTR
UTIL_FUNCTION void
store2(void * ptr, uint16_t data)
{
	memcpy(ptr, &data, sizeof data);
}
UTIL_FUNCTION void
store4(void * ptr, uint32_t data)
{
	memcpy(ptr, &data, sizeof data);
}
UTIL_FUNCTION void
store8(void * ptr, uint64_t data)
{
	memcpy(ptr, &data, sizeof data);
}
UTIL_FUNCTION void
store16(void * ptr, vector_uchar16 data)
{
	*(packed_uchar16 *)ptr = (packed_uchar16)data;
}
UTIL_FUNCTION void
store32(void * ptr, vector_uchar32 data)
{
	*(packed_uchar32 *)ptr = (packed_uchar32)data;
}
UTIL_FUNCTION void
store64(void * ptr, vector_uchar64 data)
{
	*(packed_uchar64 *)ptr = (packed_uchar64)data;
}

// Load+Store N bytes from unaligned locations SRC to DST. No overlap allowed.
UTIL_FUNCTION void
copy8(void * dst, const void * src)
{
	store8(dst, load8(src));
}
UTIL_FUNCTION void
copy16(void * dst, const void * src)
{
	*(packed_uchar16 *)dst = *(const packed_uchar16 *)src;
}
UTIL_FUNCTION void
copy32(void * dst, const void * src)
{
	*(packed_uchar32 *)dst = *(const packed_uchar32 *)src;
}

#undef memcpy
