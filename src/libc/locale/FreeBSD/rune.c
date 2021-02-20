/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstrict-prototypes"

#ifndef RUNEOFF32
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rune.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/rune.c,v 1.12 2004/07/29 06:16:19 tjr Exp $");

#include "xlocale_private.h"

#include "namespace.h"
#include <arpa/inet.h>
#include <errno.h>
#include <runetype.h>
#else
#include "runetype.h"
#endif /* !RUNEOFF32 */
#include <stdio.h>
#ifndef RUNEOFF32
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "un-namespace.h"
#endif /* !RUNEOFF32 */

#if defined(__LP64__) || defined(RUNEOFF32)
/*
 * Because the LC_CTYPE files were created with a 32-bit program, we need
 * to adjust for the larger pointers in LP64 (the longs have already been
 * replaced by 32-bit equivalents).  Also, natural alignment will pad
 * 64-bit types to 8-byte boundaries, and make structures containing
 * 64-bit types sized to 8-byte boundaries.
 */
#include <stddef.h>
#ifndef RUNEOFF32
#include "rune32.h"
#define BYTES32BITS		4
#define BYTES64BITS		8
/* whether to skip over a pointer or not (one-to-one with off64) */
int skip[] = {
	1,
	1,
	0,
	1,
	0,
	1,
	0,
	1,
	1,
	1,
	0
};
#endif /* !RUNEOFF32 */
int off64[] = {
	offsetof(_RuneLocale, __sgetrune),
	offsetof(_RuneLocale, __sputrune),
	offsetof(_RuneLocale, __runetype_ext),
	offsetof(_RuneLocale, __runetype_ext) + offsetof(_RuneRange, __ranges),
	offsetof(_RuneLocale, __maplower_ext),
	offsetof(_RuneLocale, __maplower_ext) + offsetof(_RuneRange, __ranges),
	offsetof(_RuneLocale, __mapupper_ext),
	offsetof(_RuneLocale, __mapupper_ext) + offsetof(_RuneRange, __ranges),
	offsetof(_RuneLocale, __variable),
	offsetof(_RuneLocale, __charclasses),
	sizeof(_RuneLocale)
};
#define	NOFF			(sizeof(off64) / sizeof(int))
#ifdef RUNEOFF32
/*
 * This program generates a header file (on stdout) that containes the 32-bit
 * offsets, plus some 32-bit sizes
 */
main()
{
	int i;
	printf("#define SIZEOF32_RUNEENTRY %d\n", sizeof(_RuneEntry));
	printf("#define SIZEOF32_RUNELOCALE %d\n", sizeof(_RuneLocale));
	printf("int off32[] = {\n");
	for(i = 0; i < NOFF; i++)
		printf("\t%d,\n", off64[i]);
	printf("};\n");
	return 0;
}
#endif /* RUNEOFF32 */
#else /* !__LP64__ && !RUNEOFF32 */
#define	SIZEOF32_RUNELOCALE	sizeof(_RuneLocale)
#endif /* __LP64__ || RUNEOFF32 */

#ifndef RUNEOFF32
struct __xlocale_st_runelocale *
_Read_RuneMagi(fp)
	FILE *fp;
{
	struct __xlocale_st_runelocale *data;
	void *lastp;
	_RuneLocale *rl;
	_RuneEntry *rr;
	struct stat sb;
	int x, saverr;

	if (_fstat(fileno(fp), &sb) < 0)
		return (NULL);

	if (sb.st_size < SIZEOF32_RUNELOCALE) {
		errno = EFTYPE;
		return (NULL);
	}

#ifdef __LP64__
	/* will adjust later */
	if ((data = (struct __xlocale_st_runelocale *)malloc(sizeof(struct __xlocale_st_runelocale))) == NULL)
#else /* !__LP64__ */
	if ((data = (struct __xlocale_st_runelocale *)malloc(sizeof(struct __xlocale_st_runelocale) - sizeof(_RuneLocale) + sb.st_size)) == NULL)
#endif /* __LP64__ */
		return (NULL);
	data->__refcount = 1;
	data->__free_extra = NULL;

	errno = 0;
	rewind(fp); /* Someone might have read the magic number once already */
	if (errno) {
		saverr = errno;
		free(data);
		errno = saverr;
		return (NULL);
	}

	rl = &data->_CurrentRuneLocale;

#ifdef __LP64__
	if (fread(rl, SIZEOF32_RUNELOCALE, 1, fp) != 1)
#else /* !__LP64__ */
	if (fread(rl, sb.st_size, 1, fp) != 1)
#endif /* __LP64__ */
	{
		saverr = errno;
		free(data);
		errno = saverr;
		return (NULL);
	}

#ifndef __LP64__
	lastp = (char *)rl + sb.st_size;

	rl->__variable = rl + 1;
#endif /* __LP64__ */

	if (memcmp(rl->__magic, _RUNE_MAGIC_A, sizeof(rl->__magic))) {
		free(data);
		errno = EFTYPE;
		return (NULL);
	}

#ifdef __LP64__
	/* shift things into the right position */
	for (x = NOFF - 2; x >= 0; x--)
		memmove((char *)rl + off64[x] + (skip[x] ? BYTES64BITS : 0),
			(char *)rl + off32[x] + (skip[x] ? BYTES32BITS : 0),
			off32[x + 1] - off32[x] - (skip[x] ? BYTES32BITS : 0));
#endif /* __LP64__ */
#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
	rl->__invalid_rune = ntohl(rl->__invalid_rune);
	rl->__variable_len = ntohl(rl->__variable_len);
	rl->__ncharclasses = ntohl(rl->__ncharclasses);
	rl->__runetype_ext.__nranges = ntohl(rl->__runetype_ext.__nranges);
	rl->__maplower_ext.__nranges = ntohl(rl->__maplower_ext.__nranges);
	rl->__mapupper_ext.__nranges = ntohl(rl->__mapupper_ext.__nranges);

	for (x = 0; x < _CACHED_RUNES; ++x) {
		rl->__runetype[x] = ntohl(rl->__runetype[x]);
		rl->__maplower[x] = ntohl(rl->__maplower[x]);
		rl->__mapupper[x] = ntohl(rl->__mapupper[x]);
	}
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */

#ifdef __LP64__
	{
	int count = rl->__runetype_ext.__nranges + rl->__maplower_ext.__nranges
	    + rl->__mapupper_ext.__nranges;
	int extra = sb.st_size - SIZEOF32_RUNELOCALE - count * SIZEOF32_RUNEENTRY - rl->__ncharclasses * sizeof(_RuneCharClass);
	_RuneEntry *rp;

	if (extra < 0) {
		saverr = errno;
		free(data);
		errno = saverr;
		return (NULL);
	}
	if ((data = (struct __xlocale_st_runelocale *)reallocf(data, sizeof(struct __xlocale_st_runelocale)
	    + count * sizeof(_RuneEntry)
	    + rl->__ncharclasses * sizeof(_RuneCharClass)
	    + extra)) == NULL)
		return (NULL);
	rl = &data->_CurrentRuneLocale;
	rl->__variable = rl + 1;
	rp = (_RuneEntry *)rl->__variable;
	for (x = 0; x < count; x++, rp++)
		if (fread(rp, SIZEOF32_RUNEENTRY, 1, fp) != 1) {
			saverr = errno;
			free(data);
			errno = saverr;
			return (NULL);
		}
	if (rl->__ncharclasses > 0) {
		if (fread(rp, sizeof(_RuneCharClass), rl->__ncharclasses, fp) != rl->__ncharclasses) {
			saverr = errno;
			free(data);
			errno = saverr;
			return (NULL);
		}
		rp = (_RuneEntry *)((char *)rp + rl->__ncharclasses * sizeof(_RuneCharClass));
	}
	if (extra > 0 && fread(rp, extra, 1, fp) != 1) {
		saverr = errno;
		free(data);
		errno = saverr;
		return (NULL);
	}
	lastp = (char *)rp + extra;
	}
#endif /* __LP64__ */
	rl->__runetype_ext.__ranges = (_RuneEntry *)rl->__variable;
	rl->__variable = rl->__runetype_ext.__ranges +
	    rl->__runetype_ext.__nranges;
	if (rl->__variable > lastp) {
		free(data);
		errno = EFTYPE;
		return (NULL);
	}

	rl->__maplower_ext.__ranges = (_RuneEntry *)rl->__variable;
	rl->__variable = rl->__maplower_ext.__ranges +
	    rl->__maplower_ext.__nranges;
	if (rl->__variable > lastp) {
		free(data);
		errno = EFTYPE;
		return (NULL);
	}

	rl->__mapupper_ext.__ranges = (_RuneEntry *)rl->__variable;
	rl->__variable = rl->__mapupper_ext.__ranges +
	    rl->__mapupper_ext.__nranges;
	if (rl->__variable > lastp) {
		free(data);
		errno = EFTYPE;
		return (NULL);
	}

	for (x = 0; x < rl->__runetype_ext.__nranges; ++x) {
		rr = rl->__runetype_ext.__ranges;

#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
		rr[x].__min = ntohl(rr[x].__min);
		rr[x].__max = ntohl(rr[x].__max);
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */
		if ((rr[x].__map = ntohl(rr[x].__map)) == 0) {
			int len = rr[x].__max - rr[x].__min + 1;
			rr[x].__types = rl->__variable;
			rl->__variable = rr[x].__types + len;
			if (rl->__variable > lastp) {
				free(data);
				errno = EFTYPE;
				return (NULL);
			}
#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
			while (len-- > 0)
				rr[x].__types[len] = ntohl(rr[x].__types[len]);
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */
		} else
			rr[x].__types = 0;
	}

#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
	for (x = 0; x < rl->__maplower_ext.__nranges; ++x) {
		rr = rl->__maplower_ext.__ranges;

		rr[x].__min = ntohl(rr[x].__min);
		rr[x].__max = ntohl(rr[x].__max);
		rr[x].__map = ntohl(rr[x].__map);
	}

	for (x = 0; x < rl->__mapupper_ext.__nranges; ++x) {
		rr = rl->__mapupper_ext.__ranges;

		rr[x].__min = ntohl(rr[x].__min);
		rr[x].__max = ntohl(rr[x].__max);
		rr[x].__map = ntohl(rr[x].__map);
	}
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */

	if (rl->__ncharclasses > 0) {
		rl->__charclasses = (_RuneCharClass *)rl->__variable;
		rl->__variable = (void *)(rl->__charclasses + rl->__ncharclasses);
		if (rl->__variable > lastp) {
			free(data);
			errno = EFTYPE;
			return (NULL);
		}
#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
		for (x = 0; x < rl->__ncharclasses; ++x)
			rl->__charclasses[x].__mask = ntohl(rl->__charclasses[x].__mask);
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */
	}

	if (((char *)rl->__variable) + rl->__variable_len > (char *)lastp) {
		free(data);
		errno = EFTYPE;
		return (NULL);
	}

	/*
	 * Go out and zero pointers that should be zero.
	 */
	if (!rl->__variable_len)
		rl->__variable = 0;

	if (!rl->__runetype_ext.__nranges)
		rl->__runetype_ext.__ranges = 0;

	if (!rl->__maplower_ext.__nranges)
		rl->__maplower_ext.__ranges = 0;

	if (!rl->__mapupper_ext.__nranges)
		rl->__mapupper_ext.__ranges = 0;

	data->__datasize = lastp - (void *)data;
	return (data);
}
#endif /* !RUNEOFF32 */
#pragma clang diagnostic pop
