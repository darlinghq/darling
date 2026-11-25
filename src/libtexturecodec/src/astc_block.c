/* SPDX-License-Identifier: GPL-3.0-only */
#include "astc_internal.h"

static uint32_t get_bits(const uint8_t *blk, uint32_t pos, uint32_t n)
{
	uint32_t v = 0, i, p;

	for (i = 0; i < n; i++) {
		p = pos + i;
		if (p < 128)
			v |= (uint32_t)((blk[p >> 3] >> (p & 7)) & 1) << i;
	}
	return v;
}

/* Table 161. Bits 1:0 nonzero select the first five rows by bits 3:2; with
 * bits 1:0 zero the layout comes from bits 8:7. */
static int grid_from_mode(uint32_t bm, uint32_t *rho, uint32_t *w, uint32_t *h,
			  uint32_t *dp, uint32_t *p)
{
	uint32_t r0 = (bm >> 4) & 1, a = (bm >> 7) & 3, b = (bm >> 5) & 3;

	*dp = (bm >> 10) & 1;
	*p = (bm >> 9) & 1;
	if (bm & 3) {
		*rho = r0 | (bm & 1) << 1 | ((bm >> 1) & 1) << 2;
		switch ((bm >> 2) & 3) {
		case 0: *w = a + 4; *h = b + 2; break;
		case 1: *w = a + 8; *h = b + 2; break;
		case 2: *w = b + 2; *h = a + 8; break;
		default:
			if (bm & 0x100) { *w = (a & 1) + 2; *h = b + 2; }
			else { *w = b + 2; *h = (a & 1) + 6; }
		}
		return 0;
	}
	*rho = r0 | ((bm >> 2) & 1) << 1 | ((bm >> 3) & 1) << 2;
	if ((bm & 0xC) == 0 || ((bm >> 6) & 7) == 7)
		return -1;
	switch (a) {
	case 0: *w = 12; *h = b + 2; break;
	case 1: *w = b + 2; *h = 12; break;
	case 3:
		*w = b ? 10 : 6;
		*h = b ? 6 : 10;
		break;
	default:
		*w = b + 6;
		*h = ((bm >> 9) & 3) + 6;
		*dp = 0;
		*p = 0;
	}
	return 0;
}

int astc_parse_block_mode(uint32_t bm, astc_block *b)
{
	uint32_t rho, w, h, dp, p;
	const astc_range *r;

	if (grid_from_mode(bm, &rho, &w, &h, &dp, &p))
		return -1;
	r = astc_weight_range(p, rho);
	if (!r)
		return -1;
	b->gw = (uint8_t)w;
	b->gh = (uint8_t)h;
	b->dual = (uint8_t)dp;
	b->wrange = *r;
	return 0;
}

/* QUIRK (DTC_COMPAT only): a void-extent block with D = 0 is a constant colour
 * block whatever its reserved bits 10 and 11 and its extent coordinates hold.
 * The specification (23.22, 23.23) calls cleared reserved bits and a low
 * coordinate that is not below the high one illegal, but macOS ImageIO decodes
 * such blocks as the colour, for all 176 random LDR void-extent blocks tried
 * on macOS 26.7.3 and on 10.14.4. A void-extent block with D = 1 (HDR) stays an
 * error in LDR mode. Evidence: black-box observation through CGImageSource,
 * tier 2 (vd-upstream cristim-mac-evidence.md, section 16).
 * Test: test_void_extent_compat in tests/test_astc_block.c. */
static astc_kind parse_void(const uint8_t *blk, astc_block *b, int compat)
{
	uint32_t c[4], i, ones = 0;

	if (get_bits(blk, 9, 1))
		return ASTC_BLOCK_ERROR;
	if (!compat) {
		if (get_bits(blk, 10, 2) != 3)
			return ASTC_BLOCK_ERROR;
		for (i = 0; i < 4; i++) {
			c[i] = get_bits(blk, 12 + 13 * i, 13);
			ones += c[i] == 0x1FFF;
		}
		if (ones != 4 && (c[0] >= c[1] || c[2] >= c[3]))
			return ASTC_BLOCK_ERROR;
	}
	for (i = 0; i < 4; i++)
		b->color[i] = (uint16_t)get_bits(blk, 64 + 16 * i, 16);
	return ASTC_BLOCK_VOID;
}

/* Fills cem[] and returns the number of CEM bits stored below the weights,
 * or -1 when the CEM field is unusable (spec 23.11, tables 164 to 166). */
static int parse_cems(const uint8_t *blk, astc_block *b, uint32_t wbits)
{
	uint32_t np = b->nparts, sel, stream, i, extra, cls;

	if (np == 1) {
		b->cem[0] = (uint8_t)get_bits(blk, 13, 4);
		return 0;
	}
	sel = get_bits(blk, 23, 2);
	if (sel == 0) {
		for (i = 0; i < np; i++)
			b->cem[i] = (uint8_t)get_bits(blk, 25, 4);
		return 0;
	}
	extra = 3 * np - 4;
	stream = get_bits(blk, 25, 4) |
		 get_bits(blk, 128 - wbits - extra, extra) << 4;
	for (i = 0; i < np; i++) {
		cls = sel - 1 + ((stream >> i) & 1);
		b->cem[i] = (uint8_t)(cls << 2 | ((stream >> (np + 2 * i)) & 3));
	}
	return (int)extra;
}

static int pick_color_range(astc_block *b, int remaining)
{
	uint32_t i, pairs = 0;

	for (i = 0; i < b->nparts; i++)
		pairs += (uint32_t)(b->cem[i] >> 2) + 1;
	b->ncvals = (uint8_t)(2 * pairs);
	if (2 * pairs > 18)
		return -1;
	for (i = ASTC_NUM_COLOR_RANGES; i-- > 0;)
		if ((int)astc_ise_size(2 * pairs, &astc_color_ranges[i]) <=
		    remaining) {
			b->crange = astc_color_ranges[i];
			return 0;
		}
	return -1;
}

static astc_kind parse_normal(const uint8_t *blk, uint32_t bw, uint32_t bh,
			      astc_block *b)
{
	uint32_t nw, wbits, config;
	int extra;

	if (astc_parse_block_mode(get_bits(blk, 0, 11), b))
		return ASTC_BLOCK_ERROR;
	nw = (uint32_t)b->gw * b->gh * (1u + b->dual);
	if (b->gw > bw || b->gh > bh || nw > 64)
		return ASTC_BLOCK_ERROR;
	wbits = astc_ise_size(nw, &b->wrange);
	if (wbits < 24 || wbits > 96)
		return ASTC_BLOCK_ERROR;
	b->nparts = (uint8_t)(get_bits(blk, 11, 2) + 1);
	if (b->dual && b->nparts == 4)
		return ASTC_BLOCK_ERROR;
	b->seed = (uint16_t)(b->nparts > 1 ? get_bits(blk, 13, 10) : 0);
	extra = parse_cems(blk, b, wbits);
	config = b->nparts == 1 ? 17 : (get_bits(blk, 23, 2) == 0 ? 29 :
					25 + 3 * b->nparts);
	config += b->dual ? 2 : 0;
	if (pick_color_range(b, (int)(128 - config - wbits)))
		return ASTC_BLOCK_ERROR;
	b->cstart = b->nparts == 1 ? 17 : 29;
	b->ccs = (uint8_t)(b->dual ? get_bits(blk, 128 - wbits - (uint32_t)extra - 2, 2) : 0);
	return ASTC_BLOCK_NORMAL;
}

astc_kind astc_parse_block_ex(const uint8_t blk[16], uint32_t bw, uint32_t bh,
			      astc_block *b, int compat)
{
	b->kind = (get_bits(blk, 0, 9) == 0x1FC) ? parse_void(blk, b, compat)
						 : parse_normal(blk, bw, bh, b);
	return b->kind;
}

astc_kind astc_parse_block(const uint8_t blk[16], uint32_t bw, uint32_t bh,
			   astc_block *b)
{
	return astc_parse_block_ex(blk, bw, bh, b, 0);
}
