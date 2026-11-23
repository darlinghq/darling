/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "astc_internal.h"
#include "blockbuild.h"
#include "check.h"

/* Walk every grid the table can express, in every precision/plane setting,
 * and require that parsing the encoded mode returns exactly those fields and
 * that the encodings are distinct. */
static void test_block_mode_bijection(void)
{
	static uint8_t used[2048];
	uint32_t w, h, rho, p, dp, bm, valid = 0, void_n = 0, reserved = 0;
	astc_block b;
	const astc_range *r;

	for (w = 2; w <= 12; w++)
		for (h = 2; h <= 12; h++)
			for (rho = 2; rho < 8; rho++)
				for (p = 0; p < 2; p++)
					for (dp = 0; dp < 2; dp++) {
						if (encode_mode(w, h, rho, p, dp, &bm))
							continue;
						CHECK(!used[bm]);
						used[bm] = 1;
						memset(&b, 0, sizeof(b));
						CHECK_EQ(astc_parse_block_mode(bm, &b), 0);
						r = astc_weight_range(p, rho);
						CHECK_EQ(b.gw, w);
						CHECK_EQ(b.gh, h);
						CHECK_EQ(b.dual, dp);
						CHECK(memcmp(&b.wrange, r, sizeof(*r)) == 0);
						valid++;
					}
	for (bm = 0; bm < 2048; bm++) {
		if ((bm & 0x1FF) == 0x1FC)
			void_n++;
		else if (astc_parse_block_mode(bm, &b))
			reserved++;
		else
			CHECK(used[bm]);
	}
	CHECK_EQ(valid, 1872);
	CHECK_EQ(void_n, 4);
	CHECK_EQ(reserved, 2048 - 1872 - 4);
}

static uint32_t seq_bits(uint32_t n, const astc_range *r)
{
	return n * r->bits + (r->trit ? (8 * n + 4) / 5 : 0) +
	       (r->quint ? (7 * n + 2) / 3 : 0);
}

/* Weight-bit legality of 23.23 pinned at both edges: for every encodable
 * grid on a 12x12 footprint with CEM 0 (two integers), the block is legal
 * exactly when it has at most 64 weights and 24 to 96 weight bits. */
static void test_weight_bit_bounds(void)
{
	uint32_t w, h, rho, p, dp, bm, nw, wb, at23 = 0, at24 = 0, at96 = 0, at97 = 0;
	uint8_t blk[16];
	astc_block b;
	int legal;

	for (w = 2; w <= 12; w++)
		for (h = 2; h <= 12; h++)
			for (rho = 2; rho < 8; rho++)
				for (p = 0; p < 2; p++)
					for (dp = 0; dp < 2; dp++) {
						if (encode_mode(w, h, rho, p, dp, &bm))
							continue;
						memset(blk, 0, 16);
						put(blk, 0, 11, bm);
						nw = w * h * (1 + dp);
						wb = seq_bits(nw, astc_weight_range(p, rho));
						legal = nw <= 64 && wb >= 24 && wb <= 96;
						CHECK_EQ(astc_parse_block(blk, 12, 12, &b) ==
							 ASTC_BLOCK_NORMAL, legal);
						at23 += nw <= 64 && wb == 23;
						at24 += nw <= 64 && wb == 24;
						at96 += nw <= 64 && wb == 96;
						at97 += nw <= 64 && wb == 97;
					}
	CHECK(at23 && at24 && at96 && at97);
}

static void block_4x4_2bit(uint8_t *b)
{
	uint32_t bm;

	memset(b, 0, 16);
	encode_mode(4, 4, 4, 0, 0, &bm);
	put(b, 0, 11, bm);
}

static void test_single_partition(void)
{
	uint8_t blk[16];
	astc_block b;

	block_4x4_2bit(blk);
	put(blk, 13, 4, 9);
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_NORMAL);
	CHECK_EQ(b.gw, 4);
	CHECK_EQ(b.nparts, 1);
	CHECK_EQ(b.cem[0], 9);
	CHECK_EQ(b.ncvals, 6);
	CHECK_EQ(b.wrange.max, 3);
	CHECK_EQ(b.cstart, 17);
	/* 128 - 17 - 32 = 79 bits for 6 values: 0..255 needs 48 */
	CHECK_EQ(b.crange.max, 255);
	put(blk, 13, 4, 12);
	astc_parse_block(blk, 4, 4, &b);
	CHECK_EQ(b.ncvals, 8);
}

static void test_dual_plane_ccs(void)
{
	uint8_t blk[16];
	astc_block b;
	uint32_t bm;

	memset(blk, 0, 16);
	encode_mode(4, 4, 4, 0, 1, &bm);
	put(blk, 0, 11, bm);
	put(blk, 62, 2, 2);
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_NORMAL);
	CHECK_EQ(b.dual, 1);
	CHECK_EQ(b.ccs, 2);
	put(blk, 62, 2, 3);
	astc_parse_block(blk, 4, 4, &b);
	CHECK_EQ(b.ccs, 3);
}

static void test_multi_partition_cem(void)
{
	uint8_t blk[16];
	astc_block b;

	block_4x4_2bit(blk);
	put(blk, 11, 2, 1);		/* two partitions */
	put(blk, 13, 10, 0x155);	/* partition index */
	put(blk, 23, 2, 1);		/* classes 0 or 1 */
	put(blk, 25, 1, 0);		/* C0 */
	put(blk, 26, 1, 1);		/* C1 */
	put(blk, 27, 2, 2);		/* M0 = 2: CEM 2 */
	put(blk, 128 - 32 - 2, 2, 3);	/* M1 = 3 below the weights: CEM 7 */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_NORMAL);
	CHECK_EQ(b.nparts, 2);
	CHECK_EQ(b.seed, 0x155);
	CHECK_EQ(b.cem[0], 2);
	CHECK_EQ(b.cem[1], 7);
	CHECK_EQ(b.ncvals, 6);
	CHECK_EQ(b.cstart, 29);
	/* the same CEM for every partition when the selector is 0 */
	put(blk, 23, 2, 0);
	put(blk, 25, 4, 8);
	astc_parse_block(blk, 4, 4, &b);
	CHECK_EQ(b.cem[0], 8);
	CHECK_EQ(b.cem[1], 8);
	CHECK_EQ(b.ncvals, 12);
}

static void test_multi_partition_dual_plane(void)
{
	uint8_t blk[16];
	astc_block b;
	uint32_t bm;

	memset(blk, 0, 16);
	encode_mode(4, 4, 4, 0, 1, &bm);	/* dual plane: 64 weight bits */
	put(blk, 0, 11, bm);
	put(blk, 11, 2, 1);
	put(blk, 23, 2, 1);	/* classes 0 or 1 */
	put(blk, 26, 1, 1);	/* partition 1 takes class 1 */
	put(blk, 62, 2, 0);	/* M1 below the weights: CEM 4 */
	put(blk, 60, 2, 3);	/* CCS below the extra CEM bits */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_NORMAL);
	CHECK_EQ(b.cem[0], 0);
	CHECK_EQ(b.cem[1], 4);
	CHECK_EQ(b.ccs, 3);
	CHECK_EQ(b.ncvals, 6);
	/* 128 - 33 - 64 = 31 bits for 6 values: 0..31 takes 30 */
	CHECK_EQ(b.crange.max, 31);
}

static void test_illegal_blocks(void)
{
	uint8_t blk[16];
	astc_block b;
	uint32_t bm;

	memset(blk, 0, 16);	/* reserved block mode */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);

	block_4x4_2bit(blk);	/* grid larger than the footprint */
	encode_mode(5, 4, 4, 0, 0, &bm);
	put(blk, 0, 11, bm);
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);
	CHECK_EQ(astc_parse_block(blk, 5, 4, &b), ASTC_BLOCK_NORMAL);

	encode_mode(4, 4, 2, 0, 0, &bm);	/* 16 weights of 1 bit: < 24 */
	put(blk, 0, 11, bm);
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);

	encode_mode(12, 5, 4, 0, 0, &bm);	/* 120 bits > 96 */
	put(blk, 0, 11, bm);
	CHECK_EQ(astc_parse_block(blk, 12, 12, &b), ASTC_BLOCK_ERROR);

	encode_mode(7, 5, 2, 0, 1, &bm);	/* 70 weights > 64, 70 bits */
	put(blk, 0, 11, bm);
	CHECK_EQ(astc_parse_block(blk, 8, 8, &b), ASTC_BLOCK_ERROR);
	encode_mode(6, 5, 2, 0, 1, &bm);	/* 60 weights is fine */
	put(blk, 0, 11, bm);
	CHECK_EQ(astc_parse_block(blk, 8, 8, &b), ASTC_BLOCK_NORMAL);

	block_4x4_2bit(blk);	/* 4 partitions with dual plane */
	encode_mode(4, 4, 4, 0, 1, &bm);
	put(blk, 0, 11, bm);
	put(blk, 11, 2, 3);
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);

	block_4x4_2bit(blk);	/* 24 colour integers > 18 */
	put(blk, 11, 2, 3);
	put(blk, 23, 2, 3);
	put(blk, 25, 4, 15);
	put(blk, 128 - 32 - 8, 8, 0xFF);
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);

	block_4x4_2bit(blk);	/* too few bits for eight values */
	encode_mode(4, 4, 7, 0, 1, &bm);
	put(blk, 0, 11, bm);
	put(blk, 13, 4, 12);
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);
	put(blk, 13, 4, 0);	/* the same block with two values is legal */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_NORMAL);
}

static void test_void_extent(void)
{
	uint8_t blk[16];
	astc_block b;

	void_block(blk);
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_VOID);
	CHECK_EQ(b.color[0], 0x1234);
	CHECK_EQ(b.color[1], 0x5678);
	CHECK_EQ(b.color[2], 0x9ABC);
	CHECK_EQ(b.color[3], 0xDEF0);
	/* a real extent is fine when min < max on both axes */
	put(blk, 12, 13, 10);
	put(blk, 25, 13, 20);
	put(blk, 38, 13, 5);
	put(blk, 51, 13, 6);
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_VOID);
	put(blk, 25, 13, 10);	/* min s == max s */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);
	put(blk, 25, 13, 20);
	put(blk, 51, 13, 4);	/* min t > max t */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);
	put(blk, 51, 13, 5);	/* min t == max t */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);
	void_block(blk);
	put(blk, 10, 1, 0);	/* reserved bit 10 cleared */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);
	void_block(blk);
	put(blk, 11, 1, 0);	/* reserved bit 11 cleared */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);
	void_block(blk);
	put(blk, 9, 1, 1);	/* HDR void extent in LDR mode */
	CHECK_EQ(astc_parse_block(blk, 4, 4, &b), ASTC_BLOCK_ERROR);
}

/* DTC_COMPAT (see parse_void): the extent and the reserved bits of a void-extent
 * block with D = 0 are not checked, as on macOS; D = 1 stays an error. */
static void test_void_extent_compat(void)
{
	uint8_t blk[16];
	astc_block b;

	void_block(blk);
	put(blk, 25, 13, 10);	/* min s == max s */
	put(blk, 12, 13, 10);
	CHECK_EQ(astc_parse_block_ex(blk, 4, 4, &b, 0), ASTC_BLOCK_ERROR);
	CHECK_EQ(astc_parse_block_ex(blk, 4, 4, &b, 1), ASTC_BLOCK_VOID);
	CHECK_EQ(b.color[3], 0xDEF0);
	put(blk, 12, 13, 30);	/* min s > max s */
	CHECK_EQ(astc_parse_block_ex(blk, 4, 4, &b, 1), ASTC_BLOCK_VOID);
	void_block(blk);
	put(blk, 10, 1, 0);	/* reserved bit 10 cleared */
	CHECK_EQ(astc_parse_block_ex(blk, 4, 4, &b, 0), ASTC_BLOCK_ERROR);
	CHECK_EQ(astc_parse_block_ex(blk, 4, 4, &b, 1), ASTC_BLOCK_VOID);
	void_block(blk);
	put(blk, 11, 1, 0);	/* reserved bit 11 cleared */
	CHECK_EQ(astc_parse_block_ex(blk, 4, 4, &b, 0), ASTC_BLOCK_ERROR);
	CHECK_EQ(astc_parse_block_ex(blk, 4, 4, &b, 1), ASTC_BLOCK_VOID);
	void_block(blk);
	put(blk, 9, 1, 1);	/* HDR void extent in LDR mode */
	CHECK_EQ(astc_parse_block_ex(blk, 4, 4, &b, 1), ASTC_BLOCK_ERROR);
}

int main(void)
{
	test_block_mode_bijection();
	test_weight_bit_bounds();
	test_single_partition();
	test_dual_plane_ccs();
	test_multi_partition_cem();
	test_multi_partition_dual_plane();
	test_illegal_blocks();
	test_void_extent();
	test_void_extent_compat();
	CHECK_DONE();
}
