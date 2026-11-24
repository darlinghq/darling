/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DTC_ASTC_INTERNAL_H
#define DTC_ASTC_INTERNAL_H

#include <stdint.h>

/* A value range of the integer sequence encoding: 0..max, stored as an
 * optional trit or quint plus `bits` low bits (spec 23.12). */
typedef struct astc_range {
	uint8_t trit;
	uint8_t quint;
	uint8_t bits;
	uint8_t max;
} astc_range;

#define ASTC_NUM_COLOR_RANGES 17

/* Colour endpoint ranges in ascending size (0..5 up to 0..255). */
extern const astc_range astc_color_ranges[ASTC_NUM_COLOR_RANGES];

/* Weight range for the 3-bit range value rho (2..7) and precision bit p
 * (spec table 160); NULL when rho is invalid. */
const astc_range *astc_weight_range(uint32_t p, uint32_t rho);

/* Number of bits that `count` values of range r occupy. */
uint32_t astc_ise_size(uint32_t count, const astc_range *r);

/* Decodes `count` values (as trit/quint * 2^bits + low bits) from the 128-bit
 * buffer, starting at bit `start`. Bits past start + astc_ise_size() read as
 * zero. */
void astc_ise_decode(const uint8_t buf[16], uint32_t start, uint32_t count,
		     const astc_range *r, uint8_t *out);

/* Maps a decoded value to 0..255 (colour) or 0..64 (weight). */
uint32_t astc_unquant_color(const astc_range *r, uint32_t v);
uint32_t astc_unquant_weight(const astc_range *r, uint32_t v);

typedef enum astc_kind {
	ASTC_BLOCK_NORMAL = 0,
	ASTC_BLOCK_VOID,
	ASTC_BLOCK_ERROR
} astc_kind;

/* Everything decoded from the configuration fields of one block. */
typedef struct astc_block {
	astc_kind kind;
	uint16_t color[4];  /* void-extent colour as R, G, B, A (UNORM16) */
	uint8_t gw, gh;     /* weight grid size */
	uint8_t dual;       /* dual-plane flag */
	uint8_t nparts;
	uint16_t seed;      /* partition pattern index */
	uint8_t cem[4];     /* endpoint mode per partition */
	uint8_t ncvals;     /* colour integers over all partitions */
	uint8_t ccs;        /* component that uses the second weight plane */
	uint32_t cstart;    /* first bit of the colour integer sequence */
	astc_range wrange;
	astc_range crange;
} astc_block;

/* Reads the configuration of a block for a bw x bh footprint. Returns the
 * kind and fills *b (kind == ASTC_BLOCK_ERROR means an illegal encoding). */
astc_kind astc_parse_block(const uint8_t blk[16], uint32_t bw, uint32_t bh,
			   astc_block *b);

/* astc_parse_block with the DTC_COMPAT behaviour for void-extent blocks when
 * `compat` is non-zero (see texturecodec.h). */
astc_kind astc_parse_block_ex(const uint8_t blk[16], uint32_t bw, uint32_t bh,
			      astc_block *b, int compat);

/* Block-mode field (11 bits) to weight grid, dual flag and weight range.
 * Returns 0, or -1 for the reserved modes. Void-extent must be excluded by
 * the caller. */
int astc_parse_block_mode(uint32_t bm, astc_block *b);

/* hash52 of spec 23.20. */
uint32_t astc_hash52(uint32_t p);

/* Fills out[y * bw + x] with the partition (0..nparts-1) of each texel. */
void astc_partition_map(uint32_t seed, uint32_t nparts, uint32_t bw,
			uint32_t bh, uint8_t *out);

/* Colour endpoints of every partition after unquantization and endpoint
 * decoding: ep[partition][endpoint 0 or 1][R, G, B, A], each 0..255. Bit i
 * of the return value is set when partition i uses an HDR endpoint mode
 * (unusable in LDR operation mode). */
uint32_t astc_decode_endpoints(const uint8_t blk[16], const astc_block *b,
			       int ep[4][2][4]);

#endif
