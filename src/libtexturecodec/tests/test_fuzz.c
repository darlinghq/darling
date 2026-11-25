/* SPDX-License-Identifier: GPL-3.0-only */
/* Deterministic fuzz-style tests: random, mutated and truncated input must
 * never crash, read or write out of bounds, or give nondeterministic output.
 * Every buffer is a heap allocation of exactly the size the API promises so
 * AddressSanitizer reports any overrun. Iteration count: DTC_FUZZ_ITERS. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astc_internal.h"
#include "check.h"
#include "texturecodec/texturecodec.h"

static const uint8_t foot[14][2] = {
	{ 4, 4 }, { 5, 4 }, { 5, 5 }, { 6, 5 }, { 6, 6 }, { 8, 5 }, { 8, 6 },
	{ 10, 5 }, { 10, 6 }, { 8, 8 }, { 10, 8 }, { 10, 10 }, { 12, 10 },
	{ 12, 12 }
};

static uint64_t rng_state = 0x9E3779B97F4A7C15ull;

static uint64_t rnd(void)
{
	rng_state ^= rng_state >> 12;
	rng_state ^= rng_state << 25;
	rng_state ^= rng_state >> 27;
	return rng_state * 0x2545F4914F6CDD1Dull;
}

static void rnd_bytes(uint8_t *p, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++)
		p[i] = (uint8_t)(rnd() >> 24);
}

/* Decodes one block twice into exact-size buffers and once more through the
 * image API, and checks the invariants that hold for every input. */
static void one_block(const uint8_t *src, uint32_t bw, uint32_t bh, int srgb)
{
	size_t bytes = (size_t)bw * bh * 4;
	uint8_t *blk = malloc(16), *a = malloc(bytes), *b = malloc(bytes);
	uint8_t *c = malloc(bytes);
	dtc_decode_report rep;
	int e1 = -1, e2 = -1;

	memcpy(blk, src, 16);
	CHECK_EQ(dtc_astc_decode_block(blk, bw, bh, srgb, a, &e1), DTC_OK);
	CHECK_EQ(dtc_astc_decode_block(blk, bw, bh, srgb, b, &e2), DTC_OK);
	CHECK(e1 == e2 && (e1 == 0 || e1 == 1));
	CHECK(!memcmp(a, b, bytes));
	CHECK_EQ(dtc_astc_decode_ex(blk, 16, bw, bh, bw, bh, srgb, c, &rep),
		 DTC_OK);
	CHECK(!memcmp(a, c, bytes));
	CHECK_EQ(rep.error_blocks, (uint64_t)e1);
	free(blk);
	free(a);
	free(b);
	free(c);
}

static uint64_t kinds[3];

static void test_random_blocks(uint32_t iters)
{
	uint8_t blk[16];
	astc_block b;
	uint32_t f, n, s;

	for (f = 0; f < 14; f++)
		for (s = 0; s < 2; s++)
			for (n = 0; n < iters; n++) {
				rnd_bytes(blk, 16);
				kinds[astc_parse_block(blk, foot[f][0],
						       foot[f][1], &b)]++;
				one_block(blk, foot[f][0], foot[f][1], (int)s);
			}
}

/* Every 11-bit block mode with every partition count and random remaining
 * bits: reaches all configuration branches, legal and illegal. */
static void test_all_block_modes(uint32_t reps)
{
	uint8_t blk[16];
	uint32_t bm, np, r, f;

	for (bm = 0; bm < 2048; bm++)
		for (np = 0; np < 4; np++)
			for (r = 0; r < reps; r++) {
				rnd_bytes(blk, 16);
				blk[0] = (uint8_t)bm;
				blk[1] = (uint8_t)((blk[1] & ~0x1Fu) | (bm >> 8) |
						   (np << 3));
				f = (uint32_t)(rnd() % 14);
				one_block(blk, foot[f][0], foot[f][1], (int)(r & 1));
			}
}

static void test_bit_flips_of_valid_blocks(uint32_t iters)
{
	/* a legal block: 4x4 grid of 2-bit weights, luminance endpoints */
	uint8_t base[16] = { 0x42, 0, 0, 0, 0x1E, 0x5A, 0, 0, 0, 0, 0, 0, 0x1B,
			     0xE4, 0x1B, 0xE4 };
	uint8_t blk[16];
	uint32_t n, k, f;

	for (n = 0; n < iters * 4; n++) {
		memcpy(blk, base, 16);
		for (k = (uint32_t)(rnd() % 4) + 1; k > 0; k--)
			blk[rnd() % 16] ^= (uint8_t)(1u << (rnd() % 8));
		f = (uint32_t)(rnd() % 14);
		one_block(blk, foot[f][0], foot[f][1], (int)(n & 1));
	}
}

static void test_fixed_patterns(void)
{
	uint8_t blk[16];
	uint32_t f, v;

	for (f = 0; f < 14; f++)
		for (v = 0; v < 256; v += 51) {
			memset(blk, (int)v, 16);
			one_block(blk, foot[f][0], foot[f][1], 0);
			one_block(blk, foot[f][0], foot[f][1], 1);
		}
}

/* Images whose payload is a random, usually too short, exact-size buffer. */
static void test_truncated_images(uint32_t iters)
{
	uint32_t n, bw, bh, w, h;
	size_t len, need;
	uint8_t *data, *out;
	dtc_decode_report rep;
	dtc_status st;

	for (n = 0; n < iters; n++) {
		uint32_t f = (uint32_t)(rnd() % 14);

		bw = foot[f][0];
		bh = foot[f][1];
		w = (uint32_t)(rnd() % 70) + 1;
		h = (uint32_t)(rnd() % 70) + 1;
		need = (size_t)((w + bw - 1) / bw) * ((h + bh - 1) / bh) * 16;
		len = (size_t)(rnd() % (need + 40));
		data = malloc(len ? len : 1);
		out = malloc((size_t)w * h * 4);
		rnd_bytes(data, len);
		st = dtc_astc_decode_ex(data, len, bw, bh, w, h, (int)(n & 1),
					out, &rep);
		CHECK_EQ(st, len >= need ? DTC_OK : DTC_E_TRUNCATED);
		if (st == DTC_OK)
			CHECK_EQ(rep.blocks, need / 16);
		free(data);
		free(out);
	}
}

static void test_header_fuzz(uint32_t iters)
{
	static const uint8_t magic[4] = { 0x13, 0xAB, 0xA1, 0x5C };
	uint32_t n;

	for (n = 0; n < iters * 4; n++) {
		size_t len = (size_t)(rnd() % 24);
		uint8_t *h = malloc(len ? len : 1);
		dtc_astc_header out;
		dtc_status st;

		rnd_bytes(h, len);
		if (len >= 4 && (rnd() & 1))
			memcpy(h, magic, 4);
		st = dtc_astc_file_parse(h, len, &out);
		CHECK(st == DTC_OK || st == DTC_E_TRUNCATED || st == DTC_E_CORRUPT);
		if (st == DTC_OK)
			CHECK(out.width && out.height && out.depth);
		(void)dtc_astc_file_parse_2d(h, len, &out);
		free(h);
	}
}

int main(void)
{
	const char *e = getenv("DTC_FUZZ_ITERS");
	uint32_t iters = e ? (uint32_t)strtoul(e, NULL, 10) : 20000;

	test_fixed_patterns();
	test_random_blocks(iters);
	test_all_block_modes(iters / 500 + 1);
	test_bit_flips_of_valid_blocks(iters);
	test_truncated_images(iters);
	test_header_fuzz(iters);
	printf("iterations per footprint and profile: %u\n", iters);
	printf("random blocks: normal %llu, void-extent %llu, illegal %llu\n",
	       (unsigned long long)kinds[ASTC_BLOCK_NORMAL],
	       (unsigned long long)kinds[ASTC_BLOCK_VOID],
	       (unsigned long long)kinds[ASTC_BLOCK_ERROR]);
	CHECK_DONE();
}
