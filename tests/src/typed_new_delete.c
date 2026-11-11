/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * The typed operator new/delete overloads (std::__type_descriptor_t) that
 * libc++abi exports on macOS 26 and later, called through their mangled
 * names. Link with -lc++. x86_64 only (the names encode size_t as "m").
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef unsigned long long td_t;
typedef size_t align_t;
struct nothrow_t { char unused; };
static const struct nothrow_t nt;

#define NEW(name, ...) extern void *_Z##name(__VA_ARGS__) __asm__("__Z" #name)
#define DEL(name, ...) extern void _Z##name(__VA_ARGS__) __asm__("__Z" #name)

NEW(nwmSt19__type_descriptor_t, size_t, td_t);
NEW(nwmSt19__type_descriptor_tRKSt9nothrow_t, size_t, td_t, const struct nothrow_t *);
NEW(nwmSt19__type_descriptor_tSt11align_val_t, size_t, td_t, align_t);
NEW(nwmSt19__type_descriptor_tSt11align_val_tRKSt9nothrow_t, size_t, td_t, align_t, const struct nothrow_t *);
NEW(namSt19__type_descriptor_t, size_t, td_t);
NEW(namSt19__type_descriptor_tRKSt9nothrow_t, size_t, td_t, const struct nothrow_t *);
NEW(namSt19__type_descriptor_tSt11align_val_t, size_t, td_t, align_t);
NEW(namSt19__type_descriptor_tSt11align_val_tRKSt9nothrow_t, size_t, td_t, align_t, const struct nothrow_t *);
DEL(dlPvSt19__type_descriptor_t, void *, td_t);
DEL(dlPvSt19__type_descriptor_tm, void *, td_t, size_t);
DEL(dlPvSt19__type_descriptor_tmSt11align_val_t, void *, td_t, size_t, align_t);
DEL(dlPvSt19__type_descriptor_tRKSt9nothrow_t, void *, td_t, const struct nothrow_t *);
DEL(dlPvSt19__type_descriptor_tSt11align_val_t, void *, td_t, align_t);
DEL(dlPvSt19__type_descriptor_tSt11align_val_tRKSt9nothrow_t, void *, td_t, align_t, const struct nothrow_t *);
DEL(daPvSt19__type_descriptor_t, void *, td_t);
DEL(daPvSt19__type_descriptor_tm, void *, td_t, size_t);
DEL(daPvSt19__type_descriptor_tmSt11align_val_t, void *, td_t, size_t, align_t);
DEL(daPvSt19__type_descriptor_tRKSt9nothrow_t, void *, td_t, const struct nothrow_t *);
DEL(daPvSt19__type_descriptor_tSt11align_val_t, void *, td_t, align_t);
DEL(daPvSt19__type_descriptor_tSt11align_val_tRKSt9nothrow_t, void *, td_t, align_t, const struct nothrow_t *);

static int failures;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

#define ALIGNED(p, a) ((p) != NULL && ((uintptr_t)(p) & ((a) - 1)) == 0)

int main(void)
{
	const td_t td = 0x123456789abcdef0ULL;
	void *p;

	p = _ZnwmSt19__type_descriptor_t(24, td);
	check("new", p != NULL);
	memset(p, 1, 24);
	_ZdlPvSt19__type_descriptor_t(p, td);

	p = _ZnwmSt19__type_descriptor_t(0, td);
	check("new of size 0 gives a unique pointer", p != NULL);
	_ZdlPvSt19__type_descriptor_tm(p, td, 0);

	p = _ZnwmSt19__type_descriptor_tRKSt9nothrow_t(32, td, &nt);
	check("new nothrow", p != NULL);
	_ZdlPvSt19__type_descriptor_tRKSt9nothrow_t(p, td, &nt);
	check("new nothrow returns NULL on failure", _ZnwmSt19__type_descriptor_tRKSt9nothrow_t(SIZE_MAX, td, &nt) == NULL);

	p = _ZnwmSt19__type_descriptor_tSt11align_val_t(100, td, 256);
	check("new aligned", ALIGNED(p, 256));
	_ZdlPvSt19__type_descriptor_tSt11align_val_t(p, td, 256);

	p = _ZnwmSt19__type_descriptor_tSt11align_val_tRKSt9nothrow_t(100, td, 64, &nt);
	check("new aligned nothrow", ALIGNED(p, 64));
	_ZdlPvSt19__type_descriptor_tmSt11align_val_t(p, td, 100, 64);
	check("new aligned nothrow returns NULL on failure",
	      _ZnwmSt19__type_descriptor_tSt11align_val_tRKSt9nothrow_t(SIZE_MAX - 4096, td, 64, &nt) == NULL);

	p = _ZnwmSt19__type_descriptor_tSt11align_val_t(8, td, 128);
	check("new aligned (freed with the nothrow delete)", ALIGNED(p, 128));
	_ZdlPvSt19__type_descriptor_tSt11align_val_tRKSt9nothrow_t(p, td, 128, &nt);

	p = _ZnamSt19__type_descriptor_t(40, td);
	check("new[]", p != NULL);
	_ZdaPvSt19__type_descriptor_t(p, td);

	p = _ZnamSt19__type_descriptor_tRKSt9nothrow_t(40, td, &nt);
	check("new[] nothrow", p != NULL);
	_ZdaPvSt19__type_descriptor_tRKSt9nothrow_t(p, td, &nt);

	p = _ZnamSt19__type_descriptor_t(40, td);
	check("new[] (freed with the sized delete[])", p != NULL);
	_ZdaPvSt19__type_descriptor_tm(p, td, 40);

	p = _ZnamSt19__type_descriptor_tSt11align_val_t(40, td, 512);
	check("new[] aligned", ALIGNED(p, 512));
	_ZdaPvSt19__type_descriptor_tSt11align_val_t(p, td, 512);

	p = _ZnamSt19__type_descriptor_tSt11align_val_tRKSt9nothrow_t(40, td, 32, &nt);
	check("new[] aligned nothrow", ALIGNED(p, 32));
	_ZdaPvSt19__type_descriptor_tmSt11align_val_t(p, td, 40, 32);

	p = _ZnamSt19__type_descriptor_tSt11align_val_t(40, td, 32);
	check("new[] aligned (freed with the nothrow delete[])", ALIGNED(p, 32));
	_ZdaPvSt19__type_descriptor_tSt11align_val_tRKSt9nothrow_t(p, td, 32, &nt);

	return failures != 0;
}
