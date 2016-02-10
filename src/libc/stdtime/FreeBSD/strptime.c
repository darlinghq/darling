/*
 * Powerdog Industries kindly requests feedback from anyone modifying
 * this function:
 *
 * Date: Thu, 05 Jun 1997 23:17:17 -0400  
 * From: Kevin Ruddy <kevin.ruddy@powerdog.com>
 * To: James FitzGibbon <james@nexis.net>
 * Subject: Re: Use of your strptime(3) code (fwd)
 * 
 * The reason for the "no mod" clause was so that modifications would
 * come back and we could integrate them and reissue so that a wider 
 * audience could use it (thereby spreading the wealth).  This has   
 * made it possible to get strptime to work on many operating systems.
 * I'm not sure why that's "plain unacceptable" to the FreeBSD team.
 * 
 * Anyway, you can change it to "with or without modification" as
 * you see fit.  Enjoy.                                          
 * 
 * Kevin Ruddy
 * Powerdog Industries, Inc.
 */
/*
 * Copyright (c) 1994 Powerdog Industries.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *      This product includes software developed by Powerdog Industries.
 * 4. The name of Powerdog Industries may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY POWERDOG INDUSTRIES ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE POWERDOG INDUSTRIES BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
#ifndef NOID
static char copyright[] __unused =
"@(#) Copyright (c) 1994 Powerdog Industries.  All rights reserved.";
static char sccsid[] __unused = "@(#)strptime.c	0.1 (Powerdog) 94/03/27";
#endif /* !defined NOID */
#endif /* not lint */
__FBSDID("$FreeBSD: src/lib/libc/stdtime/strptime.c,v 1.37 2009/09/02 04:56:30 ache Exp $");

#include "xlocale_private.h"

#include "namespace.h"
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <limits.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "timelocal.h"

time_t _mktime(struct tm *, const char *);

#define asizeof(a)	(sizeof (a) / sizeof ((a)[0]))

enum {CONVERT_NONE, CONVERT_GMT, CONVERT_ZONE};

#define _strptime(b,f,t,c,l)	_strptime0(b,f,t,c,l,-1,0,-1)

static char *
_strptime0(const char *buf, const char *fmt, struct tm *tm, int *convp, locale_t loc, int year, int yday, int wday)
{
	char	c;
	const char *ptr;
	int	i,
		len;
	int Ealternative, Oalternative;
	struct lc_time_T *tptr = __get_current_time_locale(loc);

	ptr = fmt;
	while (*ptr != 0) {
		if (*buf == 0) {
			fmt = ptr;
			while (isspace_l((unsigned char)*ptr, loc)) {
				ptr++;
			}
			return ((*ptr)==0) ? (char *)fmt : 0; /* trailing whitespace is ok */
		}

		c = *ptr++;

		if (c != '%') {
			if (isspace_l((unsigned char)c, loc))
				while (*buf != 0 && isspace_l((unsigned char)*buf, loc))
					buf++;
			else if (c != *buf++)
				return 0;
			continue;
		}

		Ealternative = 0;
		Oalternative = 0;
label:
		c = *ptr++;
		switch (c) {
		case 0:
		case '%':
			if (*buf++ != '%')
				return 0;
			break;

		case '+':
			buf = _strptime(buf, tptr->date_fmt, tm, convp, loc);
			if (buf == 0)
				return 0;
			break;

		case 'C':
			if (!isdigit_l((unsigned char)*buf, loc))
				return 0;

			/* XXX This will break for 3-digit centuries. */
			len = 2;
			for (i = 0; len && *buf != 0 && isdigit_l((unsigned char)*buf, loc); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i < 19)
				return 0;

			if (year != -1)
				tm->tm_year = (year % 100) + i * 100 - 1900;
			else
			tm->tm_year = i * 100 - 1900;
			year = tm->tm_year;
			break;

		case 'c':
			buf = _strptime(buf, tptr->c_fmt, tm, convp, loc);
			if (buf == 0)
				return 0;
			break;

		case 'D':
			buf = _strptime(buf, "%m/%d/%y", tm, convp, loc);
			if (buf == 0)
				return 0;
			break;

		case 'E':
			if (Ealternative || Oalternative)
				break;
			Ealternative++;
			goto label;

		case 'O':
			if (Ealternative || Oalternative)
				break;
			Oalternative++;
			goto label;

		case 'F':
			buf = _strptime(buf, "%Y-%m-%d", tm, convp, loc);
			if (buf == 0)
				return 0;
			break;

		case 'R':
			buf = _strptime(buf, "%H:%M", tm, convp, loc);
			if (buf == 0)
				return 0;
			break;

		case 'r':
			buf = _strptime(buf, tptr->ampm_fmt, tm, convp, loc);
			if (buf == 0)
				return 0;
			break;

		case 'n':
		case 't':
			if (!isspace((unsigned char)*buf))
				return 0;
			while (isspace((unsigned char)*buf))
				buf++;
			break;

		case 'T':
			buf = _strptime(buf, "%H:%M:%S", tm, convp, loc);
			if (buf == 0)
				return 0;
			break;

		case 'X':
			buf = _strptime(buf, tptr->X_fmt, tm, convp, loc);
			if (buf == 0)
				return 0;
			break;

		case 'x':
			buf = _strptime(buf, tptr->x_fmt, tm, convp, loc);
			if (buf == 0)
				return 0;
			break;

		case 'j':
			if (!isdigit_l((unsigned char)*buf, loc))
				return 0;

			len = 3;
			for (i = 0; len && *buf != 0 && isdigit_l((unsigned char)*buf, loc); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i < 1 || i > 366)
				return 0;

			tm->tm_yday = yday = i - 1;
			break;

		case 'M':
		case 'S':
			if (*buf == 0 || isspace_l((unsigned char)*buf, loc))
				break;

			if (!isdigit_l((unsigned char)*buf, loc))
				return 0;

			len = 2;
			for (i = 0; len && *buf != 0 && isdigit_l((unsigned char)*buf, loc); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}

			if (c == 'M') {
				if (i > 59)
					return 0;
				tm->tm_min = i;
			} else {
				if (i > 60)
					return 0;
				tm->tm_sec = i;
			}

			if (*buf != 0 && isspace_l((unsigned char)*buf, loc))
				while (*ptr != 0 && !isspace_l((unsigned char)*ptr, loc) && *ptr != '%')
					ptr++;
			break;

		case 'H':
		case 'I':
		case 'k':
		case 'l':
			/*
			 * Of these, %l is the only specifier explicitly
			 * documented as not being zero-padded.  However,
			 * there is no harm in allowing zero-padding.
			 *
			 * XXX The %l specifier may gobble one too many
			 * digits if used incorrectly.
			 */
			if (!isdigit_l((unsigned char)*buf, loc))
				return 0;

			len = 2;
			for (i = 0; len && *buf != 0 && isdigit_l((unsigned char)*buf, loc); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (c == 'H' || c == 'k') {
				if (i > 23)
					return 0;
			} else if (i > 12)
				return 0;

			tm->tm_hour = i;

			if (*buf != 0 && isspace_l((unsigned char)*buf, loc))
				while (*ptr != 0 && !isspace_l((unsigned char)*ptr, loc) && *ptr != '%')
					ptr++;
			break;

		case 'p':
			/*
			 * XXX This is bogus if parsed before hour-related
			 * specifiers.
			 */
			len = strlen(tptr->am);
			if (strncasecmp_l(buf, tptr->am, len, loc) == 0) {
				if (tm->tm_hour > 12)
					return 0;
				if (tm->tm_hour == 12)
					tm->tm_hour = 0;
				buf += len;
				break;
			}

			len = strlen(tptr->pm);
			if (strncasecmp_l(buf, tptr->pm, len, loc) == 0) {
				if (tm->tm_hour > 12)
					return 0;
				if (tm->tm_hour != 12)
					tm->tm_hour += 12;
				buf += len;
				break;
			}

			return 0;

		case 'A':
		case 'a':
			for (i = 0; i < asizeof(tptr->weekday); i++) {
				len = strlen(tptr->weekday[i]);
				if (strncasecmp_l(buf, tptr->weekday[i],
						len, loc) == 0)
					break;
				len = strlen(tptr->wday[i]);
				if (strncasecmp_l(buf, tptr->wday[i],
						len, loc) == 0)
					break;
			}
			if (i == asizeof(tptr->weekday))
				return 0;

			tm->tm_wday = wday = i;
			buf += len;
			break;

		case 'U':	/* Sunday week */
		case 'W':	/* Monday week */
			if (!isdigit_l((unsigned char)*buf, loc))
				return 0;

			len = 2;
			for (i = 0; len && *buf != 0 && isdigit_l((unsigned char)*buf, loc); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i > 53)
				return 0;

			/* Calculate yday if we have enough data */
			if ((year != -1) && (wday != -1)) {
				struct tm mktm;
				mktm.tm_year = year;
				mktm.tm_mon = 0;
				mktm.tm_mday = 1;
				mktm.tm_sec = 1;
				mktm.tm_min = mktm.tm_hour = 0;
				mktm.tm_isdst = 0;
				mktm.tm_gmtoff = 0;
				if (mktime(&mktm) != -1) {
					/* yday0 == Jan 1 == mktm.tm_wday */
					int delta = wday - mktm.tm_wday;
					if (!wday && c =='W')
						i++; /* Sunday is part of the following week */
					yday = 7 * i + delta;
					if (yday < 0)
						yday += 7;
					tm->tm_yday = yday;
				}
			}
			if (*buf != 0 && isspace_l((unsigned char)*buf, loc))
				while (*ptr != 0 && !isspace_l((unsigned char)*ptr, loc) && *ptr != '%')
					ptr++;
			break;

		case 'u':	/* [1,7] */
		case 'w':	/* [0,6] */
			if (!isdigit_l((unsigned char)*buf, loc))
				return 0;

			i = *buf - '0';
			if (i > 6 + (c == 'u'))
				return 0;
			if (i == 7)
				i = 0;
			tm->tm_wday = wday = i;
			buf++;
			if (*buf != 0 && isspace_l((unsigned char)*buf, loc))
				while (*ptr != 0 && !isspace_l((unsigned char)*ptr, loc) && *ptr != '%')
					ptr++;
			break;

		case 'd':
		case 'e':
			/*
			 * The %e specifier is explicitly documented as not
			 * being zero-padded but there is no harm in allowing
			 * such padding.
			 *
			 * XXX The %e specifier may gobble one too many
			 * digits if used incorrectly.
			 */
			/* Leading space is ok if date is single digit */
			len = 2;
			if (isspace_l((unsigned char)buf[0], loc) &&
			    isdigit_l((unsigned char)buf[1], loc) &&
			    !isdigit_l((unsigned char)buf[2], loc)) {
				len = 1;
				buf++;
			}
			if (!isdigit_l((unsigned char)*buf, loc))
				return 0;

			for (i = 0; len && *buf != 0 && isdigit_l((unsigned char)*buf, loc); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i > 31)
				return 0;

			tm->tm_mday = i;

			if (*buf != 0 && isspace_l((unsigned char)*buf, loc))
				while (*ptr != 0 && !isspace_l((unsigned char)*ptr, loc) && *ptr != '%')
					ptr++;
			break;

		case 'B':
		case 'b':
		case 'h':
			for (i = 0; i < asizeof(tptr->month); i++) {
				if (Oalternative) {
					if (c == 'B') {
						len = strlen(tptr->alt_month[i]);
						if (strncasecmp_l(buf,
								tptr->alt_month[i],
								len, loc) == 0)
							break;
					}
				} else {
					len = strlen(tptr->month[i]);
					if (strncasecmp_l(buf, tptr->month[i],
							len, loc) == 0)
						break;
					len = strlen(tptr->mon[i]);
					if (strncasecmp_l(buf, tptr->mon[i],
							len, loc) == 0)
						break;
				}
			}
			if (i == asizeof(tptr->month))
				return 0;

			tm->tm_mon = i;
			buf += len;
			break;

		case 'm':
			if (!isdigit_l((unsigned char)*buf, loc))
				return 0;

			len = 2;
			for (i = 0; len && *buf != 0 && isdigit_l((unsigned char)*buf, loc); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i < 1 || i > 12)
				return 0;

			tm->tm_mon = i - 1;

			if (*buf != 0 && isspace_l((unsigned char)*buf, loc))
				while (*ptr != 0 && !isspace_l((unsigned char)*ptr, loc) && *ptr != '%')
					ptr++;
			break;

		case 's':
			{
			char *cp;
			int sverrno;
			long n;
			time_t t;

			sverrno = errno;
			errno = 0;
			n = strtol_l(buf, &cp, 10, loc);
			if (errno == ERANGE || (long)(t = n) != n) {
				errno = sverrno;
				return 0;
			}
			errno = sverrno;
			buf = cp;
			gmtime_r(&t, tm);
			*convp = CONVERT_GMT;
			}
			break;

		case 'Y':
		case 'y':
			if (*buf == 0 || isspace_l((unsigned char)*buf, loc))
				break;

			if (!isdigit_l((unsigned char)*buf, loc))
				return 0;

#if __DARWIN_UNIX03
			if (c == 'Y') {
				int savei = 0;
				const char *savebuf = buf;
				int64_t i64 = 0;
				int overflow = 0;

				for (len = 0; *buf != 0 && isdigit_l((unsigned char)*buf, loc); buf++) {
					i64 *= 10;
					i64 += *buf - '0';
					if (++len <= 4) {
						savei = i64;
						savebuf = buf + 1;
					}
					if (i64 > INT_MAX) {
						overflow++;
						break;
					}
				}
				/*
				 * Conformance requires %Y to be more then 4
				 * digits.  However, there are several cases
				 * where %Y is immediately followed by other
				 * digits values.  So we do the conformance
				 * case first (as many digits as possible),
				 * and if we fail, we backup and try just 4
				 * digits for %Y.
				 */
				if (len > 4 && !overflow) {
					struct tm savetm = *tm;
					int saveconv = *convp;
					const char *saveptr = ptr;
					char *ret;

					if (i64 < 1900)
						return 0;

					tm->tm_year = i64 - 1900;

					if (*buf != 0 && isspace_l((unsigned char)*buf, loc))
						while (*ptr != 0 && !isspace_l((unsigned char)*ptr, loc) && *ptr != '%')
							ptr++;
					ret = _strptime0(buf, ptr, tm, convp, loc, tm->tm_year, yday, wday);
					if (ret) return ret;
					/* Failed, so try 4-digit year */
					*tm = savetm;
					*convp = saveconv;
					ptr = saveptr;
				}
				buf = savebuf;
				i = savei;
			} else {
				len = 2;
#else /* !__DARWIN_UNIX03 */
			len = (c == 'Y') ? 4 : 2;
#endif /* __DARWIN_UNIX03 */
			for (i = 0; len && *buf != 0 && isdigit_l((unsigned char)*buf, loc); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
#if __DARWIN_UNIX03
			}
#endif /* __DARWIN_UNIX03 */
			if (c == 'Y')
				i -= 1900;
			if (c == 'y' && i < 69)
				i += 100;
			if (i < 0)
				return 0;

			tm->tm_year = year = i;

			if (*buf != 0 && isspace_l((unsigned char)*buf, loc))
				while (*ptr != 0 && !isspace_l((unsigned char)*ptr, loc) && *ptr != '%')
					ptr++;
			break;

		case 'Z':
			{
			const char *cp;
			size_t tzlen, len;

			for (cp = buf; *cp && isupper((unsigned char)*cp); ++cp) {/*empty*/}
			len = cp - buf;
			if (len == 3 && strncmp(buf, "GMT", 3) == 0) {
				*convp = CONVERT_GMT;
				buf += len;
				break;
			}
			tzset();
			tzlen = strlen(tzname[0]);
			if (len == tzlen && strncmp(buf, tzname[0], tzlen) == 0) {
				tm->tm_isdst = 0;
				buf += len;
				break;
			}
			tzlen = strlen(tzname[1]);
			if (len == tzlen && strncmp(buf, tzname[1], tzlen) == 0) {
				tm->tm_isdst = 1;
				buf += len;
				break;
			}
			return 0;
			}

		case 'z':
			{
			int sign = 1;

			if (*buf != '+') {
				if (*buf == '-')
					sign = -1;
				else
					return 0;
			}

			buf++;
			i = 0;
			for (len = 4; len > 0; len--) {
				if (isdigit_l((unsigned char)*buf, loc)) {
					i *= 10;
					i += *buf - '0';
					buf++;
				} else
					return 0;
			}

			tm->tm_hour -= sign * (i / 100);
			tm->tm_min  -= sign * (i % 100);
			*convp = CONVERT_GMT;
			}
			break;
		}
	}
	return (char *)buf;
}


char *
strptime(const char * __restrict buf, const char * __restrict fmt,
    struct tm * __restrict tm)
{
	return strptime_l(buf, fmt, tm, __current_locale());
}

extern time_t timeoff(struct tm *, long);

char *
strptime_l(const char * __restrict buf, const char * __restrict fmt,
    struct tm * __restrict tm, locale_t loc)
{
	char *ret;
	int conv;

	NORMALIZE_LOCALE(loc);
	conv = CONVERT_NONE;
	tm->tm_zone = NULL;
	ret = _strptime(buf, fmt, tm, &conv, loc);
	if (ret) {
		time_t t;

		switch(conv) {
		case CONVERT_GMT:
			t = timegm(tm);
			localtime_r(&t, tm);
			break;
		case CONVERT_ZONE:
		    {
			long offset = tm->tm_gmtoff;
			tm->tm_gmtoff = 0;
			t = timeoff(tm, offset);
			localtime_r(&t, tm);
			break;
		    }
		}
	}

	return (ret);
}
