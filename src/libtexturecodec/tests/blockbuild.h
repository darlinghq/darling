/* SPDX-License-Identifier: GPL-3.0-only */
/* Test-side block constructors, written from the bit layouts of the spec
 * (not from the decoder). */
#ifndef DTC_TEST_BLOCKBUILD_H
#define DTC_TEST_BLOCKBUILD_H

#include <stdint.h>
#include <string.h>

static inline void put(uint8_t *b, uint32_t pos, uint32_t n, uint32_t v)
{
	uint32_t i;

	for (i = 0; i < n; i++) {
		b[(pos + i) >> 3] &= (uint8_t)~(1u << ((pos + i) & 7));
		b[(pos + i) >> 3] |= (uint8_t)(((v >> i) & 1) << ((pos + i) & 7));
	}
}

/* Inverse of table 161, written from the table: block mode bits for a grid. */
static inline int encode_mode(uint32_t w, uint32_t h, uint32_t rho, uint32_t p,
		       uint32_t dp, uint32_t *bm)
{
	uint32_t lo = (rho & 1) << 4;	/* rho0 */
	uint32_t hi = ((rho >> 2) & 1) << 1 | ((rho >> 1) & 1);	/* r2 r1 low */
	uint32_t hi_hi = ((rho >> 2) & 1) << 3 | ((rho >> 1) & 1) << 2;
	uint32_t top = dp << 10 | p << 9;

	if (w >= 4 && w <= 7 && h >= 2 && h <= 5)
		*bm = top | (w - 4) << 7 | (h - 2) << 5 | lo | 0 << 2 | hi;
	else if (w >= 8 && w <= 11 && h >= 2 && h <= 5)
		*bm = top | (w - 8) << 7 | (h - 2) << 5 | lo | 1 << 2 | hi;
	else if (w >= 2 && w <= 5 && h >= 8 && h <= 11)
		*bm = top | (h - 8) << 7 | (w - 2) << 5 | lo | 2 << 2 | hi;
	else if (w >= 2 && w <= 5 && h >= 6 && h <= 7)
		*bm = top | (h - 6) << 7 | (w - 2) << 5 | lo | 3 << 2 | hi;
	else if (w >= 2 && w <= 3 && h >= 2 && h <= 5)
		*bm = top | 1 << 8 | (w - 2) << 7 | (h - 2) << 5 | lo | 3 << 2 | hi;
	else if (w == 12 && h >= 2 && h <= 5)
		*bm = top | (h - 2) << 5 | lo | hi_hi;
	else if (h == 12 && w >= 2 && w <= 5)
		*bm = top | 1 << 7 | (w - 2) << 5 | lo | hi_hi;
	else if (w == 6 && h == 10)
		*bm = top | 3 << 7 | lo | hi_hi;
	else if (w == 10 && h == 6)
		*bm = top | 3 << 7 | 1 << 5 | lo | hi_hi;
	else if (w >= 6 && w <= 9 && h >= 6 && h <= 9 && !dp && !p)
		*bm = (h - 6) << 9 | 2 << 7 | (w - 6) << 5 | lo | hi_hi;
	else
		return -1;
	return 0;
}

static inline void void_block(uint8_t *blk)
{
	memset(blk, 0, 16);
	put(blk, 0, 9, 0x1FC);
	put(blk, 10, 2, 3);
	put(blk, 12, 13, 0x1FFF);
	put(blk, 25, 13, 0x1FFF);
	put(blk, 38, 13, 0x1FFF);
	put(blk, 51, 13, 0x1FFF);
	put(blk, 64, 16, 0x1234);
	put(blk, 80, 16, 0x5678);
	put(blk, 96, 16, 0x9ABC);
	put(blk, 112, 16, 0xDEF0);
}

/* Writes `n` values of `bits` bits into the weight stream, which grows down
 * from bit 127 (stream bit k is block bit 127 - k). */
static inline void put_weights(uint8_t *b, uint32_t first, uint32_t n, uint32_t bits,
			const uint8_t *v)
{
	uint32_t i, k;

	for (i = 0; i < n; i++)
		for (k = 0; k < bits; k++) {
			uint32_t stream = (first + i) * bits + k;

			put(b, 127 - stream, 1, (uint32_t)(v[i] >> k) & 1);
		}
}

#endif
