/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "astc_internal.h"

uint32_t astc_hash52(uint32_t p)
{
	p ^= p >> 15;
	p -= p << 17;
	p += p << 7;
	p += p << 4;
	p ^= p >> 5;
	p += p << 16;
	p ^= p >> 7;
	p ^= p >> 3;
	p ^= p << 6;
	p ^= p >> 17;
	return p;
}

/* select_partition() of spec 23.20 for z = 0, with the hash hoisted out of
 * the per-texel loop. */
void astc_partition_map(uint32_t seed, uint32_t nparts, uint32_t bw,
			uint32_t bh, uint8_t *out)
{
	uint32_t small = bw * bh < 31;
	uint32_t rnum, s[8], sh1, sh2, x, y, i, v[4], best;

	if (nparts == 1) {
		memset(out, 0, (size_t)bw * bh);
		return;
	}
	seed += (nparts - 1) * 1024;
	rnum = astc_hash52(seed);
	if (seed & 1) {
		sh1 = (seed & 2) ? 4 : 5;
		sh2 = nparts == 3 ? 6 : 5;
	} else {
		sh1 = nparts == 3 ? 6 : 5;
		sh2 = (seed & 2) ? 4 : 5;
	}
	for (i = 0; i < 8; i++) {
		s[i] = (rnum >> (4 * i)) & 0xF;
		s[i] = (s[i] * s[i]) >> ((i & 1) ? sh2 : sh1);
	}
	for (y = 0; y < bh; y++)
		for (x = 0; x < bw; x++) {
			uint32_t xs = small ? x << 1 : x;
			uint32_t ys = small ? y << 1 : y;

			for (i = 0; i < 4; i++)
				v[i] = (s[2 * i] * xs + s[2 * i + 1] * ys +
					(rnum >> (14 - 4 * i))) & 0x3F;
			if (nparts < 4)
				v[3] = 0;
			if (nparts < 3)
				v[2] = 0;
			best = 0;
			for (i = 1; i < 4; i++)
				if (v[i] > v[best])
					best = i;
			out[y * bw + x] = (uint8_t)best;
		}
}
