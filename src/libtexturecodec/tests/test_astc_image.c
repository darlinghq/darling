/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdlib.h>
#include <string.h>

#include "blockbuild.h"
#include "check.h"
#include "texturecodec/texturecodec.h"

static void solid(uint8_t *blk, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
	void_block(blk);
	put(blk, 64, 16, r << 8);
	put(blk, 80, 16, g << 8);
	put(blk, 96, 16, b << 8);
	put(blk, 112, 16, a << 8);
}

/* 5x3 image of 4x4 blocks: a full block on the left, a clipped one on the
 * right; the border bytes around the output must stay untouched. */
static void test_partial_blocks(void)
{
	uint8_t data[34], out[5 * 3 * 4 + 8];
	dtc_decode_report rep;
	int x, y;

	solid(data, 1, 2, 3, 4);
	solid(data + 16, 9, 8, 7, 6);
	data[32] = 0xEE;	/* trailing bytes are ignored */
	data[33] = 0xEE;
	memset(out, 0x5A, sizeof(out));
	CHECK_EQ(dtc_astc_decode_ex(data, sizeof(data), 4, 4, 5, 3, 0, out + 4,
				    &rep), DTC_OK);
	CHECK_EQ(rep.blocks, 2);
	CHECK_EQ(rep.error_blocks, 0);
	for (y = 0; y < 3; y++)
		for (x = 0; x < 5; x++) {
			const uint8_t *p = out + 4 + 4 * (y * 5 + x);
			const uint8_t want[4] = { 1, 2, 3, 4 };
			const uint8_t want2[4] = { 9, 8, 7, 6 };

			CHECK(!memcmp(p, x < 4 ? want : want2, 4));
		}
	CHECK_EQ(out[0], 0x5A);
	CHECK_EQ(out[sizeof(out) - 1], 0x5A);
}

static void test_report_counts_errors(void)
{
	uint8_t data[32] = { 0 }, out[8 * 4 * 4];
	dtc_decode_report rep;

	solid(data, 1, 1, 1, 1);	/* second block is all zeros: reserved */
	CHECK_EQ(dtc_astc_decode_ex(data, 32, 4, 4, 8, 4, 1, out, &rep), DTC_OK);
	CHECK_EQ(rep.blocks, 2);
	CHECK_EQ(rep.error_blocks, 1);
	CHECK_EQ(out[16 + 0], 255);
	CHECK_EQ(out[16 + 1], 0);
}

static void test_errors(void)
{
	uint8_t data[32] = { 0 }, out[64];
	dtc_status st;

	CHECK_EQ(dtc_astc_decode(NULL, 32, 4, 4, 4, 4, 0, out), DTC_E_ARG);
	CHECK_EQ(dtc_astc_decode(data, 32, 4, 4, 4, 4, 0, NULL), DTC_E_ARG);
	CHECK_EQ(dtc_astc_decode(data, 32, 4, 4, 0, 4, 0, out), DTC_E_ARG);
	CHECK_EQ(dtc_astc_decode(data, 32, 4, 4, 4, 0, 0, out), DTC_E_ARG);
	CHECK_EQ(dtc_astc_decode(data, 32, 7, 7, 4, 4, 0, out), DTC_E_ARG);
	CHECK_EQ(dtc_astc_decode(data, 15, 4, 4, 4, 4, 0, out), DTC_E_TRUNCATED);
	CHECK_EQ(dtc_astc_decode(data, 16, 4, 4, 5, 4, 0, out), DTC_E_TRUNCATED);
	CHECK_EQ(dtc_astc_decode(data, 0, 4, 4, 1, 1, 0, out), DTC_E_TRUNCATED);
	{
		dtc_decode_report rep = { 7, 7 };

		CHECK_EQ(dtc_astc_decode_ex(data, 0, 4, 4, 4, 4, 0, out, &rep),
			 DTC_E_TRUNCATED);
		CHECK(rep.blocks == 0 && rep.error_blocks == 0);
	}
	/* huge dimensions with a tiny payload must not allocate or loop */
	CHECK_EQ(dtc_astc_decode(data, 32, 4, 4, 0xFFFFFFFFu, 0xFFFFFFFFu, 0,
				 out), DTC_E_OVERFLOW);
	st = dtc_astc_decode(data, 32, 12, 12, 0xFFFFFFFFu, 1, 0, out);
	CHECK(st == DTC_E_TRUNCATED || st == DTC_E_OVERFLOW);
}

int main(void)
{
	test_partial_blocks();
	test_report_counts_errors();
	test_errors();
	CHECK_DONE();
}
