/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * The typed allocation entry points of libsystem_malloc (malloc_type_*,
 * macOS 14; malloc_zone_malloc_with_options and its typed variant, macOS
 * 26). The type id is accepted and does not change the result.
 * The expected results were observed on macOS 27 (arm64 and x86_64).
 */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc/malloc.h>

static int failures;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

static int all_bytes(const unsigned char *p, size_t n, unsigned char v)
{
	for (size_t i = 0; i < n; i++)
		if (p[i] != v)
			return 0;
	return 1;
}

int main(void)
{
	const malloc_type_id_t tid = 0xfedcba9876543210ULL;
	malloc_zone_t *zone = malloc_default_zone();
	unsigned char *p, *q;
	void *r;
	volatile size_t huge = SIZE_MAX, odd_align = 24, zero_align = 0;

	p = malloc_type_malloc(64, tid);
	check("malloc_type_malloc", p != NULL && malloc_size(p) >= 64);
	memset(p, 0x53, 64);
	q = malloc_type_realloc(p, 4096, 1);
	check("malloc_type_realloc keeps the data", q != NULL && all_bytes(q, 64, 0x53));
	errno = 0;
	r = malloc_type_realloc(q, huge, 0);
	check("failed malloc_type_realloc gives ENOMEM", r == NULL && errno == ENOMEM);
	check("failed malloc_type_realloc keeps the block", all_bytes(q, 64, 0x53));
	malloc_type_free(q, tid);

	p = malloc_type_calloc(33, 7, UINT64_MAX);
	check("malloc_type_calloc clears", p != NULL && all_bytes(p, 231, 0));
	malloc_type_free(p, 0);
	errno = 0;
	check("malloc_type_calloc overflow gives ENOMEM", malloc_type_calloc(huge, 2, 0) == NULL && errno == ENOMEM);

	p = malloc_type_realloc(NULL, 32, 0);
	check("malloc_type_realloc(NULL) allocates", p != NULL);
	free(p);

	p = malloc_type_valloc(100, tid);
	check("malloc_type_valloc is page aligned", p != NULL && ((uintptr_t)p & 4095) == 0);
	free(p);

	p = malloc_type_aligned_alloc(256, 512, tid);
	check("malloc_type_aligned_alloc aligns", p != NULL && ((uintptr_t)p & 255) == 0);
	free(p);

	r = NULL;
	check("malloc_type_posix_memalign", malloc_type_posix_memalign(&r, 128, 100, tid) == 0 && r != NULL && ((uintptr_t)r & 127) == 0);
	free(r);
	check("malloc_type_posix_memalign rejects a bad alignment", malloc_type_posix_memalign(&r, 3, 100, tid) == EINVAL);

	p = malloc_type_zone_malloc(zone, 48, tid);
	check("malloc_type_zone_malloc", p != NULL && malloc_zone_from_ptr(p) == zone);
	p = malloc_type_zone_realloc(zone, p, 96, tid);
	check("malloc_type_zone_realloc", p != NULL);
	malloc_type_zone_free(zone, p, tid);
	p = malloc_type_zone_calloc(zone, 10, 10, tid);
	check("malloc_type_zone_calloc clears", p != NULL && all_bytes(p, 100, 0));
	malloc_type_zone_free(zone, p, tid);
	p = malloc_type_zone_valloc(zone, 10, tid);
	check("malloc_type_zone_valloc is page aligned", p != NULL && ((uintptr_t)p & 4095) == 0);
	malloc_type_zone_free(zone, p, tid);
	p = malloc_type_zone_memalign(zone, 64, 64, tid);
	check("malloc_type_zone_memalign aligns", p != NULL && ((uintptr_t)p & 63) == 0);
	malloc_type_zone_free(zone, p, tid);

	p = malloc_zone_malloc_with_options(NULL, MALLOC_ZONE_MALLOC_DEFAULT_ALIGN, 40, MALLOC_ZONE_MALLOC_OPTION_CLEAR);
	check("with_options: NULL zone, default alignment, cleared", p != NULL && all_bytes(p, 40, 0) && malloc_zone_from_ptr(p) == zone);
	free(p);
	p = malloc_zone_malloc_with_options(zone, MALLOC_ZONE_MALLOC_DEFAULT_ALIGN, 41, MALLOC_ZONE_MALLOC_OPTION_NONE);
	check("with_options: default alignment allows any size", p != NULL);
	free(p);
	p = malloc_zone_malloc_with_options(zone, 4096, 8192, MALLOC_ZONE_MALLOC_OPTION_CLEAR);
	check("with_options: 4096 alignment, cleared", p != NULL && ((uintptr_t)p & 4095) == 0 && all_bytes(p, 8192, 0));
	free(p);
	p = malloc_type_zone_malloc_with_options(zone, 64, 128, tid, MALLOC_ZONE_MALLOC_OPTION_CLEAR);
	check("typed with_options: 64 alignment, cleared", p != NULL && ((uintptr_t)p & 63) == 0 && all_bytes(p, 128, 0));
	free(p);
	check("with_options: size not a multiple of the alignment fails", malloc_zone_malloc_with_options(zone, 64, 100, 0) == NULL);
	check("with_options: alignment that is not a power of two fails", malloc_zone_malloc_with_options(zone, odd_align, 48, 0) == NULL);
	/* the header asks for a non-zero alignment; macOS treats 0 as the default */
	p = malloc_zone_malloc_with_options(zone, zero_align, 16, 0);
	check("with_options: zero alignment allocates", p != NULL);
	free(p);

	return failures != 0;
}
