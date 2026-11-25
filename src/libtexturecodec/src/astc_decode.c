/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "astc_internal.h"
#include "texturecodec/texturecodec.h"

static void fill(uint8_t *out, uint32_t texels, const uint8_t px[4])
{
	uint32_t i;

	for (i = 0; i < texels; i++)
		memcpy(out + 4 * i, px, 4);
}

static void fill_error(uint8_t *out, uint32_t texels)
{
	static const uint8_t magenta[4] = { 255, 0, 255, 255 };

	fill(out, texels, magenta);
}

/* Stream bit n of the weights is block bit 127 - n (spec 23.15). */
static void reverse_bits(const uint8_t blk[16], uint8_t rev[16])
{
	uint32_t n;

	memset(rev, 0, 16);
	for (n = 0; n < 128; n++)
		rev[n >> 3] |= (uint8_t)(((blk[(127 - n) >> 3] >>
					   ((127 - n) & 7)) & 1) << (n & 7));
}

/* Unquantized weights 0..64 of both planes in grid order. */
static void decode_weights(const uint8_t blk[16], const astc_block *b,
			   uint8_t w[2][64])
{
	uint8_t rev[16], raw[64];
	uint32_t i, n = (uint32_t)b->gw * b->gh, planes = 1u + b->dual;

	reverse_bits(blk, rev);
	astc_ise_decode(rev, 0, n * planes, &b->wrange, raw);
	for (i = 0; i < n; i++) {
		w[0][i] = (uint8_t)astc_unquant_weight(&b->wrange,
						       raw[planes * i]);
		if (b->dual)
			w[1][i] = (uint8_t)astc_unquant_weight(
				&b->wrange, raw[planes * i + 1]);
	}
}

static uint32_t grid_weight(const uint8_t *g, uint32_t n, uint32_t idx)
{
	return idx < n ? g[idx] : 0;
}

/* Weight infill of spec 23.17 (2D): scale factors ds and dt are precomputed
 * per block dimension. */
static uint32_t infill(const uint8_t *g, uint32_t gw, uint32_t gh,
		       uint32_t ds, uint32_t dt, uint32_t x, uint32_t y)
{
	uint32_t gs = (ds * x * (gw - 1) + 32) >> 6;
	uint32_t gt = (dt * y * (gh - 1) + 32) >> 6;
	uint32_t fs = gs & 15, ft = gt & 15, n = gw * gh;
	uint32_t v0 = (gs >> 4) + (gt >> 4) * gw;
	uint32_t w11 = (fs * ft + 8) >> 4, w10 = ft - w11, w01 = fs - w11;
	uint32_t w00 = 16 - fs - ft + w11;

	return (grid_weight(g, n, v0) * w00 + grid_weight(g, n, v0 + 1) * w01 +
		grid_weight(g, n, v0 + gw) * w10 +
		grid_weight(g, n, v0 + gw + 1) * w11 + 8) >> 4;
}

/* A 16-bit void-extent component as 8 bits, DTC_COMPAT: the nearest of 255
 * steps over the full range, which is (v * 255 + 32767) / 65535 for every v,
 * not the top 8 bits. macOS ImageIO gives 0xBF for 0xC000 and 0x80 for 0x8000
 * (the top byte gives 0xC0 for the first).
 * QUIRK: the specification says nothing about this step. Evidence: black-box
 * observation through CGImageSource on macOS 26.7.3 and 10.14.4, tier 2
 * (vd-upstream cristim-mac-evidence.md, section 16).
 * Test: test_void_extent_compat in tests/test_astc_decode.c. */
static uint8_t void_component(uint32_t v, int compat)
{
	return (uint8_t)(compat ? (v + 128) / 257 : v >> 8);
}

/* One colour component: expand both 8-bit endpoints to 16 bits, blend, keep
 * the top 8 bits (spec 23.18.1). */
static uint8_t blend(uint32_t c0, uint32_t c1, uint32_t i, int srgb)
{
	uint32_t a = (c0 << 8) | (srgb ? 0x80 : c0);
	uint32_t b = (c1 << 8) | (srgb ? 0x80 : c1);

	return (uint8_t)((((a * (64 - i) + b * i + 32) >> 6)) >> 8);
}

typedef struct block_ctx {
	astc_block b;
	int ep[4][2][4];
	uint32_t hdr;
	uint8_t w[2][64];
	uint8_t part[144];
	uint32_t ds, dt;	/* infill scale factors of the footprint */
} block_ctx;

/* Returns -1 when the texel belongs to a partition with an HDR endpoint
 * mode, which LDR operation mode cannot decode. */
static int texel(const block_ctx *c, uint32_t bw, uint32_t x, uint32_t y,
		 int srgb, uint8_t *px)
{
	uint32_t p = c->part[y * bw + x], k, w;
	uint32_t i0 = infill(c->w[0], c->b.gw, c->b.gh, c->ds, c->dt, x, y);
	uint32_t i1 = c->b.dual ?
		infill(c->w[1], c->b.gw, c->b.gh, c->ds, c->dt, x, y) : i0;

	if (c->hdr & (1u << p))
		return -1;
	for (k = 0; k < 4; k++) {
		w = (c->b.dual && c->b.ccs == k) ? i1 : i0;
		px[k] = blend((uint32_t)c->ep[p][0][k],
			      (uint32_t)c->ep[p][1][k], w, srgb);
	}
	return 0;
}

static int decode_normal(const uint8_t blk[16], const astc_block *b,
			 uint32_t bw, uint32_t bh, int srgb, uint8_t *out)
{
	block_ctx c;
	uint32_t x, y, errors = 0;

	c.b = *b;
	c.ds = (1024 + bw / 2) / (bw - 1);
	c.dt = (1024 + bh / 2) / (bh - 1);
	c.hdr = astc_decode_endpoints(blk, b, c.ep);
	decode_weights(blk, b, c.w);
	astc_partition_map(b->seed, b->nparts, bw, bh, c.part);
	for (y = 0; y < bh; y++)
		for (x = 0; x < bw; x++) {
			uint8_t *px = out + 4 * (y * bw + x);

			if (texel(&c, bw, x, y, srgb, px)) {
				fill_error(px, 1);
				errors++;
			}
		}
	return errors != 0;
}

dtc_status dtc_astc_decode_block(const uint8_t block[16], uint32_t block_w,
				 uint32_t block_h, int srgb,
				 uint8_t *rgba_out, int *error_block)
{
	astc_block b;
	uint8_t px[4];
	uint32_t texels, i;
	int err = 0, compat = (srgb & DTC_COMPAT) != 0;

	srgb &= DTC_SRGB;

	if (!block || !rgba_out || !dtc_astc_footprint_valid(block_w, block_h))
		return DTC_E_ARG;
	texels = block_w * block_h;
	switch (astc_parse_block_ex(block, block_w, block_h, &b, compat)) {
	case ASTC_BLOCK_VOID:
		for (i = 0; i < 4; i++)
			px[i] = void_component(b.color[i], compat);
		fill(rgba_out, texels, px);
		break;
	case ASTC_BLOCK_NORMAL:
		err = decode_normal(block, &b, block_w, block_h, srgb,
				    rgba_out);
		break;
	default:
		fill_error(rgba_out, texels);
		err = 1;
	}
	if (error_block)
		*error_block = err;
	return DTC_OK;
}

dtc_status dtc_astc_decode_ex(const uint8_t *data, size_t len,
			      uint32_t block_w, uint32_t block_h,
			      uint32_t width, uint32_t height, int srgb,
			      uint8_t *rgba_out, dtc_decode_report *report)
{
	uint64_t bx, by, nblocks, row, bi;
	uint8_t tmp[144 * 4];
	uint32_t x0, y0, y, w, h;
	int err;

	if (report)
		memset(report, 0, sizeof(*report));
	if (!data || !rgba_out || !dtc_astc_footprint_valid(block_w, block_h) ||
	    !width || !height)
		return DTC_E_ARG;
	if ((uint64_t)width * height > SIZE_MAX / 4)
		return DTC_E_OVERFLOW;
	bx = ((uint64_t)width + block_w - 1) / block_w;
	by = ((uint64_t)height + block_h - 1) / block_h;
	nblocks = bx * by;
	if (nblocks > (uint64_t)len / DTC_ASTC_BLOCK_BYTES)
		return DTC_E_TRUNCATED;
	for (bi = 0; bi < nblocks; bi++) {
		dtc_astc_decode_block(data + bi * DTC_ASTC_BLOCK_BYTES,
				      block_w, block_h, srgb, tmp, &err);
		x0 = (uint32_t)(bi % bx) * block_w;
		y0 = (uint32_t)(bi / bx) * block_h;
		w = width - x0 < block_w ? width - x0 : block_w;
		h = height - y0 < block_h ? height - y0 : block_h;
		for (y = 0; y < h; y++) {
			row = ((uint64_t)(y0 + y) * width + x0) * 4;
			memcpy(rgba_out + row, tmp + 4 * y * block_w, 4 * w);
		}
		if (report) {
			report->blocks++;
			report->error_blocks += (uint64_t)err;
		}
	}
	return DTC_OK;
}

dtc_status dtc_astc_decode(const uint8_t *data, size_t len, uint32_t block_w,
			   uint32_t block_h, uint32_t width, uint32_t height,
			   int srgb, uint8_t *rgba_out)
{
	return dtc_astc_decode_ex(data, len, block_w, block_h, width, height,
				  srgb, rgba_out, NULL);
}
