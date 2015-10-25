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

#if 0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)vfprintf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/vfwprintf.c,v 1.42 2009/11/25 04:27:55 wollman Exp $");

#include "xlocale_private.h"

/*
 * Actual wprintf innards.
 *
 * Avoid making gratuitous changes to this source file; it should be kept
 * as close as possible to vfprintf.c for ease of maintenance.
 */

#include "namespace.h"
#include <sys/types.h>

#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <errno.h>
#include "un-namespace.h"

#include "libc_private.h"
#include "local.h"
#include "fvwrite.h"
#include "printflocal.h"

static int	__sprint(FILE *, locale_t, struct __suio *);
static int	__sbprintf(FILE *, locale_t, const wchar_t *, va_list);
static wint_t	__xfputwc(wchar_t, FILE *, locale_t);
static wchar_t	*__mbsconv(char *, int, locale_t);
__private_extern__ const char *__fix_nogrouping(const char *);

#define	CHAR	wchar_t
#include "printfcommon.h"

struct grouping_state {
	wchar_t thousands_sep;	/* locale-specific thousands separator */
	const char *grouping;	/* locale-specific numeric grouping rules */
	int lead;		/* sig figs before decimal or group sep */
	int nseps;		/* number of group separators with ' */
	int nrepeats;		/* number of repeats of the last group */
};

static const mbstate_t initial_mbs;

static inline wchar_t
get_decpt(locale_t loc)
{
	mbstate_t mbs;
	wchar_t decpt;
	int nconv;

	mbs = initial_mbs;
	nconv = mbrtowc_l(&decpt, localeconv_l(loc)->decimal_point, MB_CUR_MAX_L(loc), &mbs, loc);
	if (nconv == (size_t)-1 || nconv == (size_t)-2)
		decpt = '.';    /* failsafe */
	return (decpt);
}

static inline wchar_t
get_thousep(locale_t loc)
{
	mbstate_t mbs;
	wchar_t thousep;
	int nconv;

	mbs = initial_mbs;
	nconv = mbrtowc_l(&thousep, localeconv_l(loc)->thousands_sep,
	    MB_CUR_MAX_L(loc), &mbs, loc);
	if (nconv == (size_t)-1 || nconv == (size_t)-2)
		thousep = '\0';    /* failsafe */
	return (thousep);
}

/*
 * Initialize the thousands' grouping state in preparation to print a
 * number with ndigits digits. This routine returns the total number
 * of wide characters that will be printed.
 */
static int
grouping_init(struct grouping_state *gs, int ndigits, locale_t loc)
{

	gs->grouping = __fix_nogrouping(localeconv_l(loc)->grouping);
	gs->thousands_sep = get_thousep(loc);

	gs->nseps = gs->nrepeats = 0;
	gs->lead = ndigits;
	while (*gs->grouping != CHAR_MAX) {
		if (gs->lead <= *gs->grouping)
			break;
		gs->lead -= *gs->grouping;
		if (*(gs->grouping+1)) {
			gs->nseps++;
			gs->grouping++;
		} else
			gs->nrepeats++;
	}
	return (gs->nseps + gs->nrepeats);
}

/*
 * Print a number with thousands' separators.
 */
static int
grouping_print(struct grouping_state *gs, struct io_state *iop,
	       const CHAR *cp, const CHAR *ep, locale_t loc)
{
	const CHAR *cp0 = cp;

	if (io_printandpad(iop, cp, ep, gs->lead, zeroes, loc))
		return (-1);
	cp += gs->lead;
	while (gs->nseps > 0 || gs->nrepeats > 0) {
		if (gs->nrepeats > 0)
			gs->nrepeats--;
		else {
			gs->grouping--;
			gs->nseps--;
		}
		if (io_print(iop, &gs->thousands_sep, 1, loc))
			return (-1);
		if (io_printandpad(iop, cp, ep, *gs->grouping, zeroes, loc))
			return (-1);
		cp += *gs->grouping;
	}
	if (cp > ep)
		cp = ep;
	return (cp - cp0);
}


/*
 * Flush out all the vectors defined by the given uio,
 * then reset it so that it can be reused.
 *
 * XXX The fact that we do this a character at a time and convert to a
 * multibyte character sequence even if the destination is a wide
 * string eclipses the benefits of buffering.
 */
static int
__sprint(FILE *fp, locale_t loc, struct __suio *uio)
{
	struct __siov *iov;
	wchar_t *p;
	int i, len;

	iov = uio->uio_iov;
	for (; uio->uio_resid != 0; uio->uio_resid -= len, iov++) {
		p = (wchar_t *)iov->iov_base;
		len = iov->iov_len;
		for (i = 0; i < len; i++) {
			if (__xfputwc(p[i], fp, loc) == WEOF)
				return (-1);
		}
	}
	uio->uio_iovcnt = 0;
	return (0);
}

/*
 * Helper function for `fprintf to unbuffered unix file': creates a
 * temporary buffer.  We only work on write-only files; this avoids
 * worries about ungetc buffers and so forth.
 */
static int
__sbprintf(FILE *fp, locale_t loc, const wchar_t *fmt, va_list ap)
{
	int ret;
	FILE fake;
	unsigned char buf[BUFSIZ];
	struct __sFILEX ext;
	fake._extra = &ext;
	INITEXTRA(&fake);

	/* XXX This is probably not needed. */
	if (prepwrite(fp) != 0)
		return (EOF);

	/* copy the important variables */
	fake._flags = fp->_flags & ~__SNBF;
	fake._file = fp->_file;
	fake._cookie = fp->_cookie;
	fake._write = fp->_write;
	fake._orientation = fp->_orientation;
	fake._mbstate = fp->_mbstate;

	/* set up the buffer */
	fake._bf._base = fake._p = buf;
	fake._bf._size = fake._w = sizeof(buf);
	fake._lbfsize = 0;	/* not actually used, but Just In Case */

	/* do the work, then copy any error status */
	ret = __vfwprintf(&fake, loc, fmt, ap);
	if (ret >= 0 && __fflush(&fake))
		ret = WEOF;
	if (fake._flags & __SERR)
		fp->_flags |= __SERR;
	return (ret);
}

/*
 * Like __fputwc, but handles fake string (__SSTR) files properly.
 * File must already be locked.
 */
static wint_t
__xfputwc(wchar_t wc, FILE *fp, locale_t loc)
{
	mbstate_t mbs;
	char buf[MB_LEN_MAX];
	struct __suio uio;
	struct __siov iov;
	size_t len;

	if ((fp->_flags & __SSTR) == 0)
		return (__fputwc(wc, fp, loc));

	mbs = initial_mbs;
	if ((len = wcrtomb_l(buf, wc, &mbs, loc)) == (size_t)-1) {
		fp->_flags |= __SERR;
		return (WEOF);
	}
	uio.uio_iov = &iov;
	uio.uio_resid = len;
	uio.uio_iovcnt = 1;
	iov.iov_base = buf;
	iov.iov_len = len;
	return (__sfvwrite(fp, &uio) != EOF ? (wint_t)wc : WEOF);
}

/*
 * Convert a multibyte character string argument for the %s format to a wide
 * string representation. ``prec'' specifies the maximum number of bytes
 * to output. If ``prec'' is greater than or equal to zero, we can't assume
 * that the multibyte char. string ends in a null character.
 */
static wchar_t *
__mbsconv(char *mbsarg, int prec, locale_t loc)
{
	mbstate_t mbs;
	wchar_t *convbuf, *wcp;
	const char *p;
	size_t insize, nchars, nconv = 0;
	int mb_cur_max = MB_CUR_MAX_L(loc);

	if (mbsarg == NULL)
		return (NULL);

	/*
	 * Supplied argument is a multibyte string; convert it to wide
	 * characters first.
	 */
	if (prec >= 0) {
		/*
		 * String is not guaranteed to be NUL-terminated. Find the
		 * number of characters to print.
		 */
		p = mbsarg;
		insize = nchars = nconv = 0;
		mbs = initial_mbs;
		while (nchars != (size_t)prec) {
			nconv = mbrlen_l(p, mb_cur_max, &mbs, loc);
			if (nconv == 0 || nconv == (size_t)-1 ||
			    nconv == (size_t)-2)
				break;
			p += nconv;
			nchars++;
			insize += nconv;
		}
		if (nconv == (size_t)-1 || nconv == (size_t)-2)
			return (NULL);
	} else {
		insize = strlen(mbsarg);
		nconv = 0;
	}

	/*
	 * Allocate buffer for the result and perform the conversion,
	 * converting at most `size' bytes of the input multibyte string to
	 * wide characters for printing.
	 */
	convbuf = malloc((insize + 1) * sizeof(*convbuf));
	if (convbuf == NULL)
		return (NULL);
	wcp = convbuf;
	p = mbsarg;
	mbs = initial_mbs;
	while (insize != 0) {
		nconv = mbrtowc_l(wcp, p, insize, &mbs, loc);
		if (nconv == 0 || nconv == (size_t)-1 || nconv == (size_t)-2)
			break;
		wcp++;
		p += nconv;
		insize -= nconv;
	}
	if (nconv == (size_t)-1 || nconv == (size_t)-2) {
		free(convbuf);
		return (NULL);
	}
	*wcp = L'\0';

	return (convbuf);
}

/*
 * MT-safe version
 */
int
vfwprintf_l(FILE * __restrict fp, locale_t loc, const wchar_t * __restrict fmt0, va_list ap)
{
	int ret;

	NORMALIZE_LOCALE(loc);
	FLOCKFILE(fp);
	/* optimise fprintf(stderr) (and other unbuffered Unix files) */
	if ((fp->_flags & (__SNBF|__SWR|__SRW)) == (__SNBF|__SWR) &&
	    fp->_file >= 0)
		ret = __sbprintf(fp, loc, fmt0, ap);
	else
		ret = __vfwprintf(fp, loc, fmt0, ap);
	FUNLOCKFILE(fp);
	return (ret);
}

int
vfwprintf(FILE * __restrict fp, const wchar_t * __restrict fmt0, va_list ap)
{
	return vfwprintf_l(fp, __current_locale(), fmt0, ap);
}

/*
 * The size of the buffer we use as scratch space for integer
 * conversions, among other things.  We need enough space to
 * write a uintmax_t in octal (plus one byte).
 */
#if UINTMAX_MAX <= UINT64_MAX
#define	BUF	32
#else
#error "BUF must be large enough to format a uintmax_t"
#endif

/*
 * Non-MT-safe version
 */
__private_extern__ int
__vfwprintf(FILE *fp, locale_t loc, const wchar_t *fmt0, va_list ap)
{
	wchar_t *fmt;		/* format string */
	wchar_t ch;		/* character from fmt */
	int n, n2;		/* handy integer (short term usage) */
	wchar_t *cp;		/* handy char pointer (short term usage) */
	int flags;		/* flags as above */
	int ret;		/* return value accumulator */
	int width;		/* width from format (%8d), or 0 */
	int prec;		/* precision from format; <0 for N/A */
	wchar_t sign;		/* sign prefix (' ', '+', '-', or \0) */
	struct grouping_state gs; /* thousands' grouping info */
#ifndef NO_FLOATING_POINT
	/*
	 * We can decompose the printed representation of floating
	 * point numbers into several parts, some of which may be empty:
	 *
	 * [+|-| ] [0x|0X] MMM . NNN [e|E|p|P] [+|-] ZZ
	 *    A       B     ---C---      D       E   F
	 *
	 * A:	'sign' holds this value if present; '\0' otherwise
	 * B:	ox[1] holds the 'x' or 'X'; '\0' if not hexadecimal
	 * C:	cp points to the string MMMNNN.  Leading and trailing
	 *	zeros are not in the string and must be added.
	 * D:	expchar holds this character; '\0' if no exponent, e.g. %f
	 * F:	at least two digits for decimal, at least one digit for hex
	 */
	wchar_t decimal_point;	/* locale specific decimal point */
	int signflag;		/* true if float is negative */
	union {			/* floating point arguments %[aAeEfFgG] */
		double dbl;
		long double ldbl;
	} fparg;
	int expt;		/* integer value of exponent */
	char expchar;		/* exponent character: [eEpP\0] */
	char *dtoaend;		/* pointer to end of converted digits */
	int expsize;		/* character count for expstr */
	int ndig;		/* actual number of digits returned by dtoa */
	wchar_t expstr[MAXEXPDIG+2];	/* buffer for exponent string: e+ZZZ */
	char *dtoaresult;	/* buffer allocated by dtoa */
#endif
#ifdef VECTORS
	union arg vval;		/* Vector argument. */
	wchar_t *pct;		/* Pointer to '%' at beginning of specifier. */
	wchar_t vsep;		/* Vector separator character. */
#endif
	u_long	ulval;		/* integer arguments %[diouxX] */
	uintmax_t ujval;	/* %j, %ll, %q, %t, %z integers */
	int base;		/* base for [diouxX] conversion */
	int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
	int realsz;		/* field size expanded by dprec, sign, etc */
	int size;		/* size of converted field or string */
	int prsize;             /* max size of printed field */
	const char *xdigs;	/* digits for [xX] conversion */
	struct io_state io;	/* I/O buffering state */
	wchar_t buf[BUF];	/* buffer with space for digits of uintmax_t */
	wchar_t ox[2];		/* space for 0x hex-prefix */
	union arg *argtable;	/* args, built due to positional arg */
	union arg statargtable [STATIC_ARG_TBL_SIZE];
	int nextarg;		/* 1-based argument index */
	va_list orgap;		/* original argument pointer */
	wchar_t *convbuf;	/* multibyte to wide conversion result */

	static const char xdigs_lower[16] = "0123456789abcdef";
	static const char xdigs_upper[16] = "0123456789ABCDEF";

	/* BEWARE, these `goto error' on error. */
#define	PRINT(ptr, len)	do {			\
	if (io_print(&io, (ptr), (len), loc))	\
		goto error; \
} while (0)
#define	PAD(howmany, with) { \
	if (io_pad(&io, (howmany), (with), loc)) \
		goto error; \
}
#define	PRINTANDPAD(p, ep, len, with) {	\
	if (io_printandpad(&io, (p), (ep), (len), (with), loc)) \
		goto error; \
}
#define	FLUSH() { \
	if (io_flush(&io, loc)) \
		goto error; \
}

	/*
	 * Get the argument indexed by nextarg.   If the argument table is
	 * built, use it to get the argument.  If its not, get the next
	 * argument (and arguments must be gotten sequentially).
	 */
#define GETARG(type) \
	((argtable != NULL) ? *((type*)(&argtable[nextarg++])) : \
	    (nextarg++, va_arg(ap, type)))

	/*
	 * To extend shorts properly, we need both signed and unsigned
	 * argument extraction methods.
	 */
#define	SARG() \
	(flags&LONGINT ? GETARG(long) : \
	    flags&SHORTINT ? (long)(short)GETARG(int) : \
	    flags&CHARINT ? (long)(signed char)GETARG(int) : \
	    (long)GETARG(int))
#define	UARG() \
	(flags&LONGINT ? GETARG(u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)GETARG(int) : \
	    flags&CHARINT ? (u_long)(u_char)GETARG(int) : \
	    (u_long)GETARG(u_int))
#define	INTMAX_SIZE	(INTMAXT|SIZET|PTRDIFFT|LLONGINT)
#define SJARG() \
	(flags&INTMAXT ? GETARG(intmax_t) : \
	    flags&SIZET ? (intmax_t)GETARG(ssize_t) : \
	    flags&PTRDIFFT ? (intmax_t)GETARG(ptrdiff_t) : \
	    (intmax_t)GETARG(long long))
#define	UJARG() \
	(flags&INTMAXT ? GETARG(uintmax_t) : \
	    flags&SIZET ? (uintmax_t)GETARG(size_t) : \
	    flags&PTRDIFFT ? (uintmax_t)(unsigned long)GETARG(ptrdiff_t) : \
	    (uintmax_t)GETARG(unsigned long long))

	/*
	 * Get * arguments, including the form *nn$.  Preserve the nextarg
	 * that the argument can be gotten once the type is determined.
	 */
#define GETASTER(val) \
	n2 = 0; \
	cp = fmt; \
	while (is_digit(*cp)) { \
		n2 = 10 * n2 + to_digit(*cp); \
		cp++; \
	} \
	if (*cp == '$') { \
		int hold = nextarg; \
		if (argtable == NULL) { \
			argtable = statargtable; \
			if (__find_warguments (fmt0, orgap, &argtable)) { \
				ret = EOF; \
				goto error; \
			} \
		} \
		nextarg = n2; \
		val = GETARG (int); \
		nextarg = hold; \
		fmt = ++cp; \
	} else { \
		val = GETARG (int); \
	}


	/* sorry, fwprintf(read_only_file, L"") returns WEOF, not 0 */
	if (prepwrite(fp) != 0) {
		errno = EBADF;
		return (EOF);
	}
	ORIENT(fp, 1);

	convbuf = NULL;
	fmt = (wchar_t *)fmt0;
	argtable = NULL;
	nextarg = 1;
	va_copy(orgap, ap);
	io_init(&io, fp);
	ret = 0;
#ifndef NO_FLOATING_POINT
	decimal_point = get_decpt(loc);
#endif

	/*
	 * Scan the format for conversions (`%' character).
	 */
	for (;;) {
		for (cp = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
			/* void */;
		if ((n = fmt - cp) != 0) {
			if ((unsigned)ret + n > INT_MAX) {
				ret = EOF;
				goto error;
			}
			PRINT(cp, n);
			ret += n;
		}
		if (ch == '\0')
			goto done;
#ifdef VECTORS
		pct = fmt;
#endif /* VECTORS */
		fmt++;		/* skip over '%' */

		flags = 0;
		dprec = 0;
		width = 0;
		prec = -1;
		gs.grouping = NULL;
		sign = '\0';
		ox[1] = '\0';
#ifdef VECTORS
		vsep = 'X'; /* Illegal value, changed to defaults later. */
#endif /* VECTORS */

rflag:		ch = *fmt++;
reswitch:	switch (ch) {
		case ' ':
			/*-
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = ' ';
			goto rflag;
		case '#':
			flags |= ALT;
			goto rflag;
#ifdef VECTORS
		case ',': case ';': case ':': case '_':
			vsep = ch;
			goto rflag;
#endif /* VECTORS */
		case '*':
			/*-
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			GETASTER (width);
			if (width >= 0)
				goto rflag;
			width = -width;
			/* FALLTHROUGH */
		case '-':
			flags |= LADJUST;
			goto rflag;
		case '+':
			sign = '+';
			goto rflag;
		case '\'':
			flags |= GROUPING;
			goto rflag;
		case '.':
			if ((ch = *fmt++) == '*') {
				GETASTER (prec);
				goto rflag;
			}
			prec = 0;
			while (is_digit(ch)) {
				prec = 10 * prec + to_digit(ch);
				ch = *fmt++;
			}
			goto reswitch;
		case '0':
			/*-
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			flags |= ZEROPAD;
			goto rflag;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n = 0;
			do {
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			} while (is_digit(ch));
			if (ch == '$') {
				nextarg = n;
				if (argtable == NULL) {
					argtable = statargtable;
					if (__find_warguments (fmt0, orgap,
							       &argtable)) {
						ret = EOF;
						goto error;
					}
				}
				goto rflag;
			}
			width = n;
			goto reswitch;
#ifndef NO_FLOATING_POINT
		case 'L':
			flags |= LONGDBL;
			goto rflag;
#endif
		case 'h':
			if (flags & SHORTINT) {
				flags &= ~SHORTINT;
				flags |= CHARINT;
			} else
				flags |= SHORTINT;
			goto rflag;
		case 'j':
			flags |= INTMAXT;
			goto rflag;
		case 'l':
			if (flags & LONGINT) {
				flags &= ~LONGINT;
				flags |= LLONGINT;
			} else
				flags |= LONGINT;
			goto rflag;
		case 'q':
			flags |= LLONGINT;	/* not necessarily */
			goto rflag;
		case 't':
			flags |= PTRDIFFT;
			goto rflag;
		case 'z':
			flags |= SIZET;
			goto rflag;
		case 'C':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'c':
#ifdef VECTORS
			if (flags & VECTOR)
				break;
#endif /* VECTORS */
			if (flags & LONGINT)
				*(cp = buf) = (wchar_t)GETARG(wint_t);
			else
				*(cp = buf) = (wchar_t)btowc_l(GETARG(int), loc);
			size = 1;
			sign = '\0';
			break;
		case 'D':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'd':
		case 'i':
#ifdef VECTORS
			if (flags & VECTOR)
				break;
#endif /* VECTORS */
			if (flags & INTMAX_SIZE) {
				ujval = SJARG();
				if ((intmax_t)ujval < 0) {
					ujval = -ujval;
					sign = '-';
				}
			} else {
				ulval = SARG();
				if ((long)ulval < 0) {
					ulval = -ulval;
					sign = '-';
				}
			}
			base = 10;
			goto number;
#ifndef NO_FLOATING_POINT
		case 'a':
		case 'A':
#ifdef VECTORS
			if (flags & VECTOR) {
				flags |= FPT;
				break;
			}
#endif /* VECTORS */
			if (ch == 'a') {
				ox[1] = 'x';
				xdigs = xdigs_lower;
				expchar = 'p';
			} else {
				ox[1] = 'X';
				xdigs = xdigs_upper;
				expchar = 'P';
			}
			if (prec >= 0)
				prec++;
			if (flags & LONGDBL) {
				fparg.ldbl = GETARG(long double);
				dtoaresult =
				    __hldtoa(fparg.ldbl, xdigs, prec,
				        &expt, &signflag, &dtoaend);
			} else {
				fparg.dbl = GETARG(double);
				dtoaresult =
				    __hdtoa(fparg.dbl, xdigs, prec,
				        &expt, &signflag, &dtoaend);
			}
			if (prec < 0)
				prec = dtoaend - dtoaresult;
			if (expt == INT_MAX)
				ox[1] = '\0';
			if (convbuf != NULL)
				free(convbuf);
			ndig = dtoaend - dtoaresult;
			cp = convbuf = __mbsconv(dtoaresult, -1, loc);
			freedtoa(dtoaresult);
			goto fp_common;
		case 'e':
		case 'E':
#ifdef VECTORS
			if (flags & VECTOR) {
				flags |= FPT;
				break;
			}
#endif /* VECTORS */
			expchar = ch;
			if (prec < 0)	/* account for digit before decpt */
				prec = DEFPREC + 1;
			else
				prec++;
			goto fp_begin;
		case 'f':
		case 'F':
#ifdef VECTORS
			if (flags & VECTOR) {
				flags |= FPT;
				break;
			}
#endif /* VECTORS */
			expchar = '\0';
			goto fp_begin;
		case 'g':
		case 'G':
#ifdef VECTORS
			if (flags & VECTOR) {
				flags |= FPT;
				break;
			}
#endif /* VECTORS */
			expchar = ch - ('g' - 'e');
			if (prec == 0)
				prec = 1;
fp_begin:
			if (prec < 0)
				prec = DEFPREC;
			if (convbuf != NULL)
				free(convbuf);
			if (flags & LONGDBL) {
				fparg.ldbl = GETARG(long double);
				dtoaresult =
				    __ldtoa(&fparg.ldbl, expchar ? 2 : 3, prec,
				    &expt, &signflag, &dtoaend);
			} else {
				fparg.dbl = GETARG(double);
				dtoaresult =
				    dtoa(fparg.dbl, expchar ? 2 : 3, prec,
				    &expt, &signflag, &dtoaend);
				if (expt == 9999)
					expt = INT_MAX;
			}
			ndig = dtoaend - dtoaresult;
			cp = convbuf = __mbsconv(dtoaresult, -1, loc);
			freedtoa(dtoaresult);
fp_common:
			if (signflag)
				sign = '-';
			if (expt == INT_MAX) {	/* inf or nan */
				if (*cp == 'N') {
					cp = (ch >= 'a') ? L"nan" : L"NAN";
					sign = '\0';
				} else
					cp = (ch >= 'a') ? L"inf" : L"INF";
				size = 3;
				flags &= ~ZEROPAD;
				break;
			}
			flags |= FPT;
			if (ch == 'g' || ch == 'G') {
				if (expt > -4 && expt <= prec) {
					/* Make %[gG] smell like %[fF] */
					expchar = '\0';
					if (flags & ALT)
						prec -= expt;
					else
						prec = ndig - expt;
					if (prec < 0)
						prec = 0;
				} else {
					/*
					 * Make %[gG] smell like %[eE], but
					 * trim trailing zeroes if no # flag.
					 */
					if (!(flags & ALT))
						prec = ndig;
				}
			}
			if (expchar) {
				expsize = exponent(expstr, expt - 1, expchar);
				size = expsize + prec;
				if (prec > 1 || flags & ALT)
					++size;
			} else {
				/* space for digits before decimal point */
				if (expt > 0)
					size = expt;
				else	/* "0" */
					size = 1;
				/* space for decimal pt and following digits */
				if (prec || flags & ALT)
					size += prec + 1;
				if ((flags & GROUPING) && expt > 0)
					size += grouping_init(&gs, expt, loc);
			}
			break;
#endif /* !NO_FLOATING_POINT */
		case 'n':
		{
			/*
			 * Assignment-like behavior is specified if the
			 * value overflows or is otherwise unrepresentable.
			 * C99 says to use `signed char' for %hhn conversions.
			 */
			void *ptr = GETARG(void *);
			if (ptr == NULL)
				continue;
			else if (flags & LLONGINT)
				*(long long *)ptr = ret;
			else if (flags & SIZET)
				*(ssize_t *)ptr = (ssize_t)ret;
			else if (flags & PTRDIFFT)
				*(ptrdiff_t *)ptr = ret;
			else if (flags & INTMAXT)
				*(intmax_t *)ptr = ret;
			else if (flags & LONGINT)
				*(long *)ptr = ret;
			else if (flags & SHORTINT)
				*(short *)ptr = ret;
			else if (flags & CHARINT)
				*(signed char *)ptr = ret;
			else
				*(int *)ptr = ret;
			continue;	/* no output */
		}
		case 'O':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'o':
#ifdef VECTORS
			if (flags & VECTOR)
				break;
#endif /* VECTORS */
			if (flags & INTMAX_SIZE)
				ujval = UJARG();
			else
				ulval = UARG();
			base = 8;
			goto nosign;
		case 'p':
			/*-
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
#ifdef VECTORS
			if (flags & VECTOR)
				break;
#endif /* VECTORS */
			ujval = (uintmax_t)(uintptr_t)GETARG(void *);
			base = 16;
			xdigs = xdigs_lower;
			flags = flags | INTMAXT;
			ox[1] = 'x';
			goto nosign;
		case 'S':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 's':
			if (flags & LONGINT) {
				if ((cp = GETARG(wchar_t *)) == NULL)
					cp = L"(null)";
			} else {
				char *mbp;

				if (convbuf != NULL)
					free(convbuf);
				if ((mbp = GETARG(char *)) == NULL)
					cp = L"(null)";
				else {
					convbuf = __mbsconv(mbp, prec, loc);
					if (convbuf == NULL) {
						fp->_flags |= __SERR;
						goto error;
					}
					cp = convbuf;
				}
			}
#if 0 // wcsnlen needs API review first
			size = (prec >= 0) ? wcsnlen(cp, prec) : wcslen(cp);
#else
			size = wcslen(cp);
			if(prec >= 0 && prec < size)
				size = prec;
#endif
			sign = '\0';
			break;
		case 'U':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'u':
#ifdef VECTORS
			if (flags & VECTOR)
				break;
#endif /* VECTORS */
			if (flags & INTMAX_SIZE)
				ujval = UJARG();
			else
				ulval = UARG();
			base = 10;
			goto nosign;
		case 'X':
			xdigs = xdigs_upper;
			goto hex;
		case 'x':
			xdigs = xdigs_lower;
hex:
#ifdef VECTORS
			if (flags & VECTOR)
				break;
#endif /* VECTORS */
			if (flags & INTMAX_SIZE)
				ujval = UJARG();
			else
				ulval = UARG();
			base = 16;
			/* leading 0x/X only if non-zero */
			if (flags & ALT &&
			    (flags & INTMAX_SIZE ? ujval != 0 : ulval != 0))
				ox[1] = ch;

			flags &= ~GROUPING;
			/* unsigned conversions */
nosign:			sign = '\0';
			/*-
			 * ``... diouXx conversions ... if a precision is
			 * specified, the 0 flag will be ignored.''
			 *	-- ANSI X3J11
			 * except for %#.0o and zero value
			 */
number:			if ((dprec = prec) >= 0)
				flags &= ~ZEROPAD;

			/*-
			 * ``The result of converting a zero value with an
			 * explicit precision of zero is no characters.''
			 *	-- ANSI X3J11
			 *
			 * ``The C Standard is clear enough as is.  The call
			 * printf("%#.0o", 0) should print 0.''
			 *	-- Defect Report #151
			 */
			cp = buf + BUF;
			if (flags & INTMAX_SIZE) {
				if (ujval != 0 || prec != 0 ||
				    (flags & ALT && base == 8))
					cp = __ujtoa(ujval, cp, base,
					    flags & ALT, xdigs);
			} else {
				if (ulval != 0 || prec != 0 ||
				    (flags & ALT && base == 8))
					cp = __ultoa(ulval, cp, base,
					    flags & ALT, xdigs);
			}
			size = buf + BUF - cp;
			if (size > BUF)	/* should never happen */
				LIBC_ABORT("size (%d) > BUF (%d)", size, BUF);
			if ((flags & GROUPING) && size != 0)
				size += grouping_init(&gs, size, loc);
			break;
#ifdef VECTORS
		case 'v':
			flags |= VECTOR;
			goto rflag;
#endif /* VECTORS */
		default:	/* "%?" prints ?, unless ? is NUL */
			if (ch == '\0')
				goto done;
			/* pretend it was %c with argument ch */
			cp = buf;
			*cp = ch;
			size = 1;
			sign = '\0';
			break;
		}

#ifdef VECTORS
		if (flags & VECTOR) {
			/*
			 * Do the minimum amount of work necessary to construct
			 * a format specifier that can be used to recursively
			 * call vfprintf() for each element in the vector.
			 */
			int i, j;	/* Counter. */
			int vcnt;	/* Number of elements in vector. */
			char *vfmt;	/* Pointer to format specifier. */
#define EXTRAHH 2
			char vfmt_buf[32 + EXTRAHH]; /* Static buffer for format spec. */
			int vwidth = 0;	/* Width specified via '*'. */
			int vprec = 0;	/* Precision specified via '*'. */
			char *vstr;	/* Used for asprintf(). */
			int vlen;	/* Length returned by asprintf(). */
			enum {
			    V_CHAR, V_SHORT, V_INT,
			    V_PCHAR, V_PSHORT, V_PINT,
			    V_FLOAT,
#ifdef V64TYPE
			    V_LONGLONG, V_PLONGLONG,
			    V_DOUBLE,
#endif /* V64TYPE */
			} vtype;

			vval.vectorarg = GETARG(VECTORTYPE);
			/*
			 * Set vfmt.  If vfmt_buf may not be big enough,
			 * malloc() space, taking care to free it later.
			 * (EXTRAHH is for possible extra "hh")
			 */
			if (&fmt[-1] - pct + EXTRAHH < sizeof(vfmt_buf))
				vfmt = vfmt_buf;
			else
				vfmt = (char *)malloc(&fmt[-1] - pct + EXTRAHH + 1);

			/* Set the separator character, if not specified. */
			if (vsep == 'X') {
				if (ch == 'c')
					vsep = '\0';
				else
					vsep = ' ';
			}

			/* Create the format specifier. */
			for (i = j = 0; i < &fmt[-1] - pct; i++) {
				switch (pct[i]) {
				case ',': case ';': case ':': case '_':
				case 'v': case 'h': case 'l':
					/* Ignore. */
					break;
				case '*':
					if (pct[i - 1] != '.')
						vwidth = 1;
					else
						vprec = 1;
					/* FALLTHROUGH */
				default:
					vfmt[j++] = pct[i];
				}
			}
		
			/*
			 * Determine the number of elements in the vector and
			 * finish up the format specifier.
			 */
			if (flags & SHORTINT) {
				switch (ch) {
				case 'c':
					vtype = V_SHORT;
					break;
				case 'p':
					vtype = V_PSHORT;
					break;
				default:
					vfmt[j++] = 'h';
					vtype = V_SHORT;
					break;
				}
				vcnt = 8;
			} else if (flags & LONGINT) {
				vcnt = 4;
				vtype = (ch == 'p') ? V_PINT : V_INT;
#ifdef V64TYPE
			} else if (flags & LLONGINT) {
				switch (ch) {
				case 'a':
				case 'A':
				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G':
					vcnt = 2;
					vtype = V_DOUBLE;
					break;
				case 'd':
				case 'i':
				case 'u':
				case 'o':
				case 'p':
				case 'x':
				case 'X':
					vfmt[j++] = 'l';
					vfmt[j++] = 'l';
					vcnt = 2;
					vtype = (ch == 'p') ? V_PLONGLONG : V_LONGLONG;
					break;
				default:
					/*
					 * The default case should never
					 * happen.
					 */
				case 'c':
					vcnt = 16;
					vtype = V_CHAR;
				}
#endif /* V64TYPE */
			} else {
				switch (ch) {
				case 'a':
				case 'A':
				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G':
					vcnt = 4;
					vtype = V_FLOAT;
					break;
				default:
					/*
					 * The default case should never
					 * happen.
					 */
				case 'd':
				case 'i':
				case 'u':
				case 'o':
				case 'x':
				case 'X':
					vfmt[j++] = 'h';
					vfmt[j++] = 'h';
					/* drop through */
				case 'p':
				case 'c':
					vcnt = 16;
					vtype = (ch == 'p') ? V_PCHAR : V_CHAR;
				}
			}
			vfmt[j++] = ch;
			vfmt[j++] = '\0';

/* Get a vector element. */
#ifdef V64TYPE
#define VPRINT(type, ind, args...) do {					\
	switch (type) {							\
	case V_CHAR:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vuchararg[ind]); \
		break;							\
	case V_PCHAR:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, (void *)(uintptr_t)vval.vuchararg[ind]); \
		break;							\
	case V_SHORT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vushortarg[ind]); \
		break;							\
	case V_PSHORT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, (void *)(uintptr_t)vval.vushortarg[ind]); \
		break;							\
	case V_INT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vuintarg[ind]); \
		break;							\
	case V_PINT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, (void *)(uintptr_t)vval.vuintarg[ind]); \
		break;							\
	case V_LONGLONG:						\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vulonglongarg[ind]); \
		break;							\
	case V_PLONGLONG:						\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, (void *)(uintptr_t)vval.vulonglongarg[ind]); \
		break;							\
	case V_FLOAT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vfloatarg[ind]); \
		break;							\
	case V_DOUBLE:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vdoublearg[ind]); \
		break;							\
	}								\
	ret += vlen;							\
	PRINT(vstr, vlen);						\
	free(vstr);							\
} while (0)
#else /* !V64TYPE */
#define VPRINT(type, ind, args...) do {					\
	switch (type) {							\
	case V_CHAR:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vuchararg[ind]); \
		break;							\
	case V_PCHAR:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, (void *)(uintptr_t)vval.vuchararg[ind]); \
		break;							\
	case V_SHORT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vushortarg[ind]); \
		break;							\
	case V_PSHORT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, (void *)(uintptr_t)vval.vushortarg[ind]); \
		break;							\
	case V_INT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vuintarg[ind]); \
		break;							\
	case V_PINT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, (void *)(uintptr_t)vval.vuintarg[ind]); \
		break;							\
	case V_FLOAT:							\
		vlen = asprintf_l(&vstr, loc, vfmt , ## args, vval.vfloatarg[ind]); \
		break;							\
	}								\
	ret += vlen;							\
	PRINT(vstr, vlen);						\
	free(vstr);							\
} while (0)
#endif /* V64TYPE */

			/* Actually print. */
			if (vwidth == 0) {
				if (vprec == 0) {
					/* First element. */
					VPRINT(vtype, 0);
					for (i = 1; i < vcnt; i++) {
						/* Separator. */
						if(vsep)
							PRINT(&vsep, 1);

						/* Element. */
						VPRINT(vtype, i);
					}
				} else {
					/* First element. */
					VPRINT(vtype, 0, prec);
					for (i = 1; i < vcnt; i++) {
						/* Separator. */
						if(vsep)
							PRINT(&vsep, 1);

						/* Element. */
						VPRINT(vtype, i, prec);
					}
				}
			} else {
				if (vprec == 0) {
					/* First element. */
					VPRINT(vtype, 0, width);
					for (i = 1; i < vcnt; i++) {
						/* Separator. */
						if(vsep)
							PRINT(&vsep, 1);

						/* Element. */
						VPRINT(vtype, i, width);
					}
				} else {
					/* First element. */
					VPRINT(vtype, 0, width, prec);
					for (i = 1; i < vcnt; i++) {
						/* Separator. */
						if(vsep)
							PRINT(&vsep, 1);

						/* Element. */
						VPRINT(vtype, i, width, prec);
					}
				}
			}
#undef VPRINT

			if (vfmt != vfmt_buf)
				free(vfmt);

			continue;
		}
#endif /* VECTORS */
		/*
		 * All reasonable formats wind up here.  At this point, `cp'
		 * points to a string which (if not flags&LADJUST) should be
		 * padded out to `width' places.  If flags&ZEROPAD, it should
		 * first be prefixed by any sign or other prefix; otherwise,
		 * it should be blank padded before the prefix is emitted.
		 * After any left-hand padding and prefixing, emit zeroes
		 * required by a decimal [diouxX] precision, then print the
		 * string proper, then emit zeroes required by any leftover
		 * floating precision; finally, if LADJUST, pad with blanks.
		 *
		 * Compute actual size, so we know how much to pad.
		 * size excludes decimal prec; realsz includes it.
		 */
		realsz = dprec > size ? dprec : size;
		if (sign)
			realsz++;
		if (ox[1])
			realsz += 2;

		prsize = width > realsz ? width : realsz;
		if ((unsigned)ret + prsize > INT_MAX) {
			ret = EOF;
			goto error;
		}

		/* right-adjusting blank padding */
		if ((flags & (LADJUST|ZEROPAD)) == 0)
			PAD(width - realsz, blanks);

		/* prefix */
		if (sign)
			PRINT(&sign, 1);

		if (ox[1]) {	/* ox[1] is either x, X, or \0 */
			ox[0] = '0';
			PRINT(ox, 2);
		}

		/* right-adjusting zero padding */
		if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
			PAD(width - realsz, zeroes);

		/* the string or number proper */
#ifndef NO_FLOATING_POINT
		if ((flags & FPT) == 0) {
#endif
			/* leading zeroes from decimal precision */
			PAD(dprec - size, zeroes);
			if (gs.grouping) {
				if (grouping_print(&gs, &io, cp, buf+BUF, loc) < 0)
					goto error;
			} else {
				PRINT(cp, size);
			}
#ifndef NO_FLOATING_POINT
		} else {	/* glue together f_p fragments */
			if (!expchar) {	/* %[fF] or sufficiently short %[gG] */
				if (expt <= 0) {
					PRINT(zeroes, 1);
					if (prec || flags & ALT)
						PRINT(&decimal_point, 1);
					PAD(-expt, zeroes);
					/* already handled initial 0's */
					prec += expt;
				} else {
					if (gs.grouping) {
						n = grouping_print(&gs, &io,
						    cp, convbuf + ndig, loc);
						if (n < 0)
							goto error;
						cp += n;
					} else {
						PRINTANDPAD(cp, convbuf + ndig,
						    expt, zeroes);
						cp += expt;
					}
					if (prec || flags & ALT)
						PRINT(&decimal_point, 1);
				}
				PRINTANDPAD(cp, convbuf + ndig, prec, zeroes);
			} else {	/* %[eE] or sufficiently long %[gG] */
				if (prec > 1 || flags & ALT) {
					buf[0] = *cp++;
					buf[1] = decimal_point;
					PRINT(buf, 2);
					PRINT(cp, ndig-1);
					PAD(prec - ndig, zeroes);
				} else	/* XeYYY */
					PRINT(cp, 1);
				PRINT(expstr, expsize);
			}
		}
#endif
		/* left-adjusting padding (always blank) */
		if (flags & LADJUST)
			PAD(width - realsz, blanks);

		/* finally, adjust ret */
		ret += prsize;

		FLUSH();	/* copy out the I/O vectors */
	}
done:
	FLUSH();
error:
	va_end(orgap);
	if (convbuf != NULL)
		free(convbuf);
	if (__sferror(fp))
		ret = EOF;
	if ((argtable != NULL) && (argtable != statargtable))
		free (argtable);
	return (ret);
	/* NOTREACHED */
}
