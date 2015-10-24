/*
 * Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)localeconv.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/localeconv.c,v 1.14 2007/12/12 07:43:23 phantom Exp $");

#include "xlocale_private.h"

#include <limits.h>
#include <locale.h>

#include "lmonetary.h"
#include "lnumeric.h"

#ifdef __APPLE_PR3417676_HACK__
/*------------------------------------------------------------------------
 * PR-3417676: We need to provide a way to force "C" locale style number
 * formatting independent of the locale setting.  We provide private
 * routines to get and set a flag that tells localeconv() to either return
 * a "C" struct lconv, or the one dependent on the actual locale.
 *------------------------------------------------------------------------*/
static char empty[] = "";
static char numempty[] = { CHAR_MAX, '\0' };

/*
 * Default (C) locale conversion.
 */
static struct lconv _C_lconv = {
	".",			/* decimal_point */
	empty,			/* thousands_sep */
	numempty,		/* grouping */
	empty,			/* int_curr_symbol */
	empty,			/* currency_symbol */
	empty,			/* mon_decimal_point */
	empty,			/* mon_thousands_sep */
	numempty,		/* mon_grouping */
	empty,			/* positive_sign */
	empty,			/* negative_sign */
	CHAR_MAX,		/* int_frac_digits */
	CHAR_MAX,		/* frac_digits */
	CHAR_MAX,		/* p_cs_precedes */
	CHAR_MAX,		/* p_sep_by_space */
	CHAR_MAX,		/* n_cs_precedes */
	CHAR_MAX,		/* n_sep_by_space */
	CHAR_MAX,		/* p_sign_posn */
	CHAR_MAX,		/* n_sign_posn */
	CHAR_MAX,		/* int_p_cs_precedes */
	CHAR_MAX,		/* int_n_cs_precedes */
	CHAR_MAX,		/* int_p_sep_by_space */
	CHAR_MAX,		/* int_n_sep_by_space */
	CHAR_MAX,		/* int_p_sign_posn */
	CHAR_MAX,		/* int_n_sign_posn */
};
static int _onlyClocaleconv = 0;

int
__getonlyClocaleconv(void)
{
    return _onlyClocaleconv;
}

int
__setonlyClocaleconv(int val)
{
    int prev = _onlyClocaleconv;

    _onlyClocaleconv = val;
    return prev;
}
#endif /* __APPLE_PR3417676_HACK__ */

/* 
 * The localeconv() function constructs a struct lconv from the current
 * monetary and numeric locales.
 *
 * Because localeconv() may be called many times (especially by library
 * routines like printf() & strtod()), the approprate members of the 
 * lconv structure are computed only when the monetary or numeric 
 * locale has been changed.
 */

/*
 * Return the current locale conversion.
 */
struct lconv *
localeconv_l(locale_t loc)
{
    struct lconv *lc;

    NORMALIZE_LOCALE(loc);

    if (loc->__mlocale_changed) {
      XL_LOCK(loc);
      if (loc->__mlocale_changed) {
	/* LC_MONETARY part */
        struct lc_monetary_T * mptr; 
	struct lconv *lc = &loc->__lc_localeconv;

#define M_ASSIGN_STR(NAME) (lc->NAME = (char*)mptr->NAME)
#define M_ASSIGN_CHAR(NAME) (lc->NAME = mptr->NAME[0])

	mptr = __get_current_monetary_locale(loc);
	M_ASSIGN_STR(int_curr_symbol);
	M_ASSIGN_STR(currency_symbol);
	M_ASSIGN_STR(mon_decimal_point);
	M_ASSIGN_STR(mon_thousands_sep);
	M_ASSIGN_STR(mon_grouping);
	M_ASSIGN_STR(positive_sign);
	M_ASSIGN_STR(negative_sign);
	M_ASSIGN_CHAR(int_frac_digits);
	M_ASSIGN_CHAR(frac_digits);
	M_ASSIGN_CHAR(p_cs_precedes);
	M_ASSIGN_CHAR(p_sep_by_space);
	M_ASSIGN_CHAR(n_cs_precedes);
	M_ASSIGN_CHAR(n_sep_by_space);
	M_ASSIGN_CHAR(p_sign_posn);
	M_ASSIGN_CHAR(n_sign_posn);
	M_ASSIGN_CHAR(int_p_cs_precedes);
	M_ASSIGN_CHAR(int_n_cs_precedes);
	M_ASSIGN_CHAR(int_p_sep_by_space);
	M_ASSIGN_CHAR(int_n_sep_by_space);
	M_ASSIGN_CHAR(int_p_sign_posn);
	M_ASSIGN_CHAR(int_n_sign_posn);
	loc->__mlocale_changed = 0;
      }
      XL_UNLOCK(loc);
    }

    if (loc->__nlocale_changed) {
      XL_LOCK(loc);
      if (loc->__nlocale_changed) {
	/* LC_NUMERIC part */
        struct lc_numeric_T * nptr; 
	struct lconv *lc = &loc->__lc_localeconv;

#define N_ASSIGN_STR(NAME) (lc->NAME = (char*)nptr->NAME)

	nptr = __get_current_numeric_locale(loc);
	N_ASSIGN_STR(decimal_point);
	N_ASSIGN_STR(thousands_sep);
	N_ASSIGN_STR(grouping);
	loc->__nlocale_changed = 0;
      }
      XL_UNLOCK(loc);
    }

    return &loc->__lc_localeconv;
}

/*
 * Return the current locale conversion.
 */
struct lconv *
localeconv()
{
#ifdef __APPLE_PR3417676_HACK__
    /*--------------------------------------------------------------------
     * If _onlyClocaleconv is non-zero, just return __lconv, which is a "C"
     * struct lconv *.  Otherwise, do the normal thing.
     *--------------------------------------------------------------------*/
    if (_onlyClocaleconv)
	return &_C_lconv;
#endif /* __APPLE_PR3417676_HACK__ */
    return localeconv_l(__current_locale());
}
