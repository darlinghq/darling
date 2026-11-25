/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "astc_internal.h"
#include "blockbuild.h"
#include "check.h"
#include "texturecodec/texturecodec.h"

static const uint8_t magenta[4] = { 255, 0, 255, 255 };
static const uint8_t unq2[4] = { 0, 21, 43, 64 };	/* 2-bit weights, 0..64 */

/* 4x4 footprint, 4x4 weight grid of 2-bit weights, one partition, 8-bit
 * endpoint integers. With the grid as large as the footprint, infill is the
 * identity, so texel (x, y) uses weight w[y * 4 + x]. */
static void single(uint8_t *blk, uint32_t cem, const uint8_t *vals, uint32_t n,
		   const uint8_t *w0, const uint8_t *w1, uint32_t ccs)
{
	uint8_t both[32];
	uint32_t i, bm;

	memset(blk, 0, 16);
	encode_mode(4, 4, 4, 0, w1 != NULL, &bm);
	put(blk, 0, 11, bm);
	put(blk, 13, 4, cem);
	for (i = 0; i < n; i++)
		put(blk, 17 + 8 * i, 8, vals[i]);
	if (w1) {
		for (i = 0; i < 16; i++) {
			both[2 * i] = w0[i];
			both[2 * i + 1] = w1[i];
		}
		put_weights(blk, 0, 32, 2, both);
		put(blk, 62, 2, ccs);
	} else {
		put_weights(blk, 0, 16, 2, w0);
	}
}

/* Expected channel value straight from the formulas of 23.18.1. */
static uint8_t lerp(uint32_t c0, uint32_t c1, uint32_t w, int srgb)
{
	uint32_t a = srgb ? (c0 << 8 | 0x80) : (c0 << 8 | c0);
	uint32_t b = srgb ? (c1 << 8 | 0x80) : (c1 << 8 | c1);

	return (uint8_t)(((a * (64 - w) + b * w + 32) / 64) >> 8);
}

static const uint8_t wpat[16] = { 0, 1, 2, 3, 3, 2, 1, 0,
				  1, 1, 2, 2, 3, 0, 3, 0 };
static const uint8_t wpat2[16] = { 3, 3, 0, 0, 1, 2, 3, 1,
				   0, 2, 1, 3, 2, 2, 0, 1 };

static void test_luminance(int srgb)
{
	uint8_t blk[16], out[64];
	const uint8_t v[2] = { 10, 200 };
	int i, err = 9;

	single(blk, 0, v, 2, wpat, NULL, 0);
	CHECK_EQ(dtc_astc_decode_block(blk, 4, 4, srgb, out, &err), DTC_OK);
	CHECK_EQ(err, 0);
	for (i = 0; i < 16; i++) {
		uint8_t l = lerp(10, 200, unq2[wpat[i]], srgb);

		CHECK_EQ(out[4 * i], l);
		CHECK_EQ(out[4 * i + 1], l);
		CHECK_EQ(out[4 * i + 2], l);
		CHECK_EQ(out[4 * i + 3], 255);
	}
}

/* Mode 4 with CCS 3: luminance follows plane 0 and alpha plane 1. */
static void test_dual_plane_alpha(void)
{
	uint8_t blk[16], out[64];
	const uint8_t v[4] = { 20, 220, 100, 30 };
	int i;

	single(blk, 4, v, 4, wpat, wpat2, 3);
	dtc_astc_decode_block(blk, 4, 4, 0, out, NULL);
	for (i = 0; i < 16; i++) {
		CHECK_EQ(out[4 * i], lerp(20, 220, unq2[wpat[i]], 0));
		CHECK_EQ(out[4 * i + 2], lerp(20, 220, unq2[wpat[i]], 0));
		CHECK_EQ(out[4 * i + 3], lerp(100, 30, unq2[wpat2[i]], 0));
	}
}

/* Mode 6 (RGB base+scale) with CCS 1: only green uses plane 1. */
static void test_dual_plane_green(void)
{
	uint8_t blk[16], out[64];
	const uint8_t v[4] = { 200, 100, 50, 128 };
	int i;

	single(blk, 6, v, 4, wpat, wpat2, 1);
	dtc_astc_decode_block(blk, 4, 4, 0, out, NULL);
	for (i = 0; i < 16; i++) {
		CHECK_EQ(out[4 * i], lerp(100, 200, unq2[wpat[i]], 0));
		CHECK_EQ(out[4 * i + 1], lerp(50, 100, unq2[wpat2[i]], 0));
		CHECK_EQ(out[4 * i + 2], lerp(25, 50, unq2[wpat[i]], 0));
		CHECK_EQ(out[4 * i + 3], 255);
	}
}

static void test_void_extent(void)
{
	uint8_t blk[16], out[144 * 4];
	int i, srgb, err;

	void_block(blk);
	for (srgb = 0; srgb < 2; srgb++) {
		CHECK_EQ(dtc_astc_decode_block(blk, 12, 12, srgb, out, &err), DTC_OK);
		CHECK_EQ(err, 0);
		for (i = 0; i < 144; i++) {
			CHECK_EQ(out[4 * i], 0x12);
			CHECK_EQ(out[4 * i + 1], 0x56);
			CHECK_EQ(out[4 * i + 2], 0x9A);
			CHECK_EQ(out[4 * i + 3], 0xDE);
		}
	}
}

/* DTC_COMPAT (see void_component): a void-extent colour is rounded to 8 bits, not
 * cut to its top byte: 0xC000 gives 0xBF, 0x8000 gives 0x80, 0xFFFF gives 0xFF,
 * 0x0080 gives 0x00 and 0x0081 gives 0x01 (the values macOS ImageIO gives; the top
 * byte rule gives 0xC0, 0x80, 0xFF, 0x00 and 0x00). A block with its reserved bits
 * cleared is the same constant colour block. */
static void test_void_extent_compat(void)
{
	uint8_t blk[16], out[16 * 4];
	int err = 1, mode;

	void_block(blk);
	put(blk, 64, 16, 0xFFFF);
	put(blk, 80, 16, 0x8000);
	put(blk, 96, 16, 0x0080);
	put(blk, 112, 16, 0xC000);
	for (mode = DTC_COMPAT; mode <= (DTC_COMPAT | DTC_SRGB); mode++) {
		CHECK_EQ(dtc_astc_decode_block(blk, 4, 4, mode, out, &err), DTC_OK);
		CHECK_EQ(err, 0);
		CHECK_EQ(out[0], 0xFF);
		CHECK_EQ(out[1], 0x80);
		CHECK_EQ(out[2], 0x00);
		CHECK_EQ(out[3], 0xBF);
	}
	CHECK_EQ(dtc_astc_decode_block(blk, 4, 4, DTC_SRGB, out, &err), DTC_OK);
	CHECK_EQ(out[3], 0xC0);	/* without DTC_COMPAT the top byte */
	put(blk, 96, 16, 0x0081);
	put(blk, 10, 2, 0);	/* reserved bits cleared */
	CHECK_EQ(dtc_astc_decode_block(blk, 4, 4, DTC_COMPAT | DTC_SRGB, out, &err), DTC_OK);
	CHECK_EQ(err, 0);
	CHECK_EQ(out[2], 0x01);
	CHECK_EQ(out[60], 0xFF);
	CHECK_EQ(out[63], 0xBF);
	CHECK_EQ(dtc_astc_decode_block(blk, 4, 4, DTC_SRGB, out, &err), DTC_OK);
	CHECK_EQ(err, 1);	/* the specification's reading */
}

static int all_magenta(const uint8_t *out, int n)
{
	int i;

	for (i = 0; i < n; i++)
		if (memcmp(out + 4 * i, magenta, 4))
			return 0;
	return 1;
}

static void test_error_blocks(void)
{
	uint8_t blk[16], out[64];
	int err = 0;
	uint32_t bm;

	memset(blk, 0, 16);	/* reserved block mode */
	dtc_astc_decode_block(blk, 4, 4, 0, out, &err);
	CHECK(all_magenta(out, 16));
	CHECK_EQ(err, 1);
	void_block(blk);
	put(blk, 9, 1, 1);	/* HDR void extent, LDR operation mode */
	dtc_astc_decode_block(blk, 4, 4, 1, out, &err);
	CHECK(all_magenta(out, 16));
	CHECK_EQ(err, 1);
	memset(blk, 0, 16);	/* grid wider than the footprint */
	encode_mode(5, 4, 4, 0, 0, &bm);
	put(blk, 0, 11, bm);
	dtc_astc_decode_block(blk, 4, 4, 0, out, &err);
	CHECK(all_magenta(out, 16));
	CHECK_EQ(err, 1);
}

/* Two partitions, partition 1 uses HDR mode 7: its texels are magenta and
 * the others decode normally (spec 23.23, last paragraph). */
static void test_hdr_partition_is_error_only_there(void)
{
	uint8_t blk[16], out[64], map[16];
	const uint8_t v[6] = { 10, 200, 1, 2, 3, 4 };
	int i, err = 0, n0 = 0, n1 = 0;
	uint32_t bm;

	memset(blk, 0, 16);
	encode_mode(4, 4, 4, 0, 0, &bm);
	put(blk, 0, 11, bm);
	put(blk, 11, 2, 1);
	put(blk, 13, 10, 1);	/* seed 1, 2 partitions */
	put(blk, 23, 2, 1);
	put(blk, 26, 1, 1);	/* partition 1: class 1 */
	put(blk, 27, 2, 0);	/* M0 = 0: CEM 0 */
	put(blk, 128 - 32 - 2, 2, 3);	/* M1 = 3: CEM 7 */
	for (i = 0; i < 6; i++)
		put(blk, 29 + 8 * i, 8, v[i]);
	put_weights(blk, 0, 16, 2, wpat);
	astc_partition_map(1, 2, 4, 4, map);
	dtc_astc_decode_block(blk, 4, 4, 0, out, &err);
	CHECK_EQ(err, 1);
	for (i = 0; i < 16; i++) {
		if (map[i]) {
			n1++;
			CHECK(!memcmp(out + 4 * i, magenta, 4));
		} else {
			n0++;
			CHECK_EQ(out[4 * i], lerp(10, 200, unq2[wpat[i]], 0));
			CHECK_EQ(out[4 * i + 3], 255);
		}
	}
	CHECK(n0 > 0 && n1 > 0);
}

/* Spec 23.17 reads weights at js+1 and jt+1; for the legal footprints and
 * grids the interpolation weight of such a neighbour is zero, so the decoder
 * may treat an out-of-grid neighbour as 0. Checked by brute force with the
 * formulas of the spec. */
static void test_infill_stays_in_grid(void)
{
	static const uint32_t dims[] = { 4, 5, 6, 8, 10, 12 };
	uint32_t i, b, g, x, ds, gs, js, fs;

	for (i = 0; i < sizeof(dims) / sizeof(dims[0]); i++) {
		b = dims[i];
		ds = (1024 + b / 2) / (b - 1);
		for (g = 2; g <= b; g++)
			for (x = 0; x < b; x++) {
				gs = (ds * x * (g - 1) + 32) >> 6;
				js = gs >> 4;
				fs = gs & 15;
				CHECK(js + (fs ? 1u : 0u) <= g - 1);
			}
	}
}

static void test_arguments(void)
{
	uint8_t blk[16] = { 0 }, out[16 * 4];

	CHECK_EQ(dtc_astc_decode_block(NULL, 4, 4, 0, out, NULL), DTC_E_ARG);
	CHECK_EQ(dtc_astc_decode_block(blk, 4, 4, 0, NULL, NULL), DTC_E_ARG);
	CHECK_EQ(dtc_astc_decode_block(blk, 5, 6, 0, out, NULL), DTC_E_ARG);
	CHECK_EQ(dtc_astc_decode_block(blk, 0, 0, 0, out, NULL), DTC_E_ARG);
	CHECK_EQ(dtc_astc_decode_block(blk, 3, 3, 0, out, NULL), DTC_E_ARG);
}

int main(void)
{
	test_luminance(0);
	test_luminance(1);
	test_dual_plane_alpha();
	test_dual_plane_green();
	test_void_extent();
	test_void_extent_compat();
	test_error_blocks();
	test_hdr_partition_is_error_only_there();
	test_infill_stays_in_grid();
	test_arguments();
	CHECK_DONE();
}
