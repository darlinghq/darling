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
//  bitarray.c
//  bitarray
//
//  Created by Bertrand Serlet on 9/26/10.
//  Copyright (c) 2010 Apple. All rights reserved.
//

#include "bitarray.h"
#define NDEBUG 1
#include <assert.h>

/******************************** Utilities ***************************/

#define STATIC_INLINE static __inline

STATIC_INLINE unsigned __ffsll(uint64_t xx) {
#if defined(__LP64__)
	return __builtin_ffsl(xx);
#else
	return __builtin_ffsll(xx);
#endif
}

#define BIT_SET(old,bit)    ((old) | (1ULL << (bit)))
#define BIT_GET(old,bit)    ((old) & (1ULL << (bit)))
#define BIT_ZAP(old,bit)    ((old) & ~ (1ULL << (bit)))

// several variants below of bit setting or zapping to generate minimal code
// All these do 1 memory read and (maybe) 1 memory write
STATIC_INLINE bool word_get_bit_simple(uint64_t *word, unsigned bit) {
	uint64_t    old = *word;
	return BIT_GET(old, bit) != 0;
}

STATIC_INLINE void word_set_bit_simple(uint64_t *word, unsigned bit) {
	uint64_t    old = *word;
	*word = BIT_SET(old, bit);
}

STATIC_INLINE bool word_set_bit_changed(uint64_t *word, unsigned bit) {
	// returns 1 iff word has changed
	uint64_t    old = *word;
	uint64_t    new = BIT_SET(old, bit);
	if (old == new) return 0;
	*word = new;
	return 1;
}

STATIC_INLINE bool word_set_bit_changed_go_down(uint64_t *word, unsigned bit, bool *was_non_zero) {
	// returns 1 iff word changed
	// sets was_non_zero (when something changed)
	uint64_t    old = *word;
	uint64_t    new = BIT_SET(old, bit);
	if (old == new) return 0;
	*word = new;
	*was_non_zero = old != 0;
	return 1;
}

STATIC_INLINE bool word_set_bit_go_down(uint64_t *word, unsigned bit) {
	// returns 1 iff level below should be set too
	uint64_t    old = *word;
	uint64_t    new = BIT_SET(old, bit);
	if (old == new) return 0;
	*word = new;
	return !old;
}

STATIC_INLINE void word_zap_bit_simple(uint64_t *word, unsigned bit) {
	uint64_t    old = *word;
	*word = BIT_ZAP(old, bit);
}

STATIC_INLINE bool word_zap_bit_changed(uint64_t *word, unsigned bit) {
	// returns 1 iff word changed
	uint64_t    old = *word;
	uint64_t    new = BIT_ZAP(old, bit);
	if (old == new) return 0;
	*word = new;
	return 1;
}

STATIC_INLINE bool word_zap_bit_changed_go_down(uint64_t *word, unsigned bit, bool *is_now_zero) {
	// returns 1 iff word changed
	// sets is_now_zero (when something changed)
	uint64_t    old = *word;
	uint64_t    new = BIT_ZAP(old, bit);
	if (old == new) return 0;
	*word = new;
	*is_now_zero = ! new;
	return 1;
}

STATIC_INLINE bool word_zap_bit_go_down(uint64_t *word, unsigned bit) {
	// returns 1 iff level below might require a bit-zeroing
	uint64_t    old = *word;
	uint64_t    new = BIT_ZAP(old, bit);
	if (old == new) return 0;
	*word = new;
	return !new;
}

/******************************** Helpers ***************************/

#define NB      9       // number of bits we process at once
// must be at least 6 (64-bit) and 9 seems the best on x86
#define MASKNB    ((1 << NB) - 1) // to just keep these bits
#define NUM_64b (1 << (NB - 6)) // number of 64-bit words we process at once

// number of uint64_t of summaries
#define LEVEL0  (NUM_64b)
#define LEVEL1  (LEVEL0 + (1 << NB)*NUM_64b)
#define LEVEL2  (LEVEL1 + (1 << (NB+NB))*NUM_64b)
#define LEVEL3  (LEVEL2 + (1 << (NB+NB+NB))*NUM_64b)

#define MAX_LEVEL   5

static const unsigned levels_num_words[] = {LEVEL0, LEVEL1, LEVEL2, LEVEL3}; // this encodes the number of words reserved for the bitmap summaries at various levels

STATIC_INLINE bool GET_SIMPLE(uint64_t *word, unsigned bit) {
	return word_get_bit_simple(word + (bit >> 6), bit & 63);
}

STATIC_INLINE void SET_SIMPLE(uint64_t *word, unsigned bit) {
	word_set_bit_simple(word + (bit >> 6), bit & 63);
}

STATIC_INLINE bool SET_CHANGED(uint64_t *word, unsigned bit) {
	// returns 1 iff word changed
	return word_set_bit_changed(word + (bit >> 6), bit & 63);
}

STATIC_INLINE bool SET_CHANGED_GO_DOWN(uint64_t *word, unsigned bit, bool *was_non_zero) {
	// returns 1 iff word changed
	// sets was_non_zero (when something changed)
	return word_set_bit_changed_go_down(word + (bit >> 6), bit & 63, was_non_zero);
}

STATIC_INLINE bool SET_GO_DOWN(uint64_t *word, unsigned bit) {
	// returns 1 iff level below should be set too
	return word_set_bit_go_down(word + (bit >> 6), bit & 63);
}

STATIC_INLINE void ZAP_SIMPLE(uint64_t *word, unsigned bit) {
	return word_zap_bit_simple(word + (bit >> 6), bit & 63);
}

STATIC_INLINE bool ZAP_CHANGED(uint64_t *word, unsigned bit) {
	// returns 1 iff word changed
	return word_zap_bit_changed(word + (bit >> 6), bit & 63);
}

STATIC_INLINE bool all_zeros(uint64_t *words) {
	for (unsigned w = 0; w < NUM_64b; w++) {
		if (words[w]) return 0;
	}
	return 1;
}

STATIC_INLINE bool ZAP_CHANGED_GO_DOWN(uint64_t *word, unsigned bit, bool *is_now_zero) {
	// returns 1 iff word changed
	// sets is_now_zero (when something changed)
	bool changed = word_zap_bit_changed_go_down(word + (bit >> 6), bit & 63, is_now_zero);
	if (changed && (NUM_64b != 1)) {
		// One component went entirely zero, now examine all components in the level
		if (!all_zeros(word)) *is_now_zero = 0;
	}
	return changed;
}

STATIC_INLINE bool ZAP_GO_DOWN(uint64_t *word, unsigned bit) {
	// returns 1 iff level below should be changed too
	bool changed = word_zap_bit_go_down(word + (bit >> 6), bit & 63);
	if (changed && (NUM_64b != 1)) {
		// One component went entirely zero, now examine all components in the level
		if (!all_zeros(word)) return 0;
	}
	return changed;
}

STATIC_INLINE unsigned FFS(uint64_t *word) {
	// does NUM_64b memory reads, at most
#if NB==6
	return __ffsll(*word);
#else
	for (unsigned w = 0; w < NUM_64b; w++) {
		unsigned    f = __ffsll(word[w]);
		if (f) return f + (w << 6);
	}
	return 0;
#endif
}

/******************************** Entry Points ***************************/

size_t bitarray_size(unsigned log_size) {
	assert(log_size <= MAX_LEVEL * NB);
	unsigned    num = NUM_64b;
	if (log_size > NB) {
		unsigned    level = (log_size - NB - 1) / NB;
		num = levels_num_words[level] + (1 << (log_size - 6));
	}
	return num * sizeof(uint64_t);
}

bitarray_t bitarray_create(unsigned log_size) {
	return calloc(1, bitarray_size(log_size));
}

bool bitarray_get(bitarray_t bits, unsigned log_size, index_t index) {
	assert(log_size <= MAX_LEVEL * NB);
	assert(index < (1 << log_size));
	if (log_size <= NB) return GET_SIMPLE(bits, index);
	unsigned    level = (log_size - NB - 1) / NB;
	unsigned    bit;
	bit = index & MASKNB; index >>= NB;
	return GET_SIMPLE(bits + levels_num_words[level] + index*NUM_64b, bit);
}

bool bitarray_set(bitarray_t bits, unsigned log_size, index_t index) {
	// returns whether changed
	assert(log_size <= MAX_LEVEL * NB);
	assert(index < (1 << log_size));
	if (log_size <= NB) return SET_CHANGED(bits, index);
	unsigned    level = (log_size - NB - 1) / NB;
	bool    was_non_zero;
	unsigned    bit;
	bit = index & MASKNB; index >>= NB;
	// printf("SET_CHANGED_GO_DOWN(bits + %d, %d,…)\n", levels_num_words[level] + index, bit);
	if (!SET_CHANGED_GO_DOWN(bits + levels_num_words[level] + index*NUM_64b, bit, &was_non_zero)) return 0;
	if (was_non_zero) return 1;
	switch (level) {
		case 3:
			bit = index & MASKNB; index >>= NB;
			if (!SET_GO_DOWN(bits + LEVEL2 + index*NUM_64b, bit)) return 1;
			/* no break */
		case 2:
			bit = index & MASKNB; index >>= NB;
			if (!SET_GO_DOWN(bits + LEVEL1 + index*NUM_64b, bit)) return 1;
			/* no break */
		case 1:
			bit = index & MASKNB; index >>= NB;
			if (!SET_GO_DOWN(bits + LEVEL0 + index*NUM_64b, bit)) return 1;
			/* no break */
		case 0:
			SET_SIMPLE(bits, index & MASKNB);
			return 1;
		default: abort();
	}
}

bool bitarray_zap(bitarray_t bits, unsigned log_size, index_t index) {
	assert(log_size <= MAX_LEVEL * NB);
	assert(index < (1 << log_size));
	if (log_size <= NB) return ZAP_CHANGED(bits, index);
	unsigned    level = (log_size - NB - 1) / NB;
	bool    is_now_zero;
	unsigned    bit;
	bit = index & MASKNB; index >>= NB;
	if (!ZAP_CHANGED_GO_DOWN(bits + levels_num_words[level] + index*NUM_64b, bit, &is_now_zero)) return 0;
	if (!is_now_zero) return 1;
	switch (level) {
		case 3:
			bit = index & MASKNB; index >>= NB;
			if (!ZAP_GO_DOWN(bits + LEVEL2 + index*NUM_64b, bit)) return 1;
			/* no break */
		case 2:
			bit = index & MASKNB; index >>= NB;
			if (!ZAP_GO_DOWN(bits + LEVEL1 + index*NUM_64b, bit)) return 1;
			/* no break */
		case 1:
			bit = index & MASKNB; index >>= NB;
			if (!ZAP_GO_DOWN(bits + LEVEL0 + index*NUM_64b, bit)) return 1;
			/* no break */
		case 0:
			ZAP_SIMPLE(bits, index & MASKNB);
			return 1;
		default: abort();
	}
}

// Note in the following macro that "words" and "base" are variables being written
#define ADJUST_OFFSET_FOR_FFS(words, base, current_level) { \
words += (1 << (NB * current_level)) * NUM_64b; \
base = (base << NB) + FFS(words + base*NUM_64b) - 1; \
}

// Note in the following macro that "words" and "base" are variables being written
#define ADJUST_OFFSET_FOR_FFS_ACROSS_SUMMARIES(words, base, level) {\
switch (level) { \
case 4: \
ADJUST_OFFSET_FOR_FFS(words, base, 0); \
ADJUST_OFFSET_FOR_FFS(words, base, 1); \
ADJUST_OFFSET_FOR_FFS(words, base, 2); \
break; \
case 3: \
ADJUST_OFFSET_FOR_FFS(words, base, 0); \
ADJUST_OFFSET_FOR_FFS(words, base, 1); \
break; \
case 2: \
ADJUST_OFFSET_FOR_FFS(words, base, 0); \
break; \
case 1: \
break; \
default: abort(); \
} \
}

// Note in the following macro that "ix" and "bit" are variables being written
#define ZAP_SUMMARIES(bits, ix, level) {\
unsigned    bit;    \
switch (level) {    \
case 3:         \
bit = ix & MASKNB; ix >>= NB;   \
if (!ZAP_GO_DOWN(bits + LEVEL2 + ix*NUM_64b, bit)) break;   \
case 2: \
bit = ix & MASKNB; ix >>= NB;   \
if (!ZAP_GO_DOWN(bits + LEVEL1 + ix*NUM_64b, bit)) break;   \
case 1: \
bit = ix & MASKNB; ix >>= NB;   \
if (!ZAP_GO_DOWN(bits + LEVEL0 + ix*NUM_64b, bit)) break;   \
case 0:         \
ZAP_SIMPLE(bits, ix & MASKNB); \
break;  \
default: abort(); \
} \
}

index_t bitarray_first_set(const bitarray_t bits, unsigned log_size) {
	// return 0 if none set
	assert(log_size <= MAX_LEVEL * NB);
	uint64_t    *words = bits;
	unsigned    bit = FFS(words);
	if (log_size <= NB) return bit;
	if (!bit) return 0;
	unsigned    level = (log_size - 1) / NB;
	index_t     base = bit - 1; // offset, in number of uin64_t words
	ADJUST_OFFSET_FOR_FFS_ACROSS_SUMMARIES(words, base, level);
	words += (1 << (NB * (level-1))) * NUM_64b;
	base = (base << NB) + FFS(words + base*NUM_64b) - 1;
	return base+1; //+1 because bit N is encoded as N+1
}

bool bitarray_zap_first_set(bitarray_t bits, unsigned log_size, index_t *index) {
	assert(log_size <= MAX_LEVEL * NB);
	uint64_t    *words = bits;
	index_t     ix = FFS(words);
	if (!ix) return 0;
	unsigned    level = (log_size - 1) / NB;
	if (! level) {
		ix--;
		*index = ix;
		ZAP_SIMPLE(bits, ix);
		return 1;
	}
	index_t     base = ix - 1; // offset, in number of uin64_t words
	ADJUST_OFFSET_FOR_FFS_ACROSS_SUMMARIES(words, base, level);
	words += (1 << (NB * (level-1))) * NUM_64b;
	base = (base << NB) + FFS(words + base*NUM_64b) - 1;
	ix = base;
	*index = ix;
	assert(ix < (1 << log_size));
	level--;
	bool    is_now_zero;
	unsigned    bit;
	bit = ix & MASKNB; ix >>= NB;
	if (!ZAP_CHANGED_GO_DOWN(bits + levels_num_words[level] + ix*NUM_64b, bit, &is_now_zero)) return 1;
	if (!is_now_zero) return 1;
	ZAP_SUMMARIES(bits, ix, level);
	return 1;
}

static unsigned FFS_and_zap_word(uint64_t *words, unsigned max, index_t *indices, index_t to_be_added) {
	// returns the number of bits zapped
	unsigned    zapped = 0;
	for (unsigned w = 0; w < NUM_64b; w++) {
		uint64_t    word = words[w];
		if (!word) continue;
		while (1) {
			unsigned    f = __ffsll(word);
			assert(f);
			f--;
			// printf("%d ", f);
			indices[zapped++] = f + (w << 6) + to_be_added;
			word = BIT_ZAP(word, f);
			if (!word) break;
			if (zapped >= max) break;
		}
		words[w] = word;
		// printf("word=%lld \n", word);
		if (zapped >= max) break;
	}
	return zapped;
}

unsigned bitarray_zap_first_set_multiple(bitarray_t bits, unsigned log_size, unsigned max, index_t *indices) {
	assert(log_size <= MAX_LEVEL * NB);
	if (log_size <= NB) return FFS_and_zap_word(bits, max, indices, 0);
	unsigned    zapped = 0;
	unsigned    level = (log_size - 1) / NB;
	while (zapped < max) {
		/* the lines in loop could be written just as:
		 //  if (! bitarray_zap_first_set(bits, log_size, indices + zapped)) break;
		 //  zapped++;
		 but the code is faster because it wont go up and down in the summaries */
		uint64_t    *words = bits;
		index_t     ix = FFS(words);
		if (!ix) return zapped; // if the top level summary is 0, no bit is set
		index_t     base = ix - 1; // offset, in number of uin64_t words
		ADJUST_OFFSET_FOR_FFS_ACROSS_SUMMARIES(words, base, level);
		words += (1 << (NB * (level-1))) * NUM_64b; // the beginning of the non-summarized bitarray
		uint64_t    *word = words + base*NUM_64b; // the first non-zero word
		ix = base;
		// the idea here is that we zap a whole bunch of bits at once
		unsigned    z = FFS_and_zap_word(word, max - zapped, indices + zapped, base << NB);
		assert(z);
		zapped += z;
		if ((zapped < max) /* entire word was zapped */ || all_zeros(word) /* partial zap, a priori */) {
			// adjust summaries to reflect all zeros in the bitarray
			ZAP_SUMMARIES(bits, ix, level-1);
		}
	}
	return zapped;
}

#if 0
/******************************** Test and debug utilities ***************************/

static void print_ones(const uint64_t *bits, unsigned num_big_words) {
	unsigned    base = 0;
	unsigned    num = num_big_words * NUM_64b;
	// printf("In print_ones; num=%d, num_big=%d \n", num, num_big_words);
	while (num--) {
		uint64_t    word = *(bits++);
		if (word) {
			for (unsigned bit = 0; bit < 64; bit++) {
				if (word & (1ULL << bit)) printf("%d ", base + bit);
			}
		}
		base += 64;
	}
}

void bitarray_print(bitarray_t bits, unsigned log_size) {
	assert(log_size <= MAX_LEVEL * NB);
	printf("bitarray %p log_size=%d\n", bits, log_size);
	if (log_size > 4*NB) {
		printf("Level 4: "); print_ones(bits, 1); printf("\n");
		printf("Level 3: "); print_ones(bits + LEVEL0, 1 << NB); printf("\n");
		printf("Level 2: "); print_ones(bits + LEVEL1, 1 << NB); printf("\n");
		printf("Level 1: "); print_ones(bits + LEVEL2, 1 << NB); printf("\n");
		printf("Level 0: "); print_ones(bits + LEVEL3, 1 << (log_size - NB)); printf("\n");
	} else if (log_size > 3*NB) {
		printf("Level 3: "); print_ones(bits, 1); printf("\n");
		printf("Level 2: "); print_ones(bits + LEVEL0, 1 << NB); printf("\n");
		printf("Level 1: "); print_ones(bits + LEVEL1, 1 << NB); printf("\n");
		printf("Level 0: "); print_ones(bits + LEVEL2, 1 << (log_size - NB)); printf("\n");
	} else if (log_size > 2*NB) {
		printf("Level 2: "); print_ones(bits, 1); printf("\n");
		printf("Level 1: "); print_ones(bits + LEVEL0, 1 << NB); printf("\n");
		printf("Level 0: "); print_ones(bits + LEVEL1, 1 << (log_size - NB)); printf("\n");
	} else if (log_size > NB) {
		printf("Level 1: "); print_ones(bits, 1); printf("\n");
		printf("Level 0: "); print_ones(bits + LEVEL0, 1 << (log_size - NB)); printf("\n");
	} else {
		printf("Level 0: "); print_ones(bits, 1); printf("\n");
	}
}

bool compare_to_truth(bitarray_t bits, unsigned nbits, const bool *truth) {
	uint64_t     *start = bits;
	if (nbits > NB) {
		unsigned    level = (nbits - NB - 1) / NB;
		start += levels_num_words[level];
	}
	bool        ok = 1;
	for (unsigned bit = 0; bit < (1 << nbits); bit++) {
		bool    expected = truth[bit];
		uint64_t     word = start[bit >> 6];
		bool    actual = (word >> (bit & 63)) & 1;
		if (actual != expected) {
			printf("*** # for bit %d, expected=%d actual=%d\n", bit, expected, actual);
			ok = 0;
		}
	}
	return ok;
}

unsigned first_set_in_truth(const bool *truth, unsigned log_size) {
	for (unsigned bit = 0; bit < (1 << log_size); bit++) {
		if (truth[bit]) return bit+1;
	}
	return 0;
}

void truth_print(const bool *truth, unsigned log_size) {
	printf("Truth:  ");
	for (unsigned bit = 0; bit < (1 << log_size); bit++) {
		if (truth[bit]) printf("%d ", bit);
	}
	printf("\n");
}
#endif

/* vim: set noet:ts=4:sw=4:cindent: */
