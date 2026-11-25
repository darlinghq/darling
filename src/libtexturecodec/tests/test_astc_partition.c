/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "astc_internal.h"
#include "check.h"

/* Expected values come from an independent Python transcription of the
 * pseudo-code of spec 23.20 (hash52 and select_partition). */
static void test_hash52(void)
{
	CHECK_EQ(astc_hash52(0), 0);
	CHECK_EQ(astc_hash52(1), 0x0c46f563u);
	CHECK_EQ(astc_hash52(2), 0xe6a38a50u);
	CHECK_EQ(astc_hash52(0x1234), 0x778ca0f4u);
	CHECK_EQ((uint32_t)astc_hash52(0xFFFFFFFFu), 0x052da04au);
	CHECK_EQ(astc_hash52(3 * 1024 + 77), 0x326764fdu);
}

static void check_map(uint32_t seed, uint32_t np, uint32_t bw, uint32_t bh,
		      const char *want)
{
	uint8_t m[144];
	uint32_t i;

	astc_partition_map(seed, np, bw, bh, m);
	for (i = 0; i < bw * bh; i++)
		CHECK_EQ(m[i], want[i] - '0');
}

static void test_maps(void)
{
	check_map(1, 2, 4, 4, "0110111011001100");
	check_map(0, 3, 6, 6, "200000200000200000200000211111211111");
	check_map(2, 4, 8, 8,
		  "0000000000000000233311112333311123333111233333113333333133333332");
	check_map(0, 3, 5, 4, "20001200002111121111");
	check_map(1, 4, 10, 6,
		  "002333331100233333110223333311022333331122233333112223333300");
	check_map(1, 2, 12, 12,
		  "011110000000111110000000111110000000111100000000111100000000"
		  "111000000000111000000011111000001111110000111111110011111111"
		  "101111111111101111111111");
}

static void test_single_partition_is_zero(void)
{
	uint8_t m[144];
	uint32_t i, seed;

	for (seed = 0; seed < 1024; seed += 37) {
		memset(m, 9, sizeof(m));
		astc_partition_map(seed, 1, 12, 12, m);
		for (i = 0; i < 144; i++)
			CHECK_EQ(m[i], 0);
	}
}

static void test_range(void)
{
	uint8_t m[144];
	uint32_t seed, np, i;

	for (np = 2; np <= 4; np++)
		for (seed = 0; seed < 1024; seed++) {
			astc_partition_map(seed, np, 6, 6, m);
			for (i = 0; i < 36; i++)
				CHECK(m[i] < np);
		}
}

int main(void)
{
	test_hash52();
	test_maps();
	test_single_partition_is_zero();
	test_range();
	CHECK_DONE();
}
