/*	$NetBSD: strfmon.c,v 1.7 2009/01/30 23:46:03 lukem Exp $	*/

/*-
 * Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
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
 *
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
#if 0
__FBSDID("$FreeBSD: src/lib/libc/stdlib/strfmon.c,v 1.14 2003/03/20 08:18:55 ache Exp $");
#else
__RCSID("$NetBSD: strfmon.c,v 1.7 2009/01/30 23:46:03 lukem Exp $");
#endif
#endif /* LIBC_SCCS and not lint */

#if defined(__NetBSD__)
#include "namespace.h"
#include <monetary.h>
#endif

#include "xlocale_private.h"

#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* internal flags */
#define	NEED_GROUPING		0x01	/* print digits grouped (default) */
#define	SIGN_POSN_USED		0x02	/* '+' or '(' usage flag */
#define	LOCALE_POSN		0x04	/* use locale defined +/- (default) */
#define	PARENTH_POSN		0x08	/* enclose negative amount in () */
#define	SUPRESS_CURR_SYMBOL	0x10	/* supress the currency from output */
#define	LEFT_JUSTIFY		0x20	/* left justify */
#define	USE_INTL_CURRENCY	0x40	/* use international currency symbol */
#define IS_NEGATIVE		0x80	/* is argument value negative ? */

/* internal macros */
#define PRINT(CH) do {						\
	if (dst >= s + maxsize) 				\
		goto e2big_error;				\
	*dst++ = CH;						\
} while (/* CONSTCOND */ 0)

#define PRINTS(STR) do {					\
	const char *tmps = STR;					\
	while (*tmps != '\0')					\
		PRINT(*tmps++);					\
} while (/* CONSTCOND */ 0)

#define GET_NUMBER(VAR,LOC)	do {				\
	VAR = 0;						\
	while (isdigit_l((unsigned char)*fmt, (LOC))) {			\
		VAR *= 10;					\
		VAR += *fmt - '0';				\
		if (VAR > 0x00ffffff)				\
			goto e2big_error;			\
		fmt++;						\
	}							\
} while (/* CONSTCOND */ 0)

#define GRPCPY(howmany) do {					\
	int i = howmany;					\
	while (i-- > 0) {					\
		avalue_size--;					\
		*--bufend = *(avalue+avalue_size+padded);	\
	}							\
} while (/* CONSTCOND */ 0)

#define GRPSEP do {						\
	*--bufend = thousands_sep;				\
	groups++;						\
} while (/* CONSTCOND */ 0)

static void __setup_vars(int, char *, char *, char *, const char **, struct lconv *);
static int __calc_left_pad(int, char *, struct lconv *);
static char *__format_grouped_double(double, int *, int, int, int, struct lconv *, locale_t);

static ssize_t
_strfmon(char * __restrict s, size_t maxsize, locale_t loc, const char * __restrict format, va_list ap)
{
	char 		*dst;		/* output destination pointer */
	const char 	*fmt;		/* current format poistion pointer */
	struct lconv 	*lc;		/* pointer to lconv structure */
	char		*asciivalue;	/* formatted double pointer */

	int		flags;		/* formatting options */
	int		pad_char;	/* padding character */
	int		pad_size;	/* pad size */
	int		width;		/* field width */
	int		left_prec;	/* left precision */
	int		right_prec;	/* right precision */
	double		value;		/* just value */
	char		space_char = ' '; /* space after currency */

	char		cs_precedes,	/* values gathered from struct lconv */
			sep_by_space,
			sign_posn,
			*currency_symbol;
	const char	*signstr;

	char		*tmpptr;	/* temporary vars */
	int		sverrno;

	lc = localeconv_l(loc);
	dst = s;
	fmt = format;
	asciivalue = NULL;
	currency_symbol = NULL;
	pad_size = 0;

	while (*fmt) {
		/* pass nonformating characters AS IS */
		if (*fmt != '%')
			goto literal;

		/* '%' found ! */

		/* "%%" mean just '%' */
		if (*(fmt+1) == '%') {
			fmt++;
	literal:
			PRINT(*fmt++);
			continue;
		}

		/* set up initial values */
		flags = (NEED_GROUPING|LOCALE_POSN);
		pad_char = ' ';		/* padding character is "space" */
		left_prec = -1;		/* no left precision specified */
		right_prec = -1;	/* no right precision specified */
		width = -1;		/* no width specified */
		value = 0;		/* we have no value to print now */

		/* Flags */
		while (/* CONSTCOND */ 1) {
			switch (*++fmt) {
				case '=':	/* fill character */
					pad_char = *++fmt;
					if (pad_char == '\0')
						goto format_error;
					continue;
				case '^':	/* not group currency  */
					flags &= ~(NEED_GROUPING);
					continue;
				case '+':	/* use locale defined signs */
					if (flags & SIGN_POSN_USED)
						goto format_error;
					flags |= (SIGN_POSN_USED|LOCALE_POSN);
					continue;
				case '(':	/* enclose negatives with () */
					if (flags & SIGN_POSN_USED)
						goto format_error;
					flags |= (SIGN_POSN_USED|PARENTH_POSN);
					continue;
				case '!':	/* suppress currency symbol */
					flags |= SUPRESS_CURR_SYMBOL;
					continue;
				case '-':	/* alignment (left)  */
					flags |= LEFT_JUSTIFY;
					continue;
				default:
					break;
			}
			break;
		}

		/* field Width */
		if (isdigit_l((unsigned char)*fmt, loc)) {
			ptrdiff_t d = dst - s;
			GET_NUMBER(width, loc);
			/* Do we have enough space to put number with
			 * required width ?
			 */

			if ((size_t)(d + width) >= maxsize)
				goto e2big_error;
		}

		/* Left precision */
		if (*fmt == '#') {
			if (!isdigit_l((unsigned char)*++fmt, loc))
				goto format_error;
			GET_NUMBER(left_prec, loc);
		}

		/* Right precision */
		if (*fmt == '.') {
			if (!isdigit_l((unsigned char)*++fmt, loc))
				goto format_error;
			GET_NUMBER(right_prec, loc);
		}

		/* Conversion Characters */
		switch (*fmt++) {
			case 'i':	/* use internaltion currency format */
				flags |= USE_INTL_CURRENCY;
				break;
			case 'n':	/* use national currency format */
				flags &= ~(USE_INTL_CURRENCY);
				break;
			default:	/* required character is missing or
					   premature EOS */
				goto format_error;
		}

		if (currency_symbol)
			free(currency_symbol);
		if (flags & USE_INTL_CURRENCY) {
			currency_symbol = strdup(lc->int_curr_symbol);
			if (currency_symbol != NULL) {
				space_char = *(currency_symbol+3);
				currency_symbol[3] = '\0';
			}
		} else
			currency_symbol = strdup(lc->currency_symbol);

		if (currency_symbol == NULL)
			goto end_error;			/* ENOMEM. */

		/* value itself */
		value = va_arg(ap, double);

		/* detect sign */
		if (value < 0) {
			flags |= IS_NEGATIVE;
			value = -value;
		}

		/* fill left_prec with amount of padding chars */
		if (left_prec >= 0) {
			pad_size = __calc_left_pad((flags ^ IS_NEGATIVE),
							currency_symbol, lc) -
				   __calc_left_pad(flags, currency_symbol, lc);
			if (pad_size < 0)
				pad_size = 0;
		}

		asciivalue = __format_grouped_double(value, &flags,
				left_prec, right_prec, pad_char, lc, loc);
		if (asciivalue == NULL)
			goto end_error;		/* errno already set     */
						/* to ENOMEM by malloc() */

		/* set some variables for later use */
		__setup_vars(flags, &cs_precedes, &sep_by_space,
				&sign_posn, &signstr, lc);

		/*
		 * Description of some LC_MONETARY's values:
		 *
		 * p_cs_precedes & n_cs_precedes
		 *
		 * = 1 - $currency_symbol precedes the value
		 *       for a monetary quantity with a non-negative value
		 * = 0 - symbol succeeds the value
		 *
		 * p_sep_by_space & n_sep_by_space
                 *
		 * = 0 - no space separates $currency_symbol
		 *       from the value for a monetary quantity with a
		 *	 non-negative value
		 * = 1 - space separates the symbol from the value
		 * = 2 - space separates the symbol and the sign string,
		 *       if adjacent.
                 *
		 * p_sign_posn & n_sign_posn
                 *
		 * = 0 - parentheses enclose the quantity and the
		 *	 $currency_symbol
		 * = 1 - the sign string precedes the quantity and the 
		 *       $currency_symbol
		 * = 2 - the sign string succeeds the quantity and the 
		 *       $currency_symbol
		 * = 3 - the sign string precedes the $currency_symbol
		 * = 4 - the sign string succeeds the $currency_symbol
                 *
		 */

		tmpptr = dst;

		while (pad_size-- > 0)
			PRINT(' ');

		if (sign_posn == 0 && (flags & IS_NEGATIVE))
			PRINT('(');

		if (cs_precedes == 1) {
			if (sign_posn == 1 || sign_posn == 3) {
				PRINTS(signstr);
				if (sep_by_space == 2)		/* XXX: ? */
					PRINT(' ');
			}

			if (!(flags & SUPRESS_CURR_SYMBOL)) {
				PRINTS(currency_symbol);

				if (sign_posn == 4) {
					if (sep_by_space == 2)
						PRINT(space_char);
					PRINTS(signstr);
					if (sep_by_space == 1)
						PRINT(' ');
				} else if (sep_by_space == 1)
					PRINT(space_char);
			}
		} else if (sign_posn == 1) {
			PRINTS(signstr);
			if (sep_by_space == 2)
				PRINT(' ');
		}

		PRINTS(asciivalue);

		if (cs_precedes == 0) {
			if (sign_posn == 3) {
				if (sep_by_space == 1)
					PRINT(' ');
				PRINTS(signstr);
			}

			if (!(flags & SUPRESS_CURR_SYMBOL)) {
				if ((sign_posn == 3 && sep_by_space == 2)
				    || (sep_by_space == 1
				    && (sign_posn == 0
				    || sign_posn == 1
				    || sign_posn == 2
				    || sign_posn == 4)))
					PRINT(space_char);
				PRINTS(currency_symbol); /* XXX: len */
				if (sign_posn == 4) {
					if (sep_by_space == 2)
						PRINT(' ');
					PRINTS(signstr);
				}
			}
		}

		if (sign_posn == 2) {
			if (sep_by_space == 2)
				PRINT(' ');
			PRINTS(signstr);
		}

		if (sign_posn == 0) {
			if (flags & IS_NEGATIVE)
				PRINT(')');
			else if (left_prec >= 0)
				PRINT(' ');
		}

		if (dst - tmpptr < width) {
			if (flags & LEFT_JUSTIFY) {
				while (dst - tmpptr < width)
					PRINT(' ');
			} else {
				pad_size = dst-tmpptr;
				memmove(tmpptr + width-pad_size, tmpptr,
				    (size_t) pad_size);
				memset(tmpptr, ' ', (size_t) width-pad_size);
				dst += width-pad_size;
			}
		}
	}

	PRINT('\0');
	free(asciivalue);
	free(currency_symbol);
	return (dst - s - 1);	/* return size of put data except trailing '\0' */

e2big_error:
	errno = E2BIG;
	goto end_error;

format_error:
	errno = EINVAL;

end_error:
	sverrno = errno;
	if (asciivalue != NULL)
		free(asciivalue);
	if (currency_symbol != NULL)
		free(currency_symbol);
	errno = sverrno;
	return (-1);
}

static void
__setup_vars(int flags, char *cs_precedes, char *sep_by_space,
		char *sign_posn, const char **signstr, struct lconv *lc) {

	if ((flags & IS_NEGATIVE) && (flags & USE_INTL_CURRENCY)) {
		*cs_precedes = lc->int_n_cs_precedes;
		*sep_by_space = lc->int_n_sep_by_space;
		*sign_posn = (flags & PARENTH_POSN) ? 0 : lc->int_n_sign_posn;
		*signstr = (lc->negative_sign == '\0') ? "-"
		    : lc->negative_sign;
	} else if (flags & USE_INTL_CURRENCY) {
		*cs_precedes = lc->int_p_cs_precedes;
		*sep_by_space = lc->int_p_sep_by_space;
		*sign_posn = (flags & PARENTH_POSN) ? 0 : lc->int_p_sign_posn;
		*signstr = lc->positive_sign;
	} else if (flags & IS_NEGATIVE) {
		*cs_precedes = lc->n_cs_precedes;
		*sep_by_space = lc->n_sep_by_space;
		*sign_posn = (flags & PARENTH_POSN) ? 0 : lc->n_sign_posn;
		*signstr = (lc->negative_sign == '\0') ? "-"
		    : lc->negative_sign;
	} else {
		*cs_precedes = lc->p_cs_precedes;
		*sep_by_space = lc->p_sep_by_space;
		*sign_posn = (flags & PARENTH_POSN) ? 0 : lc->p_sign_posn;
		*signstr = lc->positive_sign;
	}

	/* Set defult values for unspecified information. */
	if (*cs_precedes != 0)
		*cs_precedes = 1;
	if (*sep_by_space == CHAR_MAX)
		*sep_by_space = 0;
	if (*sign_posn == CHAR_MAX)
		*sign_posn = 0;
}

static int
__calc_left_pad(int flags, char *cur_symb, struct lconv *lc) {

	char cs_precedes, sep_by_space, sign_posn;
	const char *signstr;
	int left_chars = 0;

	__setup_vars(flags, &cs_precedes, &sep_by_space, &sign_posn, &signstr, lc);

	if (cs_precedes != 0) {
		left_chars += strlen(cur_symb);
		if (sep_by_space != 0)
			left_chars++;
	}

	switch (sign_posn) {
		case 0:
			if (flags & IS_NEGATIVE)
				left_chars++;
			break;
		case 1:
			left_chars += strlen(signstr);
			break;
		case 3:
		case 4:
			if (cs_precedes != 0)
				left_chars += strlen(signstr);
	}
	return (left_chars);
}

static int
get_groups(int size, const char *grouping) {

	int	chars = 0;

	if (*grouping == CHAR_MAX || *grouping <= 0)	/* no grouping ? */
		return (0);

	while (size > (int)*grouping) {
		chars++;
		size -= (int)*grouping++;
		/* no more grouping ? */
		if (*grouping == CHAR_MAX)
			break;
		/* rest grouping with same value ? */
		if (*grouping == 0) {
			chars += (size - 1) / *(grouping - 1);
			break;
		}
	}
	return (chars);
}

/* convert double to ASCII */
__private_extern__ const char *__fix_nogrouping(const char *);

static char *
__format_grouped_double(double value, int *flags,
			int left_prec, int right_prec, int pad_char, struct lconv *lc, locale_t loc) {

	char		*rslt;
	char		*avalue;
	int		avalue_size;
	char		fmt[32];

	size_t		bufsize;
	char		*bufend;

	int		padded;

	const char	*grouping;
	char		decimal_point;
	char		thousands_sep;

	int groups = 0;

	grouping = __fix_nogrouping(lc->mon_grouping);
	decimal_point = *lc->mon_decimal_point;
	if (decimal_point == '\0')
		decimal_point = *lc->decimal_point;
	thousands_sep = *lc->mon_thousands_sep;
	if (thousands_sep == '\0')
		thousands_sep = *lc->thousands_sep;

	/* fill left_prec with default value */
	if (left_prec == -1)
		left_prec = 0;

	/* fill right_prec with default value */
	if (right_prec == -1) {
                if (*flags & USE_INTL_CURRENCY)
                        right_prec = lc->int_frac_digits;
                else
                        right_prec = lc->frac_digits;

		if (right_prec == CHAR_MAX)	/* POSIX locale ? */
			right_prec = 2;
	}

	if (*flags & NEED_GROUPING)
		left_prec += get_groups(left_prec, grouping);

	/* convert to string */
	snprintf_l(fmt, sizeof(fmt), loc, "%%%d.%df", left_prec + right_prec + 1,
	    right_prec);
	avalue_size = asprintf_l(&avalue, loc, fmt, value);
	if (avalue_size < 0)
		return (NULL);

	/* make sure that we've enough space for result string */
	bufsize = strlen(avalue)*2+1;
	rslt = malloc(bufsize);
	if (rslt == NULL) {
		free(avalue);
		return (NULL);
	}
	memset(rslt, 0, bufsize);
	bufend = rslt + bufsize - 1;	/* reserve space for trailing '\0' */

	/* skip spaces at beggining */
	padded = 0;
	while (avalue[padded] == ' ') {
		padded++;
		avalue_size--;
	}

	if (right_prec > 0) {
		bufend -= right_prec;
		memcpy(bufend, avalue + avalue_size+padded-right_prec,
		    (size_t) right_prec);
		*--bufend = decimal_point;
		avalue_size -= (right_prec + 1);
	}

	if ((*flags & NEED_GROUPING) &&
	    thousands_sep != '\0' &&	/* XXX: need investigation */
	    *grouping != CHAR_MAX &&
	    *grouping > 0) {
		while (avalue_size > (int)*grouping) {
			GRPCPY(*grouping);
			GRPSEP;
			grouping++;

			/* no more grouping ? */
			if (*grouping == CHAR_MAX)
				break;

			/* rest grouping with same value ? */
			if (*grouping == 0) {
				grouping--;
				while (avalue_size > *grouping) {
					GRPCPY(*grouping);
					GRPSEP;
				}
			}
		}
		if (avalue_size != 0)
			GRPCPY(avalue_size);
		padded -= groups;

	} else {
		bufend -= avalue_size;
		memcpy(bufend, avalue+padded, (size_t) avalue_size);
		if (right_prec == 0)
			padded--;	/* decrease assumed $decimal_point */
	}

	/* do padding with pad_char */
	if (padded > 0) {
		bufend -= padded;
		memset(bufend, pad_char, (size_t) padded);
	}

	bufsize = bufsize - (bufend - rslt) + 1;
	memmove(rslt, bufend, bufsize);
	free(avalue);
	return (rslt);
}

ssize_t
strfmon(char * __restrict s, size_t maxsize, const char * __restrict format,
    ...)
{
	ssize_t		ret;
	va_list		ap;

	va_start(ap, format);
	ret = _strfmon(s, maxsize, __current_locale(), format, ap);
	va_end(ap);
	return ret;
}

ssize_t
strfmon_l(char * __restrict s, size_t maxsize, locale_t loc,
    const char * __restrict format, ...)
{
	ssize_t		ret;
	va_list		ap;

	NORMALIZE_LOCALE(loc);
	va_start(ap, format);
	ret = _strfmon(s, maxsize, loc, format, ap);
	va_end(ap);
	return ret;
}
