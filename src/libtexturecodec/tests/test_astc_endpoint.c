/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "astc_internal.h"
#include "blockbuild.h"
#include "check.h"

/* Builds a block whose colour integers use the 8-bit range, so the stored
 * bytes are the unquantized values, and decodes partition 0 (and 1). */
static uint32_t endpoints(const uint8_t *vals, uint32_t n, const uint8_t *cems,
			  uint32_t np, int ep[4][2][4])
{
	uint8_t blk[16];
	astc_block b;
	uint32_t i;

	memset(blk, 0, sizeof(blk));
	memset(&b, 0, sizeof(b));
	for (i = 0; i < n; i++)
		put(blk, 17 + 8 * i, 8, vals[i]);
	b.nparts = (uint8_t)np;
	for (i = 0; i < np; i++)
		b.cem[i] = cems[i];
	b.ncvals = (uint8_t)n;
	b.cstart = 17;
	b.crange.bits = 8;
	b.crange.max = 255;
	return astc_decode_endpoints(blk, &b, ep);
}

static void expect(int ep[2][4], int r0, int g0, int b0, int a0, int r1,
		   int g1, int b1, int a1)
{
	CHECK_EQ(ep[0][0], r0);
	CHECK_EQ(ep[0][1], g0);
	CHECK_EQ(ep[0][2], b0);
	CHECK_EQ(ep[0][3], a0);
	CHECK_EQ(ep[1][0], r1);
	CHECK_EQ(ep[1][1], g1);
	CHECK_EQ(ep[1][2], b1);
	CHECK_EQ(ep[1][3], a1);
}

static void one(uint8_t cem, const uint8_t *v, uint32_t n, int ep[4][2][4])
{
	CHECK_EQ(endpoints(v, n, &cem, 1, ep), 0);
}

/* Every expectation below was computed by hand from section 23.14.1. */
static void test_luminance(void)
{
	int ep[4][2][4];
	uint8_t v0[] = { 10, 200 }, v1[] = { 0x45, 0xC5 }, v1b[] = { 0x45, 0xFF };
	uint8_t v4[] = { 10, 20, 30, 40 };
	uint8_t v5[] = { 100, 0x83, 50, 0x41 };

	one(0, v0, 2, ep);
	expect(ep[0], 10, 10, 10, 255, 200, 200, 200, 255);
	one(1, v1, 2, ep);
	expect(ep[0], 209, 209, 209, 255, 214, 214, 214, 255);
	one(1, v1b, 2, ep);
	expect(ep[0], 209, 209, 209, 255, 255, 255, 255, 255);
	one(4, v4, 4, ep);
	expect(ep[0], 10, 10, 10, 30, 20, 20, 20, 40);
	one(5, v5, 4, ep);
	expect(ep[0], 178, 178, 178, 25, 179, 179, 179, 0);
}

static void test_scale_modes(void)
{
	int ep[4][2][4];
	uint8_t v6[] = { 200, 100, 50, 128 };
	uint8_t v10[] = { 200, 100, 50, 128, 7, 9 };

	one(6, v6, 4, ep);
	expect(ep[0], 100, 50, 25, 255, 200, 100, 50, 255);
	one(10, v10, 6, ep);
	expect(ep[0], 100, 50, 25, 7, 200, 100, 50, 9);
}

static void test_direct_modes(void)
{
	int ep[4][2][4];
	uint8_t d8[] = { 10, 20, 30, 40, 50, 60 };
	uint8_t c8[] = { 200, 20, 150, 40, 100, 60 };
	uint8_t d12[] = { 10, 20, 30, 40, 50, 60, 70, 80 };
	uint8_t c12[] = { 200, 20, 150, 40, 100, 60, 5, 250 };

	one(8, d8, 6, ep);
	expect(ep[0], 10, 30, 50, 255, 20, 40, 60, 255);
	one(8, c8, 6, ep);	/* blue contraction, endpoints swap */
	expect(ep[0], 40, 50, 60, 255, 150, 125, 100, 255);
	one(12, d12, 8, ep);
	expect(ep[0], 10, 30, 50, 70, 20, 40, 60, 80);
	one(12, c12, 8, ep);
	expect(ep[0], 40, 50, 60, 250, 150, 125, 100, 5);
}

static void test_offset_modes(void)
{
	int ep[4][2][4];
	uint8_t pos9[] = { 100, 0x3C, 100, 0x3C, 100, 0x3C };
	uint8_t neg9[] = { 100, 0x7E, 100, 0x7E, 200, 0x7E };
	uint8_t neg13[] = { 100, 0x7E, 100, 0x7E, 200, 0x7E, 100, 0x3C };

	one(9, pos9, 6, ep);
	expect(ep[0], 50, 50, 50, 255, 80, 80, 80, 255);
	one(9, neg9, 6, ep);
	expect(ep[0], 74, 74, 99, 255, 75, 75, 100, 255);
	one(13, neg13, 8, ep);
	expect(ep[0], 74, 74, 99, 80, 75, 75, 100, 50);
}

static void test_hdr_modes_are_flagged(void)
{
	static const uint8_t hdr[] = { 2, 3, 7, 11, 14, 15 };
	static const uint8_t ldr[] = { 0, 1, 4, 5, 6, 8, 9, 10, 12, 13 };
	uint8_t v[8] = { 0 };
	int ep[4][2][4];
	uint32_t i;

	for (i = 0; i < sizeof(hdr); i++)
		CHECK_EQ(endpoints(v, ((uint32_t)(hdr[i] >> 2) + 1) * 2, &hdr[i], 1, ep), 1);
	for (i = 0; i < sizeof(ldr); i++)
		CHECK_EQ(endpoints(v, ((uint32_t)(ldr[i] >> 2) + 1) * 2, &ldr[i], 1, ep), 0);
}

static void test_two_partitions(void)
{
	int ep[4][2][4];
	uint8_t cems[2] = { 0, 7 };
	uint8_t v[] = { 5, 6, 1, 2, 3, 4 };

	CHECK_EQ(endpoints(v, 6, cems, 2, ep), 2);	/* partition 1 is HDR */
	expect(ep[0], 5, 5, 5, 255, 6, 6, 6, 255);
	cems[1] = 4;
	CHECK_EQ(endpoints(v, 6, cems, 2, ep), 0);
	expect(ep[1], 1, 1, 1, 3, 2, 2, 2, 4);
}

int main(void)
{
	test_luminance();
	test_scale_modes();
	test_direct_modes();
	test_offset_modes();
	test_hdr_modes_are_flagged();
	test_two_partitions();
	CHECK_DONE();
}
