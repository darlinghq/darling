/*
 * Copyright (c) 2000, 2001 Alexey Zelkin <phantom@FreeBSD.org>
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/lmonetary.c,v 1.19 2003/06/26 10:46:16 phantom Exp $");

#include "xlocale_private.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ldpart.h"
#include "lmonetary.h"

extern const char * __fix_locale_grouping_str(const char *);

#define LCMONETARY_SIZE_FULL (sizeof(struct lc_monetary_T) / sizeof(char *))
#define LCMONETARY_SIZE_MIN \
		(offsetof(struct lc_monetary_T, int_p_cs_precedes) / \
		    sizeof(char *))

static char	empty[] = "";
static char	numempty[] = { CHAR_MAX, '\0'};

static const struct lc_monetary_T _C_monetary_locale = {
	empty,		/* int_curr_symbol */
	empty,		/* currency_symbol */
	empty,		/* mon_decimal_point */
	empty,		/* mon_thousands_sep */
	empty,		/* mon_grouping [C99 7.11.2.1]*/
	empty,		/* positive_sign */
	empty,		/* negative_sign */
	numempty,	/* int_frac_digits */
	numempty,	/* frac_digits */
	numempty,	/* p_cs_precedes */
	numempty,	/* p_sep_by_space */
	numempty,	/* n_cs_precedes */
	numempty,	/* n_sep_by_space */
	numempty,	/* p_sign_posn */
	numempty,	/* n_sign_posn */
	numempty,	/* int_p_cs_precedes */
	numempty,	/* int_n_cs_precedes */
	numempty,	/* int_p_sep_by_space */
	numempty,	/* int_n_sep_by_space */
	numempty,	/* int_p_sign_posn */
	numempty	/* int_n_sign_posn */
};

static char
cnv(const char *str)
{
	int i = strtol(str, NULL, 10);

	if (i == -1)
		i = CHAR_MAX;
	return ((char)i);
}

__private_extern__ int
__monetary_load_locale(const char *name, locale_t loc)
{
	int ret;
	struct __xlocale_st_monetary *xp;
	static struct __xlocale_st_monetary *cache = NULL;

	/* 'name' must be already checked. */
	if (strcmp(name, "C") == 0 || strcmp(name, "POSIX") == 0) {
		if (!loc->_monetary_using_locale)
			return (_LDP_CACHE);
		loc->_monetary_using_locale = 0;
		XL_RELEASE(loc->__lc_monetary);
		loc->__lc_monetary = NULL;
		loc->__mlocale_changed = 1;
		return (_LDP_CACHE);
	}

	if (loc->_monetary_using_locale && strcmp(name, loc->__lc_monetary->_monetary_locale_buf) == 0)
		return (_LDP_CACHE);
	/*
	 * If the locale name is the same as our cache, use the cache.
	 */
	if (cache && cache->_monetary_locale_buf && strcmp(name, cache->_monetary_locale_buf) == 0) {
		loc->_monetary_using_locale = 1;
		XL_RELEASE(loc->__lc_monetary);
		loc->__lc_monetary = cache;
		XL_RETAIN(loc->__lc_monetary);
		loc->__mlocale_changed = 1;
		return (_LDP_CACHE);
	}
	if ((xp = (struct __xlocale_st_monetary *)malloc(sizeof(*xp))) == NULL)
		return _LDP_ERROR;
	xp->__refcount = 1;
	xp->__free_extra = (__free_extra_t)__ldpart_free_extra;
	xp->_monetary_locale_buf = NULL;

	ret = __part_load_locale(name, &loc->_monetary_using_locale,
		&xp->_monetary_locale_buf, "LC_MONETARY",
		LCMONETARY_SIZE_FULL, LCMONETARY_SIZE_MIN,
		(const char **)&xp->_monetary_locale);
	if (ret != _LDP_ERROR)
		loc->__mlocale_changed = 1;
	else
		free(xp);
	if (ret == _LDP_LOADED) {
		xp->_monetary_locale.mon_grouping =
		     __fix_locale_grouping_str(xp->_monetary_locale.mon_grouping);

#define M_ASSIGN_CHAR(NAME) (((char *)xp->_monetary_locale.NAME)[0] = \
			     cnv(xp->_monetary_locale.NAME))

		M_ASSIGN_CHAR(int_frac_digits);
		M_ASSIGN_CHAR(frac_digits);
		M_ASSIGN_CHAR(p_cs_precedes);
		M_ASSIGN_CHAR(p_sep_by_space);
		M_ASSIGN_CHAR(n_cs_precedes);
		M_ASSIGN_CHAR(n_sep_by_space);
		M_ASSIGN_CHAR(p_sign_posn);
		M_ASSIGN_CHAR(n_sign_posn);

		/*
		 * The six additional C99 international monetary formatting
		 * parameters default to the national parameters when
		 * reading FreeBSD LC_MONETARY data files.
		 */
#define	M_ASSIGN_ICHAR(NAME)						\
		do {							\
			if (xp->_monetary_locale.int_##NAME == NULL)	\
				xp->_monetary_locale.int_##NAME =	\
				    xp->_monetary_locale.NAME;		\
			else						\
				M_ASSIGN_CHAR(int_##NAME);		\
		} while (0)

		M_ASSIGN_ICHAR(p_cs_precedes);
		M_ASSIGN_ICHAR(n_cs_precedes);
		M_ASSIGN_ICHAR(p_sep_by_space);
		M_ASSIGN_ICHAR(n_sep_by_space);
		M_ASSIGN_ICHAR(p_sign_posn);
		M_ASSIGN_ICHAR(n_sign_posn);
		XL_RELEASE(loc->__lc_monetary);
		loc->__lc_monetary = xp;
		XL_RELEASE(cache);
		cache = xp;
		XL_RETAIN(cache);
	}
	return (ret);
}

__private_extern__ struct lc_monetary_T *
__get_current_monetary_locale(locale_t loc)
{
	return (loc->_monetary_using_locale
		? &loc->__lc_monetary->_monetary_locale
		: (struct lc_monetary_T *)&_C_monetary_locale);
}

#ifdef LOCALE_DEBUG
void
monetdebug() {
locale_t loc = __current_locale();
printf(	"int_curr_symbol = %s\n"
	"currency_symbol = %s\n"
	"mon_decimal_point = %s\n"
	"mon_thousands_sep = %s\n"
	"mon_grouping = %s\n"
	"positive_sign = %s\n"
	"negative_sign = %s\n"
	"int_frac_digits = %d\n"
	"frac_digits = %d\n"
	"p_cs_precedes = %d\n"
	"p_sep_by_space = %d\n"
	"n_cs_precedes = %d\n"
	"n_sep_by_space = %d\n"
	"p_sign_posn = %d\n"
	"n_sign_posn = %d\n",
	"int_p_cs_precedes = %d\n"
	"int_p_sep_by_space = %d\n"
	"int_n_cs_precedes = %d\n"
	"int_n_sep_by_space = %d\n"
	"int_p_sign_posn = %d\n"
	"int_n_sign_posn = %d\n",
	loc->__lc_monetary->_monetary_locale.int_curr_symbol,
	loc->__lc_monetary->_monetary_locale.currency_symbol,
	loc->__lc_monetary->_monetary_locale.mon_decimal_point,
	loc->__lc_monetary->_monetary_locale.mon_thousands_sep,
	loc->__lc_monetary->_monetary_locale.mon_grouping,
	loc->__lc_monetary->_monetary_locale.positive_sign,
	loc->__lc_monetary->_monetary_locale.negative_sign,
	loc->__lc_monetary->_monetary_locale.int_frac_digits[0],
	loc->__lc_monetary->_monetary_locale.frac_digits[0],
	loc->__lc_monetary->_monetary_locale.p_cs_precedes[0],
	loc->__lc_monetary->_monetary_locale.p_sep_by_space[0],
	loc->__lc_monetary->_monetary_locale.n_cs_precedes[0],
	loc->__lc_monetary->_monetary_locale.n_sep_by_space[0],
	loc->__lc_monetary->_monetary_locale.p_sign_posn[0],
	loc->__lc_monetary->_monetary_locale.n_sign_posn[0],
	loc->__lc_monetary->_monetary_locale.int_p_cs_precedes[0],
	loc->__lc_monetary->_monetary_locale.int_p_sep_by_space[0],
	loc->__lc_monetary->_monetary_locale.int_n_cs_precedes[0],
	loc->__lc_monetary->_monetary_locale.int_n_sep_by_space[0],
	loc->__lc_monetary->_monetary_locale.int_p_sign_posn[0],
	loc->__lc_monetary->_monetary_locale.int_n_sign_posn[0]
);
}
#endif /* LOCALE_DEBUG */
