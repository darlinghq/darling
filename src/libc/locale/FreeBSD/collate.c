/*-
 * Copyright (c) 1995 Alex Tatmanjants <alex@elvisti.kiev.ua>
 *		at Electronni Visti IA, Kiev, Ukraine.
 *			All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/collate.c,v 1.35 2005/02/27 20:31:13 ru Exp $");

#include "xlocale_private.h"
/* assumes the locale_t variable is named loc */
#define __collate_chain_equiv_table	(loc->__lc_collate->__chain_equiv_table)
#define __collate_chain_pri_table	(loc->__lc_collate->__chain_pri_table)
#define __collate_char_pri_table	(loc->__lc_collate->__char_pri_table)
#define __collate_info			(&loc->__lc_collate->__info)
#define __collate_large_char_pri_table	(loc->__lc_collate->__large_char_pri_table)
#define __collate_substitute_table	(loc->__lc_collate->__substitute_table)

#include "namespace.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include <unistd.h>
#include <sysexits.h>
#include <ctype.h>
#include "un-namespace.h"

#include "collate.h"
#include "setlocale.h"
#include "ldpart.h"

#include "libc_private.h"

#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
static void wntohl(wchar_t *, int);
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */
void __collate_err(int ex, const char *f) __dead2;

/*
 * Normally, the __collate_* routines should all be __private_extern__,
 * but grep is using them (3715846).  Until we can provide an alternative,
 * we leave them public, and provide a read-only __collate_load_error variable
 */
#undef __collate_load_error
int __collate_load_error = 1;

__private_extern__ int
__collate_load_tables(const char *encoding, locale_t loc)
{
	FILE *fp;
	int i, saverr, chains, z;
	char strbuf[STR_LEN], buf[PATH_MAX];
	struct __xlocale_st_collate *TMP;
	static struct __xlocale_st_collate *cache = NULL;
	struct __collate_st_info info;
	void *vp;

	/* 'encoding' must be already checked. */
	if (strcmp(encoding, "C") == 0 || strcmp(encoding, "POSIX") == 0) {
		loc->__collate_load_error = 1;
		if (loc == &__global_locale)
			__collate_load_error = 1;
		XL_RELEASE(loc->__lc_collate);
		loc->__lc_collate = NULL;
		return (_LDP_CACHE);
	}

	/*
	 * If the locale name is the same as our cache, use the cache.
	 */
	if (cache && strcmp(encoding, cache->__encoding) == 0) {
		loc->__collate_load_error = 0;
		if (loc == &__global_locale)
			__collate_load_error = 0;
		XL_RELEASE(loc->__lc_collate);
		loc->__lc_collate = cache;
		XL_RETAIN(loc->__lc_collate);
		return (_LDP_CACHE);
	}

	/*
	 * Slurp the locale file into the cache.
	 */

	/* 'PathLocale' must be already set & checked. */
	/* Range checking not needed, encoding has fixed size */
	(void)strcpy(buf, _PathLocale);
	(void)strcat(buf, "/");
	(void)strcat(buf, encoding);
	(void)strcat(buf, "/LC_COLLATE");
	if ((fp = fopen(buf, "r")) == NULL)
		return (_LDP_ERROR);

	if (fread(strbuf, sizeof(strbuf), 1, fp) != 1) {
		saverr = errno;
		(void)fclose(fp);
		errno = saverr;
		return (_LDP_ERROR);
	}
	chains = -1;
	if (strcmp(strbuf, COLLATE_VERSION1_1A) == 0)
		chains = 1;
	if (chains < 0) {
		(void)fclose(fp);
		errno = EFTYPE;
		return (_LDP_ERROR);
	}
	if (chains) {
		if (fread(&info, sizeof(info), 1, fp) != 1) {
			saverr = errno;
			(void)fclose(fp);
			errno = saverr;
			return (_LDP_ERROR);
		}
#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
		for(z = 0; z < info.directive_count; z++) {
			info.undef_pri[z] = ntohl(info.undef_pri[z]);
			info.subst_count[z] = ntohl(info.subst_count[z]);
		}
		info.chain_count = ntohl(info.chain_count);
		info.large_pri_count = ntohl(info.large_pri_count);
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */
		if ((chains = info.chain_count) < 0) {
			(void)fclose(fp);
			errno = EFTYPE;
			return (_LDP_ERROR);
		}
	} else
		chains = TABLE_SIZE;

	i = sizeof(struct __xlocale_st_collate)
	    + sizeof(struct __collate_st_chain_pri) * chains
	    + sizeof(struct __collate_st_large_char_pri) * info.large_pri_count;
	for(z = 0; z < info.directive_count; z++)
		i += sizeof(struct __collate_st_subst) * info.subst_count[z];
	if ((TMP = (struct __xlocale_st_collate *)malloc(i)) == NULL) {
		saverr = errno;
		(void)fclose(fp);
		errno = saverr;
		return (_LDP_ERROR);
	}
	TMP->__refcount = 2; /* one for the locale, one for the cache */
	TMP->__free_extra = NULL;

#define FREAD(a, b, c, d) \
{ \
	if (fread(a, b, c, d) != c) { \
		saverr = errno; \
		free(TMP); \
		(void)fclose(d); \
		errno = saverr; \
		return (_LDP_ERROR); \
	} \
}

	/* adjust size to read the remaining in one chunk */
	i -= offsetof(struct __xlocale_st_collate, __char_pri_table);
	FREAD(TMP->__char_pri_table, i, 1, fp);
	(void)fclose(fp);

	vp = (void *)(TMP + 1);

	/* the COLLATE_SUBST_DUP optimization relies on COLL_WEIGHTS_MAX == 2 */
	if (info.subst_count[0] > 0) {
		TMP->__substitute_table[0] = (struct __collate_st_subst *)vp;
		vp += info.subst_count[0] * sizeof(struct __collate_st_subst);
	} else
		TMP->__substitute_table[0] = NULL;
	if (info.flags & COLLATE_SUBST_DUP)
		TMP->__substitute_table[1] = TMP->__substitute_table[0];
	else if (info.subst_count[1] > 0) {
		TMP->__substitute_table[1] = (struct __collate_st_subst *)vp;
		vp += info.subst_count[1] * sizeof(struct __collate_st_subst);
	} else
		TMP->__substitute_table[1] = NULL;

	if (chains > 0) {
		TMP->__chain_pri_table = (struct __collate_st_chain_pri *)vp;
		vp += chains * sizeof(struct __collate_st_chain_pri);
	} else
		TMP->__chain_pri_table = NULL;
	if (info.large_pri_count > 0)
		TMP->__large_char_pri_table = (struct __collate_st_large_char_pri *)vp;
	else
		TMP->__large_char_pri_table = NULL;

#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
	{
		struct __collate_st_char_pri *p = TMP->__char_pri_table;
		for(i = UCHAR_MAX + 1; i-- > 0; p++) {
			for(z = 0; z < info.directive_count; z++)
				p->pri[z] = ntohl(p->pri[z]);
		}
	}
	for(z = 0; z < info.directive_count; z++)
		if (info.subst_count[z] > 0) {
			struct __collate_st_subst *p = TMP->__substitute_table[z];
			for(i = info.subst_count[z]; i-- > 0; p++) {
				p->val = ntohl(p->val);
				wntohl(p->str, STR_LEN);
			}
		}
	{
		struct __collate_st_chain_pri *p = TMP->__chain_pri_table;
		for(i = chains; i-- > 0; p++) {
			wntohl(p->str, STR_LEN);
			for(z = 0; z < info.directive_count; z++)
				p->pri[z] = ntohl(p->pri[z]);
		}
	}
	if (info.large_pri_count > 0) {
		struct __collate_st_large_char_pri *p = TMP->__large_char_pri_table;
		for(i = info.large_pri_count; i-- > 0; p++) {
			p->val = ntohl(p->val);
			for(z = 0; z < info.directive_count; z++)
				p->pri.pri[z] = ntohl(p->pri.pri[z]);
		}
	}
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */
	(void)strcpy(TMP->__encoding, encoding);
	(void)memcpy(&TMP->__info, &info, sizeof(info));
	XL_RELEASE(cache);
	cache = TMP;
	XL_RELEASE(loc->__lc_collate);
	loc->__lc_collate = cache;
	/* no need to retain, since we set __refcount to 2 above */
	
	loc->__collate_substitute_nontrivial = (info.subst_count[0] > 0 || info.subst_count[1] > 0);
	loc->__collate_load_error = 0;
	if (loc == &__global_locale)
		__collate_load_error = 0;

	return (_LDP_LOADED);
}

static int
__collate_wcsnlen(const wchar_t *s, int len)
{
	int n = 0;
	while (*s && n < len) {
		s++;
		n++;
	}
	return n;
}

static struct __collate_st_subst *
substsearch(const wchar_t key, struct __collate_st_subst *tab, int n)
{
	int low = 0;
	int high = n - 1;
	int next, compar;
	struct __collate_st_subst *p;

	while (low <= high) {
		next = (low + high) / 2;
		p = tab + next;
		compar = key - p->val;
		if (compar == 0)
			return p;
		if (compar > 0)
			low = next + 1;
		else
			high = next - 1;
	}
	return NULL;
}

__private_extern__ wchar_t *
__collate_substitute(const wchar_t *s, int which, locale_t loc)
{
	int dest_len, len, nlen;
	int n, delta, nsubst;
	wchar_t *dest_str = NULL;
	const wchar_t *fp;
	struct __collate_st_subst *subst, *match;

	if (s == NULL || *s == '\0')
		return (__collate_wcsdup(L""));
	dest_len = wcslen(s);
	nsubst = __collate_info->subst_count[which];
	if (nsubst <= 0)
		return __collate_wcsdup(s);
	subst = __collate_substitute_table[which];
	delta = dest_len / 4;
	if (delta < 2)
		delta = 2;
	dest_str = (wchar_t *)malloc((dest_len += delta) * sizeof(wchar_t));
	if (dest_str == NULL)
		__collate_err(EX_OSERR, __func__);
	len = 0;
	while (*s) {
		if ((match = substsearch(*s, subst, nsubst)) != NULL) {
			fp = match->str;
			n = __collate_wcsnlen(fp, STR_LEN);
		} else {
			fp = s;
			n = 1;
		}
		nlen = len + n;
		if (dest_len <= nlen) {
			dest_str = reallocf(dest_str, (dest_len = nlen + delta) * sizeof(wchar_t));
			if (dest_str == NULL)
				__collate_err(EX_OSERR, __func__);
		}
		wcsncpy(dest_str + len, fp, n);
		len += n;
		s++;
	}
	dest_str[len] = 0;
	return (dest_str);
}

static struct __collate_st_chain_pri *
chainsearch(const wchar_t *key, int *len, locale_t loc)
{
	int low = 0;
	int high = __collate_info->chain_count - 1;
	int next, compar, l;
	struct __collate_st_chain_pri *p;
	struct __collate_st_chain_pri *tab = __collate_chain_pri_table;

	while (low <= high) {
		next = (low + high) / 2;
		p = tab + next;
		compar = *key - *p->str;
		if (compar == 0) {
			l = __collate_wcsnlen(p->str, STR_LEN);
			compar = wcsncmp(key, p->str, l);
			if (compar == 0) {
				*len = l;
				return p;
			}
		}
		if (compar > 0)
			low = next + 1;
		else
			high = next - 1;
	}
	return NULL;
}

static struct __collate_st_large_char_pri *
largesearch(const wchar_t key, locale_t loc)
{
	int low = 0;
	int high = __collate_info->large_pri_count - 1;
	int next, compar;
	struct __collate_st_large_char_pri *p;
	struct __collate_st_large_char_pri *tab = __collate_large_char_pri_table;

	while (low <= high) {
		next = (low + high) / 2;
		p = tab + next;
		compar = key - p->val;
		if (compar == 0)
			return p;
		if (compar > 0)
			low = next + 1;
		else
			high = next - 1;
	}
	return NULL;
}

__private_extern__ void
__collate_lookup_l(const wchar_t *t, int *len, int *prim, int *sec, locale_t loc)
{
	struct __collate_st_chain_pri *p2;
	int l;

	*len = 1;
	*prim = *sec = 0;
	p2 = chainsearch(t, &l, loc);
	/* use the chain if prim >= 0 */
	if (p2 && p2->pri[0] >= 0) {
		*len = l;
		*prim = p2->pri[0];
		*sec = p2->pri[1];
		return;
	}
	if (*t <= UCHAR_MAX) {
		*prim = __collate_char_pri_table[*t].pri[0];
		*sec = __collate_char_pri_table[*t].pri[1];
		return;
	}
	if (__collate_info->large_pri_count > 0) {
		struct __collate_st_large_char_pri *match;
		match = largesearch(*t, loc);
		if (match) {
			*prim = match->pri.pri[0];
			*sec = match->pri.pri[1];
			return;
		}
	}
	*prim = (l = __collate_info->undef_pri[0]) >= 0 ? l : *t - l;
	*sec = (l = __collate_info->undef_pri[1]) >= 0 ? l : *t - l;
}

/*
 * This is only provided for programs (like grep) that are calling this
 * private function.  This will go away eventually.
 */
void
__collate_lookup(const unsigned char *t, int *len, int *prim, int *sec)
{
	locale_t loc = __current_locale();
	wchar_t *w = __collate_mbstowcs((const char *)t, loc);
	int sverrno;

	__collate_lookup_l(w, len, prim, sec, loc);
	sverrno = errno;
	free(w);
	errno = sverrno;
}
 
__private_extern__ void
__collate_lookup_which(const wchar_t *t, int *len, int *pri, int which, locale_t loc)
{
	struct __collate_st_chain_pri *p2;
	int p, l;

	*len = 1;
	*pri = 0;
	p2 = chainsearch(t, &l, loc);
	if (p2) {
		p = p2->pri[which];
		/* use the chain if pri >= 0 */
		if (p >= 0) {
			*len = l;
			*pri = p;
			return;
		}
	}
	if (*t <= UCHAR_MAX) {
		*pri = __collate_char_pri_table[*t].pri[which];
		return;
	}
	if (__collate_info->large_pri_count > 0) {
		struct __collate_st_large_char_pri *match;
		match = largesearch(*t, loc);
		if (match) {
			*pri = match->pri.pri[which];
			return;
		}
	}
	*pri = (l = __collate_info->undef_pri[which]) >= 0 ? l : *t - l;
}

__private_extern__ wchar_t *
__collate_mbstowcs(const char *s, locale_t loc)
{
	static const mbstate_t initial;
	mbstate_t st;
	size_t len;
	const char *ss;
	wchar_t *wcs;

	ss = s;
	st = initial;
	if ((len = mbsrtowcs_l(NULL, &ss, 0, &st, loc)) == (size_t)-1)
		return NULL;
	if ((wcs = (wchar_t *)malloc((len + 1) * sizeof(wchar_t))) == NULL)
		__collate_err(EX_OSERR, __func__);
	st = initial;
	mbsrtowcs_l(wcs, &s, len, &st, loc);
	wcs[len] = 0;

	return (wcs);
}

__private_extern__ wchar_t *
__collate_wcsdup(const wchar_t *s)
{
	size_t len = wcslen(s) + 1;
	wchar_t *wcs;

	if ((wcs = (wchar_t *)malloc(len * sizeof(wchar_t))) == NULL)
		__collate_err(EX_OSERR, __func__);
	wcscpy(wcs, s);
	return (wcs);
}

__private_extern__ void
__collate_xfrm(const wchar_t *src, wchar_t **xf, locale_t loc)
{
	int pri, len;
	size_t slen;
	const wchar_t *t;
	wchar_t *tt = NULL, *tr = NULL;
	int direc, pass;
	wchar_t *xfp;
	struct __collate_st_info *info = __collate_info;
	int sverrno;

	for(pass = 0; pass < COLL_WEIGHTS_MAX; pass++)
		xf[pass] = NULL;
	for(pass = 0; pass < info->directive_count; pass++) {
		direc = info->directive[pass];
		if (pass == 0 || !(info->flags & COLLATE_SUBST_DUP)) {
			sverrno = errno;
			free(tt);
			errno = sverrno;
			tt = __collate_substitute(src, pass, loc);
		}
		if (direc & DIRECTIVE_BACKWARD) {
			wchar_t *bp, *fp, c;
			sverrno = errno;
			free(tr);
			errno = sverrno;
			tr = __collate_wcsdup(tt ? tt : src);
			bp = tr;
			fp = tr + wcslen(tr) - 1;
			while(bp < fp) {
				c = *bp;
				*bp++ = *fp;
				*fp-- = c;
			}
			t = (const wchar_t *)tr;
		} else if (tt)
			t = (const wchar_t *)tt;
		else
			t = (const wchar_t *)src;
		sverrno = errno;
		if ((xf[pass] = (wchar_t *)malloc(sizeof(wchar_t) * (wcslen(t) + 1))) == NULL) {
			errno = sverrno;
			slen = 0;
			goto end;
		}
		errno = sverrno;
		xfp = xf[pass];
		if (direc & DIRECTIVE_POSITION) {
			while(*t) {
				__collate_lookup_which(t, &len, &pri, pass, loc);
				t += len;
				if (pri <= 0) {
					if (pri < 0) {
						errno = EINVAL;
						slen = 0;
						goto end;
					}
					pri = COLLATE_MAX_PRIORITY;
				}
				*xfp++ = pri;
			}
		} else {
			while(*t) {
				__collate_lookup_which(t, &len, &pri, pass, loc);
				t += len;
				if (pri <= 0) {
					if (pri < 0) {
						errno = EINVAL;
						slen = 0;
						goto end;
					}
					continue;
				}
				*xfp++ = pri;
			}
 		}
		*xfp = 0;
	}
  end:
	sverrno = errno;
	free(tt);
	free(tr);
	errno = sverrno;
}

__private_extern__ void
__collate_err(int ex, const char *f)
{
	const char *s;
	int serrno = errno;

	s = _getprogname();
	_write(STDERR_FILENO, s, strlen(s));
	_write(STDERR_FILENO, ": ", 2);
	s = f;
	_write(STDERR_FILENO, s, strlen(s));
	_write(STDERR_FILENO, ": ", 2);
	s = strerror(serrno);
	_write(STDERR_FILENO, s, strlen(s));
	_write(STDERR_FILENO, "\n", 1);
	exit(ex);
}

/*
 * __collate_collating_symbol takes the multibyte string specified by
 * src and slen, and using ps, converts that to a wide character.  Then
 * it is checked to verify it is a collating symbol, and then copies
 * it to the wide character string specified by dst and dlen (the
 * results are not null terminated).  The length of the wide characters
 * copied to dst is returned if successful.  Zero is returned if no such
 * collating symbol exists.  (size_t)-1 is returned if there are wide-character
 * conversion errors, if the length of the converted string is greater that
 * STR_LEN or if dlen is too small.  It is up to the calling routine to
 * preserve the mbstate_t structure as needed.
 */
__private_extern__ size_t
__collate_collating_symbol(wchar_t *dst, size_t dlen, const char *src, size_t slen, mbstate_t *ps, locale_t loc)
{
	wchar_t wname[STR_LEN];
	wchar_t w, *wp;
	size_t len, l;

	/* POSIX locale */
	if (loc->__collate_load_error) {
		if (dlen < 1)
			return (size_t)-1;
		if (slen != 1 || !isascii(*src))
			return 0;
		*dst = *src;
		return 1;
	}
	for(wp = wname, len = 0; slen > 0; len++) {
		l = mbrtowc_l(&w, src, slen, ps, loc);
		if (l == (size_t)-1 || l == (size_t)-2)
			return (size_t)-1;
		if (l == 0)
			break;
		if (len >= STR_LEN)
			return -1;
		*wp++ = w;
		src += l;
		slen = (long)slen - (long)l;
	}
	if (len == 0 || len > dlen)
		return (size_t)-1;
	if (len == 1) {
		if (*wname <= UCHAR_MAX) {
			if (__collate_char_pri_table[*wname].pri[0] >= 0) {
				if (dlen > 0)
					*dst = *wname;
				return 1;
			}
			return 0;
		} else if (__collate_info->large_pri_count > 0) {
			struct __collate_st_large_char_pri *match;
			match = largesearch(*wname, loc);
			if (match && match->pri.pri[0] >= 0) {
				if (dlen > 0)
					*dst = *wname;
				return 1;
			}
		}
		return 0;
	}
	*wp = 0;
	if (__collate_info->chain_count > 0) {
		struct __collate_st_chain_pri *match;
		int ll;
		match = chainsearch(wname, &ll, loc);
		if (match) {
			if (ll < dlen)
				dlen = ll;
			wcsncpy(dst, wname, dlen);
			return ll;
		}
	}
	return 0;
}

/*
 * __collate_equiv_class returns the equivalence class number for the symbol
 * specified by src and slen, using ps to convert from multi-byte to wide
 * character.  Zero is returned if the symbol is not in an equivalence
 * class.  -1 is returned if there are wide character conversion error,
 * if there are any greater-than-8-bit characters or if a multi-byte symbol
 * is greater or equal to STR_LEN in length.  It is up to the calling
 * routine to preserve the mbstate_t structure as needed.
 */
__private_extern__ int
__collate_equiv_class(const char *src, size_t slen, mbstate_t *ps, locale_t loc)
{
	wchar_t wname[STR_LEN];
	wchar_t w, *wp;
	size_t len, l;
	int e;

	/* POSIX locale */
	if (loc->__collate_load_error)
		return 0;
	for(wp = wname, len = 0; slen > 0; len++) {
		l = mbrtowc_l(&w, src, slen, ps, loc);
		if (l == (size_t)-1 || l == (size_t)-2)
			return -1;
		if (l == 0)
			break;
		if (len >= STR_LEN)
			return -1;
		*wp++ = w;
		src += l;
		slen = (long)slen - (long)l;
	}
	if (len == 0)
		return -1;
	if (len == 1) {
		e = -1;
		if (*wname <= UCHAR_MAX)
			e = __collate_char_pri_table[*wname].pri[0];
		else if (__collate_info->large_pri_count > 0) {
			struct __collate_st_large_char_pri *match;
			match = largesearch(*wname, loc);
			if (match)
				e = match->pri.pri[0];
		}
		if (e == 0)
			return IGNORE_EQUIV_CLASS;
		return e > 0 ? e : 0;
	}
	*wp = 0;
	if (__collate_info->chain_count > 0) {
		struct __collate_st_chain_pri *match;
		int ll;
		match = chainsearch(wname, &ll, loc);
		if (match) {
			e = match->pri[0];
			if (e == 0)
				return IGNORE_EQUIV_CLASS;
			return e < 0 ? -e : e;
		}
	}
	return 0;
}

/*
 * __collate_equiv_match tries to match any single or multi-character symbol
 * in equivalence class equiv_class in the multi-byte string specified by src
 * and slen.  If start is non-zero, it is taken to be the first (pre-converted)
 * wide character.  Subsequence wide characters, if needed, will use ps in
 * the conversion.  On a successful match, the length of the matched string
 * is returned (including the start character).  If dst is non-NULL, the
 * matched wide-character string is copied to dst, a wide character array of
 * length dlen (the results are not zero-terminated).  If rlen is non-NULL,
 * the number of character in src actually used is returned.  Zero is
 * returned by __collate_equiv_match if there is no match.  (size_t)-1 is
 * returned on error: if there were conversion errors or if dlen is too small
 * to accept the results.  On no match or error, ps is restored to its incoming
 * state.
 */
size_t
__collate_equiv_match(int equiv_class, wchar_t *dst, size_t dlen, wchar_t start, const char *src, size_t slen, mbstate_t *ps, size_t *rlen, locale_t loc)
{
	wchar_t w;
	size_t len, l, clen;
	int i;
	wchar_t buf[STR_LEN], *wp;
	mbstate_t save;
	const char *s = src;
	size_t sl = slen;
	struct __collate_st_chain_pri *ch = NULL;

	/* POSIX locale */
	if (loc->__collate_load_error)
		return (size_t)-1;
	if (equiv_class == IGNORE_EQUIV_CLASS)
		equiv_class = 0;
	if (ps)
		save = *ps;
	wp = buf;
	len = clen = 0;
	if (start) {
		*wp++ = start;
		len = 1;
	}
	/* convert up to the max chain length */
	while(sl > 0 && len < __collate_info->chain_max_len) {
		l = mbrtowc_l(&w, s, sl, ps, loc);
		if (l == (size_t)-1 || l == (size_t)-2 || l == 0)
			break;
		*wp++ = w;
		s += l;
		clen += l;
		sl -= l;
		len++;
	}
	*wp = 0;
	if (len > 1 && (ch = chainsearch(buf, &i, loc)) != NULL) {
		int e = ch->pri[0];
		if (e < 0)
			e = -e;
		if (e == equiv_class)
			goto found;
	}
	/* try single character */
	i = 1;
	if (*buf <= UCHAR_MAX) {
		if (equiv_class == __collate_char_pri_table[*buf].pri[0])
			goto found;
	} else if (__collate_info->large_pri_count > 0) {
		struct __collate_st_large_char_pri *match;
		match = largesearch(*buf, loc);
		if (match && equiv_class == match->pri.pri[0])
			goto found;
	}
	/* no match */
	if (ps)
		*ps = save;
	return 0;
found:
	/* if we converted more than we used, restore to initial and reconvert
	 * up to what did match */
	if (i < len) {
		len = i;
		if (ps)
			*ps = save;
		if (start)
			i--;
		clen = 0;
		while(i-- > 0) {
			l = mbrtowc_l(&w, src, slen, ps, loc);
			src += l;
			clen += l;
			slen -= l;
		}
	}
	if (dst) {
		if (dlen < len) {
			if (ps)
				*ps = save;
			return (size_t)-1;
		}
		for(wp = buf; len > 0; len--)
		    *dst++ = *wp++;
	}
	if (rlen)
		*rlen = clen;
	return len;
}

/*
 * __collate_equiv_value returns the primary collation value for the given
 * collating symbol specified by str and len.  Zero or negative is return
 * if the collating symbol was not found.  (Use by the bracket code in TRE.)
 */
__private_extern__ int
__collate_equiv_value(locale_t loc, const wchar_t *str, size_t len)
{
	int e;

	if (len < 1 || len >= STR_LEN)
		return -1;

	/* POSIX locale */
	if (loc->__collate_load_error)
		return (len == 1 && *str <= UCHAR_MAX) ? *str : -1;

	if (len == 1) {
		e = -1;
		if (*str <= UCHAR_MAX)
			e = __collate_char_pri_table[*str].pri[0];
		else if (__collate_info->large_pri_count > 0) {
			struct __collate_st_large_char_pri *match;
			match = largesearch(*str, loc);
			if (match)
				e = match->pri.pri[0];
		}
		if (e == 0)
			return IGNORE_EQUIV_CLASS;
		return e > 0 ? e : 0;
	}
	if (__collate_info->chain_count > 0) {
		wchar_t name[STR_LEN];
		struct __collate_st_chain_pri *match;
		int ll;

		wcsncpy(name, str, len);
		name[len] = 0;
		match = chainsearch(name, &ll, loc);
		if (match) {
			e = match->pri[0];
			if (e == 0)
				return IGNORE_EQUIV_CLASS;
			return e < 0 ? -e : e;
		}
	}
	return 0;
}

#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
static void
wntohl(wchar_t *str, int len)
{
	for(; *str && len > 0; str++, len--)
		*str = ntohl(*str);
}
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */

#ifdef COLLATE_DEBUG
static char *
show(int c)
{
	static char buf[5];

	if (c >=32 && c <= 126)
		sprintf(buf, "'%c' ", c);
	else
		sprintf(buf, "\\x{%02x}", c);
	return buf;
}

static char *
showwcs(const wchar_t *t, int len)
{
	static char buf[64];
	char *cp = buf;

	for(; *t && len > 0; len--, t++) {
		if (*t >=32 && *t <= 126)
			*cp++ = *t;
		else {
			sprintf(cp, "\\x{%02x}", *t);
			cp += strlen(cp);
		}
	}
	*cp = 0;
	return buf;
}

void
__collate_print_tables()
{
	int i, z;
	locale_t loc = __current_locale();

	printf("Info: p=%d s=%d f=0x%02x m=%d dc=%d up=%d us=%d pc=%d sc=%d cc=%d lc=%d\n",
	    __collate_info->directive[0], __collate_info->directive[1],
	    __collate_info->flags, __collate_info->chain_max_len,
	    __collate_info->directive_count,
	    __collate_info->undef_pri[0], __collate_info->undef_pri[1],
	    __collate_info->subst_count[0], __collate_info->subst_count[1],
	    __collate_info->chain_count, __collate_info->large_pri_count);
	for(z = 0; z < __collate_info->directive_count; z++) {
		if (__collate_info->subst_count[z] > 0) {
			struct __collate_st_subst *p2 = __collate_substitute_table[z];
			if (z == 0 && (__collate_info->flags & COLLATE_SUBST_DUP))
				printf("Both substitute tables:\n");
			else
				printf("Substitute table %d:\n", z);
			for (i = __collate_info->subst_count[z]; i-- > 0; p2++)
				printf("\t%s --> \"%s\"\n",
					show(p2->val),
					showwcs(p2->str, STR_LEN));
		}
	}
	if (__collate_info->chain_count > 0) {
		printf("Chain priority table:\n");
		struct __collate_st_chain_pri *p2 = __collate_chain_pri_table;
		for (i = __collate_info->chain_count; i-- > 0; p2++) {
			printf("\t\"%s\" :", showwcs(p2->str, STR_LEN));
			for(z = 0; z < __collate_info->directive_count; z++)
				printf(" %d", p2->pri[z]);
			putchar('\n');
		}
	}
	printf("Char priority table:\n");
	{
		struct __collate_st_char_pri *p2 = __collate_char_pri_table;
		for (i = 0; i < UCHAR_MAX + 1; i++, p2++) {
			printf("\t%s :", show(i));
			for(z = 0; z < __collate_info->directive_count; z++)
				printf(" %d", p2->pri[z]);
			putchar('\n');
		}
	}
	if (__collate_info->large_pri_count > 0) {
		struct __collate_st_large_char_pri *p2 = __collate_large_char_pri_table;
		printf("Large priority table:\n");
		for (i = __collate_info->large_pri_count; i-- > 0; p2++) {
			printf("\t%s :", show(p2->val));
			for(z = 0; z < __collate_info->directive_count; z++)
				printf(" %d", p2->pri.pri[z]);
			putchar('\n');
		}
	}
}
#endif
