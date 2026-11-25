/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "astc_internal.h"
#include "check.h"

static const astc_range trit0 = { 1, 0, 0, 2 };
static const astc_range quint0 = { 0, 1, 0, 4 };

/* Every one of the 243 trit tuples must be reachable from the 256 packed
 * values, and every packed value must decode to digits 0..2. */
static void test_trit_surjective(void)
{
	static uint8_t seen[243];
	uint8_t buf[16], v[5];
	uint32_t T, i, code, n = 0;

	for (T = 0; T < 256; T++) {
		memset(buf, 0, sizeof(buf));
		buf[0] = (uint8_t)T;
		astc_ise_decode(buf, 0, 5, &trit0, v);
		code = 0;
		for (i = 0; i < 5; i++) {
			CHECK(v[i] <= 2);
			code = code * 3 + v[i];
		}
		if (code < 243)
			seen[code] = 1;
	}
	for (i = 0; i < 243; i++)
		n += seen[i];
	CHECK_EQ(n, 243);
}

static void test_quint_surjective(void)
{
	static uint8_t seen[125];
	uint8_t buf[16], v[3];
	uint32_t Q, i, code, n = 0;

	for (Q = 0; Q < 128; Q++) {
		memset(buf, 0, sizeof(buf));
		buf[0] = (uint8_t)Q;
		astc_ise_decode(buf, 0, 3, &quint0, v);
		code = 0;
		for (i = 0; i < 3; i++) {
			CHECK(v[i] <= 4);
			code = code * 5 + v[i];
		}
		if (code < 125)
			seen[code] = 1;
	}
	for (i = 0; i < 125; i++)
		n += seen[i];
	CHECK_EQ(n, 125);
}

/* Hand-derived from the pseudo-code of section 23.12. */
static void test_known_packings(void)
{
	uint8_t buf[16] = { 0xFF }, v[5];

	astc_ise_decode(buf, 0, 5, &trit0, v);
	CHECK_EQ(v[0], 2);
	CHECK_EQ(v[1], 1);
	CHECK_EQ(v[2], 2);
	CHECK_EQ(v[3], 2);
	CHECK_EQ(v[4], 2);
	buf[0] = 0;
	astc_ise_decode(buf, 0, 5, &trit0, v);
	CHECK(!v[0] && !v[1] && !v[2] && !v[3] && !v[4]);
	buf[0] = 0x7F;
	astc_ise_decode(buf, 0, 3, &quint0, v);
	CHECK_EQ(v[0], 1);
	CHECK_EQ(v[1], 3);
	CHECK_EQ(v[2], 4);
	buf[0] = 6;
	astc_ise_decode(buf, 0, 3, &quint0, v);
	CHECK_EQ(v[0], 4);
	CHECK_EQ(v[1], 4);
	CHECK_EQ(v[2], 0);
}

static void put(uint8_t *buf, uint32_t pos, uint32_t n, uint32_t v)
{
	uint32_t i;

	for (i = 0; i < n; i++)
		buf[(pos + i) >> 3] |= (uint8_t)(((v >> i) & 1) << ((pos + i) & 7));
}

/* Field positions from the drawings of tables 169 (trits, n = 4) and
 * 170/171 (quints, n = 4). */
static void test_field_layout(void)
{
	static const astc_range tr4 = { 1, 0, 4, 47 };
	static const astc_range qu4 = { 0, 1, 4, 79 };
	static const uint32_t m[5] = { 3, 9, 5, 12, 15 };
	uint8_t buf[16] = { 0 }, v[5];
	uint32_t i;

	put(buf, 0, 4, m[0]);
	put(buf, 4, 2, 3);
	put(buf, 6, 4, m[1]);
	put(buf, 10, 2, 3);
	put(buf, 12, 4, m[2]);
	put(buf, 16, 1, 1);
	put(buf, 17, 4, m[3]);
	put(buf, 21, 2, 3);
	put(buf, 23, 4, m[4]);
	put(buf, 27, 1, 1);
	astc_ise_decode(buf, 0, 5, &tr4, v);
	for (i = 0; i < 5; i++)
		CHECK_EQ(v[i] & 15, m[i]);
	CHECK_EQ(v[0] >> 4, 2);
	CHECK_EQ(v[4] >> 4, 2);
	memset(buf, 0, sizeof(buf));
	put(buf, 0, 4, m[0]);
	put(buf, 4, 3, 5);
	put(buf, 7, 4, m[1]);
	put(buf, 11, 2, 0);
	put(buf, 13, 4, m[2]);
	put(buf, 17, 2, 0);
	astc_ise_decode(buf, 0, 3, &qu4, v);
	for (i = 0; i < 3; i++)
		CHECK_EQ(v[i] & 15, m[i]);
}

static void test_start_offset_and_padding(void)
{
	static const astc_range tr1 = { 1, 0, 1, 5 };
	uint8_t buf[16], v[2];

	memset(buf, 0xFF, sizeof(buf));
	/* two values occupy 2 + 4 = 6 bits; trit bit 4 and above read as 0 */
	astc_ise_decode(buf, 13, 2, &tr1, v);
	CHECK_EQ(astc_ise_size(2, &tr1), 6);
	CHECK_EQ(v[0] & 1, 1);
	CHECK_EQ(v[1] & 1, 1);
	/* starting inside the last byte: bits past 127 are zero */
	astc_ise_decode(buf, 126, 2, &tr1, v);
	CHECK(v[0] <= 5 && v[1] <= 5);
}

static void test_sizes(void)
{
	static const astc_range tr = { 1, 0, 0, 2 };
	static const astc_range qu = { 0, 1, 0, 4 };
	static const astc_range b3 = { 0, 0, 3, 7 };

	CHECK_EQ(astc_ise_size(5, &tr), 8);
	CHECK_EQ(astc_ise_size(1, &tr), 2);
	CHECK_EQ(astc_ise_size(2, &tr), 4);
	CHECK_EQ(astc_ise_size(3, &tr), 5);
	CHECK_EQ(astc_ise_size(4, &tr), 7);
	CHECK_EQ(astc_ise_size(3, &qu), 7);
	CHECK_EQ(astc_ise_size(1, &qu), 3);
	CHECK_EQ(astc_ise_size(2, &qu), 5);
	CHECK_EQ(astc_ise_size(10, &b3), 30);
}

static int cmp(const void *a, const void *b)
{
	return *(const int *)a - *(const int *)b;
}

/* The decoded set of a range must be max+1 distinct, evenly spread values
 * covering 0..top; a transcription error in the tables breaks this. */
static void spread_check(const astc_range *r, int top, int tol, int weight)
{
	int vals[256], n = r->max + 1, i, gmin = 1000, gmax = 0;

	for (i = 0; i < n; i++)
		vals[i] = (int)(weight ? astc_unquant_weight(r, (uint32_t)i)
				       : astc_unquant_color(r, (uint32_t)i));
	qsort(vals, (size_t)n, sizeof(int), cmp);
	CHECK_EQ(vals[0], 0);
	CHECK_EQ(vals[n - 1], top);
	for (i = 1; i < n; i++) {
		int g = vals[i] - vals[i - 1];

		CHECK(g > 0);
		gmin = g < gmin ? g : gmin;
		gmax = g > gmax ? g : gmax;
	}
	CHECK(gmax - gmin <= tol);
}

static void test_unquant_colors(void)
{
	int i;

	for (i = 0; i < ASTC_NUM_COLOR_RANGES; i++)
		spread_check(&astc_color_ranges[i], 255, 3, 0);
	CHECK_EQ(astc_unquant_color(&astc_color_ranges[16], 77), 77);
	/* 3-bit value abc becomes abcabcab */
	CHECK_EQ(astc_unquant_color(&astc_color_ranges[1], 5), 0xB6);
	CHECK_EQ(astc_unquant_color(&astc_color_ranges[1], 7), 255);
}

static void test_unquant_weights(void)
{
	uint32_t p, rho, i;
	const astc_range *r;
	static const astc_range tr = { 1, 0, 0, 2 };
	static const astc_range qu = { 0, 1, 0, 4 };
	static const uint8_t t_exp[3] = { 0, 32, 64 };
	static const uint8_t q_exp[5] = { 0, 16, 32, 48, 64 };

	for (p = 0; p < 2; p++)
		for (rho = 2; rho < 8; rho++) {
			r = astc_weight_range(p, rho);
			CHECK(r != NULL);
			spread_check(r, 64, 3, 1);
		}
	for (i = 0; i < 3; i++)
		CHECK_EQ(astc_unquant_weight(&tr, i), t_exp[i]);
	for (i = 0; i < 5; i++)
		CHECK_EQ(astc_unquant_weight(&qu, i), q_exp[i]);
	CHECK(astc_weight_range(0, 0) == NULL);
	CHECK(astc_weight_range(1, 1) == NULL);
	CHECK(astc_weight_range(0, 8) == NULL);
	r = astc_weight_range(0, 2);
	CHECK_EQ(astc_unquant_weight(r, 0), 0);
	CHECK_EQ(astc_unquant_weight(r, 1), 64);
}

int main(void)
{
	test_trit_surjective();
	test_quint_surjective();
	test_known_packings();
	test_field_layout();
	test_start_offset_and_padding();
	test_sizes();
	test_unquant_colors();
	test_unquant_weights();
	CHECK_DONE();
}
