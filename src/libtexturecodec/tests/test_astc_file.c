/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "check.h"
#include "texturecodec/texturecodec.h"

static void make(uint8_t *h, uint8_t bw, uint8_t bh, uint8_t bd, uint32_t w,
		 uint32_t ht, uint32_t d)
{
	static const uint8_t m[4] = { 0x13, 0xAB, 0xA1, 0x5C };
	uint32_t v[3] = { w, ht, d };
	int i;

	memcpy(h, m, 4);
	h[4] = bw;
	h[5] = bh;
	h[6] = bd;
	for (i = 0; i < 3; i++) {
		h[7 + 3 * i] = (uint8_t)v[i];
		h[8 + 3 * i] = (uint8_t)(v[i] >> 8);
		h[9 + 3 * i] = (uint8_t)(v[i] >> 16);
	}
}

static void test_valid(void)
{
	uint8_t h[16];
	dtc_astc_header o;

	make(h, 8, 6, 1, 0x010203, 0xFFFFFF, 1);
	CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_OK);
	CHECK_EQ(o.block_w, 8);
	CHECK_EQ(o.block_h, 6);
	CHECK_EQ(o.block_d, 1);
	CHECK_EQ(o.width, 0x010203);
	CHECK_EQ(o.height, 0xFFFFFF);
	CHECK_EQ(o.depth, 1);
	CHECK_EQ(dtc_astc_file_parse_2d(h, 16, &o), DTC_OK);
}

static void test_errors(void)
{
	uint8_t h[16];
	dtc_astc_header o;

	make(h, 4, 4, 1, 4, 4, 1);
	CHECK_EQ(dtc_astc_file_parse(NULL, 16, &o), DTC_E_ARG);
	CHECK_EQ(dtc_astc_file_parse(h, 16, NULL), DTC_E_ARG);
	CHECK_EQ(dtc_astc_file_parse(h, 15, &o), DTC_E_TRUNCATED);
	CHECK_EQ(dtc_astc_file_parse(h, 0, &o), DTC_E_TRUNCATED);
	h[0] ^= 1;
	CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_E_CORRUPT);
	h[0] ^= 1;
	make(h, 5, 6, 1, 4, 4, 1);
	CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_E_CORRUPT);
	make(h, 0, 0, 0, 4, 4, 1);
	CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_E_CORRUPT);
	make(h, 4, 4, 1, 0, 4, 1);
	CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_E_CORRUPT);
	make(h, 4, 4, 1, 4, 0, 1);
	CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_E_CORRUPT);
	make(h, 4, 4, 1, 4, 4, 0);
	CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_E_CORRUPT);
	make(h, 4, 4, 2, 4, 4, 2);
	CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_E_CORRUPT);
}

static void test_unsupported_3d(void)
{
	uint8_t h[16];
	dtc_astc_header o;

	make(h, 4, 4, 4, 8, 8, 8);
	CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_OK);
	CHECK_EQ(dtc_astc_file_parse_2d(h, 16, &o), DTC_E_UNSUPPORTED);
	make(h, 4, 4, 1, 8, 8, 3);
	CHECK_EQ(dtc_astc_file_parse_2d(h, 16, &o), DTC_E_UNSUPPORTED);
}

static void test_3d_footprint_table(void)
{
	static const uint8_t ok[10][3] = {
		{ 3, 3, 3 }, { 4, 3, 3 }, { 4, 4, 3 }, { 4, 4, 4 }, { 5, 4, 4 },
		{ 5, 5, 4 }, { 5, 5, 5 }, { 6, 5, 5 }, { 6, 6, 5 }, { 6, 6, 6 }
	};
	uint8_t h[16];
	dtc_astc_header o;
	uint32_t x, y, z, n = 0;

	for (x = 0; x < 9; x++)
		for (y = 0; y < 9; y++)
			for (z = 2; z < 9; z++) {
				make(h, (uint8_t)x, (uint8_t)y, (uint8_t)z, 8, 8, 8);
				n += dtc_astc_file_parse(h, 16, &o) == DTC_OK;
			}
	CHECK_EQ(n, 10);
	for (x = 0; x < 10; x++) {
		make(h, ok[x][0], ok[x][1], ok[x][2], 8, 8, 8);
		CHECK_EQ(dtc_astc_file_parse(h, 16, &o), DTC_OK);
	}
}

static void test_footprints(void)
{
	static const uint8_t ok[14][2] = {
		{ 4, 4 }, { 5, 4 }, { 5, 5 }, { 6, 5 }, { 6, 6 },
		{ 8, 5 }, { 8, 6 }, { 10, 5 }, { 10, 6 }, { 8, 8 },
		{ 10, 8 }, { 10, 10 }, { 12, 10 }, { 12, 12 }
	};
	uint32_t w, h, n = 0;
	size_t i;

	for (i = 0; i < 14; i++)
		CHECK(dtc_astc_footprint_valid(ok[i][0], ok[i][1]));
	for (w = 0; w < 16; w++)
		for (h = 0; h < 16; h++)
			n += (uint32_t)dtc_astc_footprint_valid(w, h);
	CHECK_EQ(n, 14);
}

int main(void)
{
	test_valid();
	test_errors();
	test_unsupported_3d();
	test_footprints();
	test_3d_footprint_table();
	CHECK_DONE();
}
