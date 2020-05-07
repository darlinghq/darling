/*
 * Copyright (c) 2000-2013 Apple Inc. All rights reserved.
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

/* direct-mapped partial matching compressor with simple 22/10 split
 *
 *  Compresses buffers using a dictionary based match and partial match
 *  (high bits only or full match) scheme.
 *
 *  Paul Wilson -- wilson@cs.utexas.edu
 *  Scott F. Kaplan -- sfkaplan@cs.utexas.edu
 *  September 1997
 */

/* compressed output format, in memory order
 *  1. a four-word HEADER containing four one-word values:
 *     i.   a one-word code saying what algorithm compressed the data
 *     ii.  an integer WORD offset into the page saying
 *          where the queue position area starts
 *     iii. an integer WORD offset into the page saying where
 *          the low-bits area starts
 *     iv.  an integer WORD offset into the page saying where the
 *          low-bits area ends
 *
 *  2. a 64-word TAGS AREA holding one two-bit tag for each word in
 *     the original (1024-word) page, packed 16 per word
 *
 *  3. a variable-sized FULL WORDS AREA (always word aligned and an
 *     integral number of words) holding full-word patterns that
 *     were not in the dictionary when encoded (i.e., dictionary misses)
 *
 *  4. a variable-sized QUEUE POSITIONS AREA (always word aligned and
 *     an integral number of words) holding four-bit queue positions,
 *     packed eight per word.
 *
 *  5. a variable-sized LOW BITS AREA (always word aligned and an
 *     integral number of words) holding ten-bit low-bit patterns
 *     (from partial matches), packed three per word.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/vm_param.h>


#define WKdm_SCRATCH_BUF_SIZE_INTERNAL  PAGE_SIZE

typedef unsigned int WK_word;

#if defined(__arm64__)

void
WKdm_decompress_4k(const WK_word* src_buf,
    WK_word* dest_buf,
    WK_word* scratch,
    unsigned int bytes);
int
WKdm_compress_4k(const WK_word* src_buf,
    WK_word* dest_buf,
    WK_word* scratch,
    unsigned int limit);

void
WKdm_decompress_16k(WK_word* src_buf,
    WK_word* dest_buf,
    WK_word* scratch,
    unsigned int bytes);
int
WKdm_compress_16k(WK_word* src_buf,
    WK_word* dest_buf,
    WK_word* scratch,
    unsigned int limit);
#else

void
WKdm_decompress_new(WK_word* src_buf,
    WK_word* dest_buf,
    WK_word* scratch,
    unsigned int bytes);
int
WKdm_compress_new(const WK_word* src_buf,
    WK_word* dest_buf,
    WK_word* scratch,
    unsigned int limit);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
