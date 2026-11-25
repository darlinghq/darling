/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astc_internal.h"
#include "check.h"
#include "texturecodec/texturecodec.h"

/* DTC_VECTOR_DIR overrides the directories, to run a larger scratch set. */
static const char *default_dirs[] = { "tests/vectors", "tests/vectors-random" };

typedef struct coverage {
	uint32_t kind[3];
	uint32_t parts[5];
	uint32_t dual;
	uint32_t cem[16];
	uint32_t wtrit, wquint, wbits;
	uint32_t ctrit, cquint, cbits;
	uint32_t wmax[256], cmax[256];
} coverage;

static void *slurp(const char *path, size_t *len)
{
	FILE *f = fopen(path, "rb");
	void *buf;
	long n;

	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	n = ftell(f);
	fseek(f, 0, SEEK_SET);
	buf = malloc((size_t)n + 1);
	if (buf && fread(buf, 1, (size_t)n, f) != (size_t)n) {
		free(buf);
		buf = NULL;
	}
	fclose(f);
	*len = (size_t)n;
	return buf;
}

static void note_blocks(const uint8_t *data, size_t nblocks, uint32_t bw,
			uint32_t bh, coverage *cv)
{
	size_t i;
	uint32_t k;
	astc_block b;

	for (i = 0; i < nblocks; i++) {
		cv->kind[astc_parse_block(data + 16 * i, bw, bh, &b)]++;
		if (b.kind != ASTC_BLOCK_NORMAL)
			continue;
		cv->parts[b.nparts]++;
		cv->dual += b.dual;
		for (k = 0; k < b.nparts; k++)
			cv->cem[b.cem[k]]++;
		cv->wtrit += b.wrange.trit;
		cv->wquint += b.wrange.quint;
		cv->wbits += !b.wrange.trit && !b.wrange.quint;
		cv->ctrit += b.crange.trit;
		cv->cquint += b.crange.quint;
		cv->cbits += !b.crange.trit && !b.crange.quint;
		cv->wmax[b.wrange.max]++;
		cv->cmax[b.crange.max]++;
	}
}

static uint64_t fnv1a64(const uint8_t *p, size_t n)
{
	uint64_t h = 0xCBF29CE484222325ull;
	size_t i;

	for (i = 0; i < n; i++)
		h = (h ^ p[i]) * 0x100000001B3ull;
	return h;
}

static int compare(const char *name, const uint8_t *got, const uint8_t *want,
		   uint32_t w, uint32_t h)
{
	size_t i, n = (size_t)w * h * 4;
	int bad = 0;

	for (i = 0; i < n; i++)
		if (got[i] != want[i]) {
			if (!bad)
				printf("  %s first diff at texel (%zu,%zu) ch %zu: got %u want %u\n",
				       name, i / 4 % w, i / 4 / w, i % 4,
				       got[i], want[i]);
			bad++;
		}
	return bad;
}

typedef struct vector {
	char name[128];
	uint32_t bw, bh, w, h;
	int srgb;
	uint64_t hash;	/* FNV-1a of the reference RGBA, 0 when not listed */
	int random_blocks;	/* contains illegal blocks on purpose */
} vector;

static void check_output(const vector *v, const uint8_t *out, const char *dir)
{
	char path[512];
	size_t rlen, n = (size_t)v->w * v->h * 4;
	uint8_t *ref;
	int bad = 0;

	CHECK(v->hash != 0);
	if (v->hash)
		CHECK(fnv1a64(out, n) == v->hash);
	snprintf(path, sizeof(path), "%s/%s.rgba", dir, v->name);
	ref = slurp(path, &rlen);
	if (!ref)
		return;	/* hash-only vector */
	CHECK_EQ(rlen, n);
	if (rlen == n)
		bad = compare(v->name, out, ref, v->w, v->h);
	CHECK_EQ(bad, 0);
	free(ref);
}

static void run_vector(const vector *v, const char *dir, coverage *cv)
{
	char path[512];
	size_t alen;
	uint8_t *astc, *out;
	dtc_astc_header hd;
	dtc_decode_report rep;

	snprintf(path, sizeof(path), "%s/%s.astc", dir, v->name);
	astc = slurp(path, &alen);
	CHECK(astc != NULL);
	if (!astc)
		return;
	CHECK_EQ(dtc_astc_file_parse_2d(astc, alen, &hd), DTC_OK);
	CHECK(hd.block_w == v->bw && hd.block_h == v->bh && hd.width == v->w &&
	      hd.height == v->h);
	out = malloc((size_t)v->w * v->h * 4);
	CHECK_EQ(dtc_astc_decode_ex(astc + 16, alen - 16, v->bw, v->bh, v->w,
				    v->h, v->srgb, out, &rep), DTC_OK);
	check_output(v, out, dir);
	if (!v->random_blocks)
		CHECK_EQ(rep.error_blocks, 0);	/* an encoder emits none */
	note_blocks(astc + 16, (alen - 16) / 16, v->bw, v->bh, cv);
	free(out);
	free(astc);
}

/* name, WxH, width, height, srgb, two argument columns, fnv1a64 */
static int parse_line(char *line, vector *v)
{
	char *f[8];
	int n = 0;

	for (f[n++] = line; n < 8 && (line = strchr(line, '\t')); f[n++] = ++line)
		*line = 0;
	if (n != 8 || sscanf(f[1], "%ux%u", &v->bw, &v->bh) != 2)
		return -1;
	snprintf(v->name, sizeof(v->name), "%s", f[0]);
	v->w = (uint32_t)strtoul(f[2], NULL, 10);
	v->h = (uint32_t)strtoul(f[3], NULL, 10);
	v->srgb = atoi(f[4]);
	v->hash = strtoull(f[7], NULL, 16);
	v->random_blocks = !strcmp(f[5], "random-blocks");
	return 0;
}

static void report_coverage(const coverage *cv, unsigned nvec)
{
	int i;

	printf("%u vectors; blocks: normal %u, void %u, error %u\n", nvec,
	       cv->kind[ASTC_BLOCK_NORMAL], cv->kind[ASTC_BLOCK_VOID],
	       cv->kind[ASTC_BLOCK_ERROR]);
	printf("partitions 1..4: %u %u %u %u; dual-plane %u\n", cv->parts[1],
	       cv->parts[2], cv->parts[3], cv->parts[4], cv->dual);
	printf("weights trit/quint/bits: %u %u %u; colours trit/quint/bits: %u %u %u\n",
	       cv->wtrit, cv->wquint, cv->wbits, cv->ctrit, cv->cquint, cv->cbits);
	printf("weight ranges 0..N seen:");
	for (i = 0; i < 256; i++)
		if (cv->wmax[i])
			printf(" %d", i);
	printf("\ncolour ranges 0..N seen:");
	for (i = 0; i < 256; i++)
		if (cv->cmax[i])
			printf(" %d", i);
	printf("\ncem uses:");
	for (i = 0; i < 16; i++)
		printf(" %d:%u", i, cv->cem[i]);
	printf("\n");
}

static unsigned run_dir(const char *dir, coverage *cv)
{
	char tsv_path[512];
	char *tsv, *line, *eol;
	size_t tlen;
	unsigned nvec = 0;

	snprintf(tsv_path, sizeof(tsv_path), "%s/vectors.tsv", dir);
	tsv = slurp(tsv_path, &tlen);
	CHECK(tsv != NULL);
	if (!tsv)
		return 0;
	tsv[tlen] = 0;
	for (line = tsv; *line; line = eol + 1) {
		vector v;
		int n;

		eol = strchr(line, '\n');
		if (!eol)
			eol = line + strlen(line) - 1;
		else
			*eol = 0;
		if (!strncmp(line, "name\t", 5))
			continue;
		n = parse_line(line, &v);
		CHECK_EQ(n, 0);
		if (n)
			continue;
		run_vector(&v, dir, cv);
		nvec++;
	}
	free(tsv);
	return nvec;
}

int main(void)
{
	const char *env = getenv("DTC_VECTOR_DIR");
	coverage cv;
	unsigned nvec = 0;
	size_t i;

	memset(&cv, 0, sizeof(cv));
	if (env)
		nvec = run_dir(env, &cv);
	else
		for (i = 0; i < sizeof(default_dirs) / sizeof(default_dirs[0]); i++)
			nvec += run_dir(default_dirs[i], &cv);
	report_coverage(&cv, nvec);
	CHECK_DONE();
}
