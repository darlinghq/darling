/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcomma"

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)vfscanf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/vfscanf.c,v 1.43 2009/01/19 06:19:51 das Exp $");

#include "xlocale_private.h"

#include "namespace.h"
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <pthread.h>
#include "un-namespace.h"

#include "collate.h"
#include "libc_private.h"
#include "local.h"

#ifndef NO_FLOATING_POINT
#include <locale.h>
#endif

#define	BUF		513	/* Maximum length of numeric string. */

/*
 * Flags used during conversion.
 */
#define	LONG		0x01	/* l: long or double */
#define	LONGDBL		0x02	/* L: long double */
#define	SHORT		0x04	/* h: short */
#define	SUPPRESS	0x08	/* *: suppress assignment */
#define	POINTER		0x10	/* p: void * (as hex) */
#define	NOSKIP		0x20	/* [ or c: do not skip blanks */
#define	LONGLONG	0x400	/* ll: long long (+ deprecated q: quad) */
#define	INTMAXT		0x800	/* j: intmax_t */
#define	PTRDIFFT	0x1000	/* t: ptrdiff_t */
#define	SIZET		0x2000	/* z: size_t */
#define	SHORTSHORT	0x4000	/* hh: char */
#define	UNSIGNED	0x8000	/* %[oupxX] conversions */

/*
 * The following are used in integral conversions only:
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS
 */
#define	SIGNOK		0x40	/* +/- is (still) legal */
#define	NDIGITS		0x80	/* no digits detected */
#define	PFXOK		0x100	/* 0x prefix is (still) legal */
#define	NZDIGITS	0x200	/* no zero digits detected */
#define	HAVESIGN	0x10000	/* sign detected */

/*
 * Conversion types.
 */
#define	CT_CHAR		0	/* %c conversion */
#define	CT_CCL		1	/* %[...] conversion */
#define	CT_STRING	2	/* %s conversion */
#define	CT_INT		3	/* %[dioupxX] conversion */
#define	CT_FLOAT	4	/* %[efgEFG] conversion */

static const u_char *__sccl(char *, const u_char *, locale_t);
#ifndef NO_FLOATING_POINT
static int parsefloat(FILE *, char **, size_t, locale_t);
#endif

__weak_reference(__vfscanf, vfscanf);

/*
 * __vfscanf - MT-safe version
 */
int
__vfscanf(FILE * __restrict fp, char const * __restrict fmt0, va_list ap)
{
	int ret;

	FLOCKFILE(fp);
	ret = __svfscanf_l(fp, __current_locale(), fmt0, ap);
	FUNLOCKFILE(fp);
	return (ret);
}

int
vfscanf_l(FILE * __restrict fp, locale_t loc, char const * __restrict fmt0, va_list ap)
{
	int ret;

	NORMALIZE_LOCALE(loc);
	FLOCKFILE(fp);
	ret = __svfscanf_l(fp, loc, fmt0, ap);
	FUNLOCKFILE(fp);
	return (ret);
}

/*
 * __svfscanf - non-MT-safe version of __vfscanf
 */
__private_extern__ int
__svfscanf_l(FILE * __restrict fp, locale_t loc, const char * __restrict fmt0, va_list ap)
{
	const u_char *fmt = (const u_char *)fmt0;
	int c;			/* character from format, or conversion */
	size_t width;		/* field width, or 0 */
	char *p;		/* points into all kinds of strings */
	int n;			/* handy integer */
	int flags;		/* flags as defined above */
	char *p0;		/* saves original value of p when necessary */
	int nassigned;		/* number of fields assigned */
	int nread;		/* number of characters consumed from fp */
	int base;		/* base argument to conversion function */
	char ccltab[256];	/* character class table for %[...] */
	char buf[BUF];		/* buffer for numeric and mb conversions */
	wchar_t *wcp;		/* handy wide character pointer */
	size_t nconv;		/* length of multibyte sequence converted */
	int index;		/* %index$, zero if unset */
	va_list ap_orig;	/* to reset ap to first argument */
	static const mbstate_t initial;
	mbstate_t mbs;
	int mb_cur_max;

	/* `basefix' is used to avoid `if' tests in the integer scanner */
	static const short basefix[17] =
		{ 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

	NORMALIZE_LOCALE(loc);
	mb_cur_max = MB_CUR_MAX_L(loc);
	ORIENT(fp, -1);

	nassigned = 0;
	nread = 0;
	va_copy(ap_orig, ap);
	for (;;) {
		c = *fmt++;
		if (c == 0)
			return (nassigned);
		if (isspace_l(c, loc)) {
			while ((fp->_r > 0 || __srefill(fp) == 0) && isspace_l(*fp->_p, loc))
				nread++, fp->_r--, fp->_p++;
			continue;
		}
		if (c != '%') {
			if (fp->_r <= 0 && __srefill(fp))
				goto input_failure;
			goto literal;
		}
		width = 0;
		flags = 0;
		/*
		 * switch on the format.  continue if done;
		 * break once format type is derived.
		 */
again:		c = *fmt++;
		switch (c) {
		case '%':
			/* Consume leading white space */
			for(;;) {
				if (fp->_r <= 0 && __srefill(fp))
					goto input_failure;
				if (!isspace_l(*fp->_p, loc))
					break;
				nread++;
				fp->_r--;
				fp->_p++;
			}
literal:
			if (*fp->_p != c)
				goto match_failure;
			fp->_r--, fp->_p++;
			nread++;
			continue;

		case '$':
			index = width;
			if (index < 1 || index > NL_ARGMAX || fmt[-3] != '%') {
				goto input_failure;
			}
			width = 0;
			va_end(ap);
			va_copy(ap, ap_orig); /* reset to %1$ */
			for (; index > 1; index--) {
				va_arg(ap, void*);
			}
			goto again;
		case '*':
			flags |= SUPPRESS;
			goto again;
		case 'j':
			flags |= INTMAXT;
			goto again;
		case 'l':
			if (flags & LONG) {
				flags &= ~LONG;
				flags |= LONGLONG;
			} else
				flags |= LONG;
			goto again;
		case 'q':
			flags |= LONGLONG;	/* not quite */
			goto again;
		case 't':
			flags |= PTRDIFFT;
			goto again;
		case 'z':
			flags |= SIZET;
			goto again;
		case 'L':
			flags |= LONGDBL;
			goto again;
		case 'h':
			if (flags & SHORT) {
				flags &= ~SHORT;
				flags |= SHORTSHORT;
			} else
				flags |= SHORT;
			goto again;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			width = width * 10 + c - '0';
			goto again;

		/*
		 * Conversions.
		 */
		case 'd':
			c = CT_INT;
			base = 10;
			break;

		case 'i':
			c = CT_INT;
			base = 0;
			break;

		case 'o':
			c = CT_INT;
			flags |= UNSIGNED;
			base = 8;
			break;

		case 'u':
			c = CT_INT;
			flags |= UNSIGNED;
			base = 10;
			break;

		case 'X':
		case 'x':
			flags |= PFXOK;	/* enable 0x prefixing */
			c = CT_INT;
			flags |= UNSIGNED;
			base = 16;
			break;

#ifndef NO_FLOATING_POINT
		case 'A': case 'E': case 'F': case 'G':
		case 'a': case 'e': case 'f': case 'g':
			c = CT_FLOAT;
			break;
#endif

		case 'S':
			flags |= LONG;
			/* FALLTHROUGH */
		case 's':
			c = CT_STRING;
			break;

		case '[':
			fmt = __sccl(ccltab, fmt, loc);
			flags |= NOSKIP;
			c = CT_CCL;
			break;

		case 'C':
			flags |= LONG;
			/* FALLTHROUGH */
		case 'c':
			flags |= NOSKIP;
			c = CT_CHAR;
			break;

		case 'p':	/* pointer format is like hex */
			flags |= POINTER | PFXOK;
			c = CT_INT;		/* assumes sizeof(uintmax_t) */
			flags |= UNSIGNED;	/*      >= sizeof(uintptr_t) */
			base = 16;
			break;

		case 'n':
		{
			if (flags & SUPPRESS)	/* ??? */
				continue;
			void *ptr = va_arg(ap, void *);
			if (ptr == NULL)
				continue;
			else if (flags & SHORTSHORT)
				*(char *)ptr = nread;
			else if (flags & SHORT)
				*(short *)ptr = nread;
			else if (flags & LONG)
				*(long *)ptr = nread;
			else if (flags & LONGLONG)
				*(long long *)ptr = nread;
			else if (flags & INTMAXT)
				*(intmax_t *)ptr = nread;
			else if (flags & SIZET)
				*(size_t *)ptr = nread;
			else if (flags & PTRDIFFT)
				*(ptrdiff_t *)ptr = nread;
			else
				*(int *)ptr = nread;
			continue;
		}
		default:
			goto match_failure;

		/*
		 * Disgusting backwards compatibility hack.	XXX
		 */
		case '\0':	/* compat */
			return (EOF);
		}

		/*
		 * We have a conversion that requires input.
		 */
		if (fp->_r <= 0 && __srefill(fp))
			goto input_failure;

		/*
		 * Consume leading white space, except for formats
		 * that suppress this.
		 */
		if ((flags & NOSKIP) == 0) {
			while (isspace_l(*fp->_p, loc)) {
				nread++;
				if (--fp->_r > 0)
					fp->_p++;
				else if (__srefill(fp))
					goto input_failure;
			}
			/*
			 * Note that there is at least one character in
			 * the buffer, so conversions that do not set NOSKIP
			 * ca no longer result in an input failure.
			 */
		}

		/*
		 * Do the conversion.
		 */
		switch (c) {

		case CT_CHAR:
			/* scan arbitrary characters (sets NOSKIP) */
			if (width == 0)
				width = 1;
			if (flags & LONG) {
				if ((flags & SUPPRESS) == 0)
					wcp = va_arg(ap, wchar_t *);
				else
					wcp = NULL;
				n = 0;
				while (width != 0) {
					if (n == mb_cur_max) {
						fp->_flags |= __SERR;
						goto input_failure;
					}
					buf[n++] = *fp->_p;
					fp->_p++;
					fp->_r--;
					mbs = initial;
					nconv = mbrtowc_l(wcp, buf, n, &mbs, loc);
					if (nconv == (size_t)-1) {
						fp->_flags |= __SERR;
						goto input_failure;
					}
					if (nconv == 0 && !(flags & SUPPRESS))
						*wcp = L'\0';
					if (nconv != (size_t)-2) {
						nread += n;
						width--;
						if (!(flags & SUPPRESS))
							wcp++;
						n = 0;
					}
					if (fp->_r <= 0 && __srefill(fp)) {
						if (n != 0) {
							fp->_flags |= __SERR;
							goto input_failure;
						}
						break;
					}
				}
				if (!(flags & SUPPRESS))
					nassigned++;
			} else if (flags & SUPPRESS) {
				size_t sum = 0;
				for (;;) {
					if ((n = fp->_r) < width) {
						sum += n;
						width -= n;
						fp->_p += n;
						if (__srefill(fp)) {
							if (sum == 0)
							    goto input_failure;
							break;
						}
					} else {
						sum += width;
						fp->_r -= width;
						fp->_p += width;
						break;
					}
				}
				nread += sum;
			} else {
				size_t r = __fread((void *)va_arg(ap, char *), 1,
				    width, fp);

				if (r == 0)
					goto input_failure;
				nread += r;
				nassigned++;
			}
			break;

		case CT_CCL:
			/* scan a (nonempty) character class (sets NOSKIP) */
			if (width == 0)
				width = (size_t)~0;	/* `infinity' */
			/* take only those things in the class */
			if (flags & LONG) {
				wchar_t twc;
				int nchars;

				if ((flags & SUPPRESS) == 0)
					wcp = va_arg(ap, wchar_t *);
				else
					wcp = &twc;
				n = 0;
				nchars = 0;
				while (width != 0) {
					if (n == mb_cur_max) {
						fp->_flags |= __SERR;
						goto input_failure;
					}
					buf[n++] = *fp->_p;
					fp->_p++;
					fp->_r--;
					mbs = initial;
					nconv = mbrtowc_l(wcp, buf, n, &mbs, loc);
					if (nconv == (size_t)-1) {
						fp->_flags |= __SERR;
						goto input_failure;
					}
					if (nconv == 0)
						*wcp = L'\0';
					if (nconv != (size_t)-2) {
						if (wctob_l(*wcp, loc) != EOF &&
						    !ccltab[wctob_l(*wcp, loc)]) {
							while (n != 0) {
								n--;
								__ungetc(buf[n],
								    fp);
							}
							break;
						}
						nread += n;
						width--;
						if (!(flags & SUPPRESS))
							wcp++;
						nchars++;
						n = 0;
					}
					if (fp->_r <= 0 && __srefill(fp)) {
						if (n != 0) {
							fp->_flags |= __SERR;
							goto input_failure;
						}
						break;
					}
				}
				if (n != 0) {
					fp->_flags |= __SERR;
					goto input_failure;
				}
				n = nchars;
				if (n == 0)
					goto match_failure;
				if (!(flags & SUPPRESS)) {
					*wcp = L'\0';
					nassigned++;
				}
			} else if (flags & SUPPRESS) {
				n = 0;
				while (ccltab[*fp->_p]) {
					n++, fp->_r--, fp->_p++;
					if (--width == 0)
						break;
					if (fp->_r <= 0 && __srefill(fp)) {
						if (n == 0)
							goto input_failure;
						break;
					}
				}
				if (n == 0)
					goto match_failure;
			} else {
				p0 = p = va_arg(ap, char *);
				while (ccltab[*fp->_p]) {
					fp->_r--;
					*p++ = *fp->_p++;
					if (--width == 0)
						break;
					if (fp->_r <= 0 && __srefill(fp)) {
						if (p == p0)
							goto input_failure;
						break;
					}
				}
				n = p - p0;
				if (n == 0)
					goto match_failure;
				*p = 0;
				nassigned++;
			}
			nread += n;
			break;

		case CT_STRING:
			/* like CCL, but zero-length string OK, & no NOSKIP */
			if (width == 0)
				width = (size_t)~0;
			if (flags & LONG) {
				wchar_t twc;

				if ((flags & SUPPRESS) == 0)
					wcp = va_arg(ap, wchar_t *);
				else
					wcp = &twc;
				n = 0;
				while (width != 0) {
					if (n == mb_cur_max) {
						fp->_flags |= __SERR;
						goto input_failure;
					}
					buf[n++] = *fp->_p;
					fp->_p++;
					fp->_r--;
					mbs = initial;
					nconv = mbrtowc_l(wcp, buf, n, &mbs, loc);
					if (nconv == (size_t)-1) {
						fp->_flags |= __SERR;
						goto input_failure;
					}
					if (nconv == 0)
						*wcp = L'\0';
					if (nconv != (size_t)-2) {
						if (iswspace_l(*wcp, loc)) {
							while (n != 0) {
								n--;
								__ungetc(buf[n],
								    fp);
							}
							break;
						}
						nread += n;
						width--;
						if (!(flags & SUPPRESS))
							wcp++;
						n = 0;
					}
					if (fp->_r <= 0 && __srefill(fp)) {
						if (n != 0) {
							fp->_flags |= __SERR;
							goto input_failure;
						}
						break;
					}
				}
				if (!(flags & SUPPRESS)) {
					*wcp = L'\0';
					nassigned++;
				}
			} else if (flags & SUPPRESS) {
				n = 0;
				while (!isspace_l(*fp->_p, loc)) {
					n++, fp->_r--, fp->_p++;
					if (--width == 0)
						break;
					if (fp->_r <= 0 && __srefill(fp))
						break;
				}
				nread += n;
			} else {
				p0 = p = va_arg(ap, char *);
				while (!isspace_l(*fp->_p, loc)) {
					fp->_r--;
					*p++ = *fp->_p++;
					if (--width == 0)
						break;
					if (fp->_r <= 0 && __srefill(fp))
						break;
				}
				*p = 0;
				nread += p - p0;
				nassigned++;
			}
			continue;

		case CT_INT:
			/* scan an integer as if by the conversion function */
#ifdef hardway
			if (width == 0 || width > sizeof(buf) - 1)
				width = sizeof(buf) - 1;
#else
			/* size_t is unsigned, hence this optimisation */
			if (--width > sizeof(buf) - 2)
				width = sizeof(buf) - 2;
			width++;
#endif
			flags |= SIGNOK | NDIGITS | NZDIGITS;
			for (p = buf; width; width--) {
				c = *fp->_p;
				/*
				 * Switch on the character; `goto ok'
				 * if we accept it as a part of number.
				 */
				switch (c) {

				/*
				 * The digit 0 is always legal, but is
				 * special.  For %i conversions, if no
				 * digits (zero or nonzero) have been
				 * scanned (only signs), we will have
				 * base==0.  In that case, we should set
				 * it to 8 and enable 0x prefixing.
				 * Also, if we have not scanned zero digits
				 * before this, do not turn off prefixing
				 * (someone else will turn it off if we
				 * have scanned any nonzero digits).
				 */
				case '0':
					if (base == 0) {
						base = 8;
						flags |= PFXOK;
					}
					if (flags & NZDIGITS)
					    flags &= ~(SIGNOK|NZDIGITS|NDIGITS);
					else
					    flags &= ~(SIGNOK|PFXOK|NDIGITS);
					goto ok;

				/* 1 through 7 always legal */
				case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					base = basefix[base];
					flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* digits 8 and 9 ok iff decimal or hex */
				case '8': case '9':
					base = basefix[base];
					if (base <= 8)
						break;	/* not legal here */
					flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* letters ok iff hex */
				case 'A': case 'B': case 'C':
				case 'D': case 'E': case 'F':
				case 'a': case 'b': case 'c':
				case 'd': case 'e': case 'f':
					/* no need to fix base here */
					if (base <= 10)
						break;	/* not legal here */
					flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* sign ok only as first character */
				case '+': case '-':
					if (flags & SIGNOK) {
						flags &= ~SIGNOK;
						flags |= HAVESIGN;
						goto ok;
					}
					break;
					
				/*
				 * x ok iff flag still set & 2nd char (or
				 * 3rd char if we have a sign).
				 */
				case 'x': case 'X':
					if (flags & PFXOK && p ==
					    buf + 1 + !!(flags & HAVESIGN)) {
						base = 16;	/* if %i */
						flags &= ~PFXOK;
						goto ok;
					}
					break;
				}

				/*
				 * If we got here, c is not a legal character
				 * for a number.  Stop accumulating digits.
				 */
				break;
		ok:
				/*
				 * c is legal: store it and look at the next.
				 */
				*p++ = c;
				if (--fp->_r > 0)
					fp->_p++;
				else if (__srefill(fp))
					break;		/* EOF */
			}
			/*
			 * If we had only a sign, it is no good; push
			 * back the sign.  If the number ends in `x',
			 * it was [sign] '0' 'x', so push back the x
			 * and treat it as [sign] '0'.
			 */
			if (flags & NDIGITS) {
				if (p > buf)
					(void) __ungetc(*(u_char *)--p, fp);
				goto match_failure;
			}
			c = ((u_char *)p)[-1];
			if (c == 'x' || c == 'X') {
				--p;
				(void) __ungetc(c, fp);
			}
			if ((flags & SUPPRESS) == 0) {
				uintmax_t res;

				*p = 0;
				if ((flags & UNSIGNED) == 0)
				    res = strtoimax_l(buf, (char **)NULL, base, loc);
				else
				    res = strtoumax_l(buf, (char **)NULL, base, loc);
				if (flags & POINTER)
					*va_arg(ap, void **) =
							(void *)(uintptr_t)res;
				else if (flags & SHORTSHORT)
					*va_arg(ap, char *) = res;
				else if (flags & SHORT)
					*va_arg(ap, short *) = res;
				else if (flags & LONG)
					*va_arg(ap, long *) = res;
				else if (flags & LONGLONG)
					*va_arg(ap, long long *) = res;
				else if (flags & INTMAXT)
					*va_arg(ap, intmax_t *) = res;
				else if (flags & PTRDIFFT)
					*va_arg(ap, ptrdiff_t *) = res;
				else if (flags & SIZET)
					*va_arg(ap, size_t *) = res;
				else
					*va_arg(ap, int *) = res;
				nassigned++;
			}
			nread += p - buf;
			break;

#ifndef NO_FLOATING_POINT
		case CT_FLOAT:
		{
			char *pbuf;
			/* scan a floating point number as if by strtod */
			if ((width = parsefloat(fp, &pbuf, width, loc)) == 0)
				goto match_failure;
			if ((flags & SUPPRESS) == 0) {
				if (flags & LONGDBL) {
					long double res = strtold_l(pbuf, &p, loc);
					*va_arg(ap, long double *) = res;
				} else if (flags & LONG) {
					double res = strtod_l(pbuf, &p, loc);
					*va_arg(ap, double *) = res;
				} else {
					float res = strtof_l(pbuf, &p, loc);
					*va_arg(ap, float *) = res;
				}
				nassigned++;
			}
			nread += width;
			break;
		}
#endif /* !NO_FLOATING_POINT */
		}
	}
input_failure:
	return (nassigned ? nassigned : EOF);
match_failure:
	return (nassigned);
}

int
__svfscanf(FILE * __restrict fp, const char * __restrict fmt0, va_list ap)
{
	return __svfscanf_l(fp, __current_locale(), fmt0, ap);
}

/*
 * Fill in the given table from the scanset at the given format
 * (just after `[').  Return a pointer to the character past the
 * closing `]'.  The table has a 1 wherever characters should be
 * considered part of the scanset.
 */
static const u_char *
__sccl(tab, fmt, loc)
	char *tab;
	const u_char *fmt;
	locale_t loc;
{
	int c, n, v, i;

	/* first `clear' the whole table */
	c = *fmt++;		/* first char hat => negated scanset */
	if (c == '^') {
		v = 1;		/* default => accept */
		c = *fmt++;	/* get new first char */
	} else
		v = 0;		/* default => reject */

	/* XXX: Will not work if sizeof(tab*) > sizeof(char) */
	(void) memset(tab, v, 256);

	if (c == 0)
		return (fmt - 1);/* format ended before closing ] */

	/*
	 * Now set the entries corresponding to the actual scanset
	 * to the opposite of the above.
	 *
	 * The first character may be ']' (or '-') without being special;
	 * the last character may be '-'.
	 */
	v = 1 - v;
	for (;;) {
		tab[c] = v;		/* take character c */
doswitch:
		n = *fmt++;		/* and examine the next */
		switch (n) {

		case 0:			/* format ended too soon */
			return (fmt - 1);

		case '-':
		{
			/*
			 * A scanset of the form
			 *	[01+-]
			 * is defined as `the digit 0, the digit 1,
			 * the character +, the character -', but
			 * the effect of a scanset such as
			 *	[a-zA-Z0-9]
			 * is implementation defined.  The V7 Unix
			 * scanf treats `a-z' as `the letters a through
			 * z', but treats `a-a' as `the letter a, the
			 * character -, and the letter a'.
			 *
			 * For compatibility, the `-' is not considerd
			 * to define a range if the character following
			 * it is either a close bracket (required by ANSI)
			 * or is not numerically greater than the character
			 * we just stored in the table (c).
			 */
			n = *fmt;
			if (n == ']'
			    || (loc->__collate_load_error ? n < c :
				__collate_range_cmp (n, c, loc) < 0
			       )
			   ) {
				c = '-';
				break;	/* resume the for(;;) */
			}
			fmt++;
			/* fill in the range */
			if (loc->__collate_load_error) {
				do {
					tab[++c] = v;
				} while (c < n);
			} else {
				for (i = 0; i < 256; i ++)
					if (   __collate_range_cmp (c, i, loc) < 0
					    && __collate_range_cmp (i, n, loc) <= 0
					   )
						tab[i] = v;
			}
#if 1	/* XXX another disgusting compatibility hack */
			c = n;
			/*
			 * Alas, the V7 Unix scanf also treats formats
			 * such as [a-c-e] as `the letters a through e'.
			 * This too is permitted by the standard....
			 */
			goto doswitch;
#else
			c = *fmt++;
			if (c == 0)
				return (fmt - 1);
			if (c == ']')
				return (fmt);
#endif
			break;
		}
		case ']':		/* end of scanset */
			return (fmt);

		default:		/* just another character */
			c = n;
			break;
		}
	}
	/* NOTREACHED */
}

#ifndef NO_FLOATING_POINT
/*
 * Maintain a per-thread parsefloat buffer, shared by __svfscanf_l and
 * __vfwscanf.
 */
#ifdef BUILDING_VARIANT
extern char *__parsefloat_buf(size_t s);
#else /* !BUILDING_VARIANT */
__private_extern__ char *
__parsefloat_buf(size_t s)
{
	char *b;
	static pthread_key_t    parsefloat_tsd_key = (pthread_key_t)-1;
	static pthread_mutex_t  parsefloat_tsd_lock = PTHREAD_MUTEX_INITIALIZER;
	static size_t bsiz = 0;

	if (parsefloat_tsd_key == (pthread_key_t)-1) {
		pthread_mutex_lock(&parsefloat_tsd_lock);
		if (parsefloat_tsd_key == (pthread_key_t)-1) {
			parsefloat_tsd_key = __LIBC_PTHREAD_KEY_PARSEFLOAT;
			pthread_key_init_np(parsefloat_tsd_key, free);
		}
		pthread_mutex_unlock(&parsefloat_tsd_lock);
	}
	if ((b = (char *)pthread_getspecific(parsefloat_tsd_key)) == NULL) {
		bsiz = s > BUF ? s : BUF;
		b = (char *)malloc(bsiz);
		if (b == NULL) {
			bsiz = 0;
			return NULL;
		}
		pthread_setspecific(parsefloat_tsd_key, b);
		return b;
	}
	if (s > bsiz) {
		b = (char *)reallocf(b, s);
		pthread_setspecific(parsefloat_tsd_key, b);
		if (b == NULL) {
			bsiz = 0;
			return NULL;
		}
		bsiz = s;
	}
	return b;
}
#endif /* BUILDING_VARIANT */

static int
parsefloat(FILE *fp, char **buf, size_t width, locale_t loc)
{
	char *commit, *p;
	int infnanpos = 0, decptpos = 0;
	enum {
		S_START, S_GOTSIGN, S_INF, S_NAN, S_DONE, S_MAYBEHEX,
		S_DIGITS, S_DECPT, S_FRAC, S_EXP, S_EXPDIGITS
	} state = S_START;
	unsigned char c;
	const char *decpt = localeconv_l(loc)->decimal_point;
	_Bool gotmantdig = 0, ishex = 0;
	char *b;
	char *e;
	size_t s;

	s = (width == 0 ? BUF : (width + 1));
	if ((b = __parsefloat_buf(s)) == NULL) {
		*buf = NULL;
		return 0;
	}
	e = b + (s - 1);
	/*
	 * We set commit = p whenever the string we have read so far
	 * constitutes a valid representation of a floating point
	 * number by itself.  At some point, the parse will complete
	 * or fail, and we will ungetc() back to the last commit point.
	 * To ensure that the file offset gets updated properly, it is
	 * always necessary to read at least one character that doesn't
	 * match; thus, we can't short-circuit "infinity" or "nan(...)".
	 */
	commit = b - 1;
	for (p = b; width == 0 || p < e; ) {
		c = *fp->_p;
reswitch:
		switch (state) {
		case S_START:
			state = S_GOTSIGN;
			if (c == '-' || c == '+')
				break;
			else
				goto reswitch;
		case S_GOTSIGN:
			switch (c) {
			case '0':
				state = S_MAYBEHEX;
				commit = p;
				break;
			case 'I':
			case 'i':
				state = S_INF;
				break;
			case 'N':
			case 'n':
				state = S_NAN;
				break;
			default:
				state = S_DIGITS;
				goto reswitch;
			}
			break;
		case S_INF:
			if (infnanpos > 6 ||
			    (c != "nfinity"[infnanpos] &&
			     c != "NFINITY"[infnanpos]))
				goto parsedone;
			if (infnanpos == 1 || infnanpos == 6)
				commit = p;	/* inf or infinity */
			infnanpos++;
			break;
		case S_NAN:
			switch (infnanpos) {
			case 0:
				if (c != 'A' && c != 'a')
					goto parsedone;
				break;
			case 1:
				if (c != 'N' && c != 'n')
					goto parsedone;
				else
					commit = p;
				break;
			case 2:
				if (c != '(')
					goto parsedone;
				break;
			default:
				if (c == ')') {
					commit = p;
					state = S_DONE;
				} else if (!isalnum_l(c, loc) && c != '_')
					goto parsedone;
				break;
			}
			infnanpos++;
			break;
		case S_DONE:
			goto parsedone;
		case S_MAYBEHEX:
			state = S_DIGITS;
			if (c == 'X' || c == 'x') {
				ishex = 1;
				break;
			} else {	/* we saw a '0', but no 'x' */
				gotmantdig = 1;
				goto reswitch;
			}
		case S_DIGITS:
			if ((ishex && isxdigit_l(c, loc)) || isdigit_l(c, loc)) {
				gotmantdig = 1;
				commit = p;
				break;
			} else {
				state = S_DECPT;
				goto reswitch;
			}
		case S_DECPT:
			if (c == decpt[decptpos]) {
				if (decpt[++decptpos] == '\0') {
					/* We read the complete decpt seq. */
					state = S_FRAC;
					if (gotmantdig)
						commit = p;
				}
				break;
			} else if (!decptpos) {
				/* We didn't read any decpt characters. */
				state = S_FRAC;
				goto reswitch;
			} else {
				/*
				 * We read part of a multibyte decimal point,
				 * but the rest is invalid, so bail.
				 */
				goto parsedone;
			}
		case S_FRAC:
			if (((c == 'E' || c == 'e') && !ishex) ||
			    ((c == 'P' || c == 'p') && ishex)) {
				if (!gotmantdig)
					goto parsedone;
				else
					state = S_EXP;
			} else if ((ishex && isxdigit_l(c, loc)) || isdigit_l(c, loc)) {
				commit = p;
				gotmantdig = 1;
			} else
				goto parsedone;
			break;
		case S_EXP:
			state = S_EXPDIGITS;
			if (c == '-' || c == '+')
				break;
			else
				goto reswitch;
		case S_EXPDIGITS:
			if (isdigit_l(c, loc))
				commit = p;
			else
				goto parsedone;
			break;
		default:
			LIBC_ABORT("unknown state %d", state);
		}
		if (p >= e) {
			ssize_t diff = (p - b);
			ssize_t com = (commit - b);
			s += BUF;
			b = __parsefloat_buf(s);
			if (b == NULL) {
				*buf = NULL;
				return 0;
			}
			e = b + (s - 1);
			p = b + diff;
			commit = b + com;
		}
		*p++ = c;
		if (--fp->_r > 0)
			fp->_p++;
		else if (__srefill(fp))
			break;	/* EOF */
	}

parsedone:
	while (commit < --p)
		__ungetc(*(u_char *)p, fp);
	*++commit = '\0';
	*buf = b;
	return (commit - b);
}
#endif
#pragma clang diagnostic push
