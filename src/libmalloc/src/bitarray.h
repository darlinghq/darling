/*
 * Copyright (c) 1999, 2000, 2003, 2005, 2008, 2012 Apple Inc. All rights reserved.
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
//
//  bitarray.h
//  bitarray
//
//  Created by Bertrand Serlet on 9/26/10.
//  Copyright (c) 2010 Apple. All rights reserved.
//

#include <stdbool.h>
#include <libc.h>

typedef uint64_t *bitarray_t; // array of bits, assumed to be mostly 0
typedef uint32_t index_t; // we limit the number of bits to be a 32-bit quantity

/* A bitarray uses a summarization to be able to quickly say what's the first bit that is set to 1;
 All together each of the entry points will do a very small number of memory access (exact number depends on log_size) */

extern size_t bitarray_size(unsigned log_size);
    // For a bitarray with 1<<log_size bits, returns the number of bytes needed to contain the whole bitarray

extern bitarray_t bitarray_create(unsigned log_size);
    // creates a bitarray with 1<<log_size bits, all initialized to 0.  
    // Use free() to free bitarray

extern bool bitarray_get(bitarray_t bits, unsigned log_size, index_t index);

extern bool bitarray_set(bitarray_t bits, unsigned log_size, index_t index);
    // Set a bit in bitarray

extern bool bitarray_zap(bitarray_t bits, unsigned log_size, index_t index);
    // Clears a bit in bitarray

extern index_t bitarray_first_set(const bitarray_t bits, unsigned log_size);
    // Returns the index first bit that's 1, plus 1, or 0 if all the bits are zero

extern bool bitarray_zap_first_set(bitarray_t bits, unsigned log_size, index_t *index);
    // finds the first bit set, and if found, zaps it and sets index

extern unsigned bitarray_zap_first_set_multiple(bitarray_t bits, unsigned log_size, unsigned max, index_t *indices);
    // finds all the bits set, up to max, and zaps each and sets the index for each
    // returns number zapped

